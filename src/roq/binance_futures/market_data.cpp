/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/market_data.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/charconv/to_string.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "md"sv;

auto const SUPPORTS_PRIMARY = Mask{
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

auto const SUPPORTS_SECONDARY = Mask{
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto priority) {
  auto result = fmt::format("{}:{}"sv, stream_id, NAME);
  switch (priority) {
    using enum Priority;
    case UNDEFINED:
      break;
    case PRIMARY:
      result.append(":1"sv);
      break;
    case SECONDARY:
      result.append(":2"sv);
      break;
  }
  return result;
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = settings.ws.host,
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = settings.net.disconnect_on_idle_timeout,
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

auto get_supports(auto priority) {
  switch (priority) {
    using enum Priority;
    case UNDEFINED:
      log::fatal("Unexpected"sv);
      break;
    case PRIMARY:
      break;
    case SECONDARY:
      return SUPPORTS_SECONDARY;
  }
  return SUPPORTS_PRIMARY;
}
}  // namespace

// === IMPLEMENTATION ===

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Priority priority, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, priority_{priority}, name_{create_name(stream_id_, priority_)}, index_{index},
      connection_{create_connection(*this, shared.settings, context)}, decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      request_id_{static_cast<uint64_t>(stream_id_) * 1000000},  // scale (debugging)
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
          .total_bytes_received = create_metrics(shared.settings, name_, "total_bytes_received"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .result = create_metrics(shared.settings, name_, "result"sv),
          .agg_trade = create_metrics(shared.settings, name_, "agg_trade"sv),
          .mark_price_update = create_metrics(shared.settings, name_, "mark_price_update"sv),
          .mini_ticker = create_metrics(shared.settings, name_, "mini_ticker"sv),
          .book_ticker = create_metrics(shared.settings, name_, "book_ticker"sv),
          .depth_update = create_metrics(shared.settings, name_, "depth_update"sv),
          .kline = create_metrics(shared.settings, name_, "kline"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      shared_{shared} {
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if ((*connection_).ready()) {
    check_subscribe_queue(now);
  }
}

void MarketData::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      .write(counter_.total_bytes_received, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.result, metrics::Type::PROFILE)
      .write(profile_.agg_trade, metrics::Type::PROFILE)
      .write(profile_.mark_price_update, metrics::Type::PROFILE)
      .write(profile_.mini_ticker, metrics::Type::PROFILE)
      .write(profile_.book_ticker, metrics::Type::PROFILE)
      .write(profile_.depth_update, metrics::Type::PROFILE)
      .write(profile_.kline, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready()) {
    subscribe(shared_.symbols.get_slice(index_, start_from));
  }
}

void MarketData::operator()(web::socket::Client::Connected const &) {
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  subscribe_queue_.clear();
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
  counter_.total_bytes_received.update((*connection_).total_bytes_received());
}

void MarketData::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = get_supports(priority_),
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols)) {
    return;
  }
  if (priority_ == Priority::PRIMARY) {
    subscribe(symbols, "aggTrade"sv);
    subscribe(symbols, "markPrice"sv);
    subscribe(symbols, "miniTicker"sv);
  }
  subscribe(symbols, "bookTicker"sv);
  auto frequency = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.ws.subscribe_depth_freq);
  auto depth = fmt::format(R"(depth@{}ms)"sv, frequency.count());
  subscribe(symbols, depth);
  if (shared_.settings.download.time_series_lookback.count()) {
    subscribe(symbols, "kline_1m"sv);
    for (auto &symbol : symbols) {
      shared_.time_series_request_queue.emplace_back(symbol);
    }
  }
}

void MarketData::subscribe(std::span<Symbol const> const &symbols, std::string_view const &channel) {
  assert(!std::empty(symbols));
  auto id = ++request_id_;
  auto separator = fmt::format(R"(@{}",")"sv, channel);
  auto message = fmt::format(
      R"({{)"
      R"("method":"SUBSCRIBE",)"
      R"("params":["{}@{}"],)"
      R"("id":{})"
      R"(}})"sv,
      fmt::join(symbols, separator),
      channel,
      id);
  subscribe_queue_.emplace_back(message);
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::MarketStreamParser::dispatch(*this, message, decode_buffer_, trace_info, shared_.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void MarketData::operator()(Trace<json::Error> const &event, int32_t id) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::warn("error={}, id={}"sv, error, id);
  });
}

void MarketData::operator()(Trace<json::Result> const &event, int32_t id) {
  profile_.result([&]() {
    auto &[trace_info, result] = event;
    log::info("result={}, id={}"sv, result, id);
  });
}

void MarketData::operator()(Trace<json::AggTrade> const &event) {
  profile_.agg_trade([&]() {
    auto &[trace_info, agg_trade] = event;
    log::info<3>("agg_trade={}"sv, agg_trade);
    (*connection_).touch(trace_info.source_receive_time);
    auto side = agg_trade.buyer_is_maker ? Side::SELL : Side::BUY;
    auto trade = Trade{
        .side = side,
        .price = agg_trade.price,
        .quantity = agg_trade.quantity,
        .trade_id = {},
        .taker_order_id = {},
        .maker_order_id = {},
    };
    utils::charconv::to_string(std::back_inserter(trade.trade_id), agg_trade.agg_trade_id);
    auto trade_summary = TradeSummary{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = agg_trade.symbol,
        .trades = {&trade, 1},
        .exchange_time_utc = agg_trade.trade_time,
        .exchange_sequence = {},
        .sending_time_utc = agg_trade.event_time,
    };
    create_trace_and_dispatch(handler_, event.trace_info, trade_summary, true);
  });
}

void MarketData::operator()(Trace<json::MiniTicker> const &event) {
  profile_.mini_ticker([&]() {
    auto &[trace_info, mini_ticker] = event;
    log::info<3>("mini_ticker={}"sv, mini_ticker);
    (*connection_).touch(trace_info.source_receive_time);
    std::array<Statistics, 5> statistics{{
        {
            .type = StatisticsType::HIGHEST_TRADED_PRICE,
            .value = mini_ticker.high_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::LOWEST_TRADED_PRICE,
            .value = mini_ticker.low_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::OPEN_PRICE,
            .value = mini_ticker.open_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::CLOSE_PRICE,
            .value = mini_ticker.close_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::TRADE_VOLUME,
            .value = mini_ticker.total_traded_base_asset_volume,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    }};
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = mini_ticker.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = mini_ticker.event_time,
    };
    create_trace_and_dispatch(handler_, event.trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::BookTicker> const &event) {
  profile_.book_ticker([&]() {
    auto &[trace_info, book_ticker] = event;
    log::info<3>("book_ticker={}"sv, book_ticker);
    (*connection_).touch(trace_info.source_receive_time);
    auto &instrument = shared_.get_instrument(book_ticker.symbol);
    if (!instrument.tob_update(utils::safe_cast(book_ticker.order_book_update_id))) {
      return;
    }
    auto top_of_book = TopOfBook{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = book_ticker.symbol,
        .layer{
            .bid_price = book_ticker.best_bid_price,
            .bid_quantity = book_ticker.best_bid_qty,
            .ask_price = book_ticker.best_ask_price,
            .ask_quantity = book_ticker.best_ask_qty,
        },
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = book_ticker.transaction_time,
        .exchange_sequence = book_ticker.order_book_update_id,
        .sending_time_utc = book_ticker.event_time,
    };
    create_trace_and_dispatch(handler_, event.trace_info, top_of_book, true);
  });
}

void MarketData::operator()(Trace<json::DepthUpdate> const &event) {
  profile_.depth_update([&]() {
    auto &trace_info = event.trace_info;
    auto &depth_update = event.value;
    log::info<3>(R"(depth_update={})"sv, depth_update);
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = depth_update.symbol;
    auto first_sequence = depth_update.first_update_id;
    auto last_sequence = depth_update.final_update_id;
    auto previous_sequence = depth_update.final_update_id_in_last_stream;
    auto &instrument = shared_.get_instrument(symbol);
    if (!instrument.mbp_update(utils::safe_cast(depth_update.final_update_id))) {
      return;
    }
    auto &sequencer = instrument.sequencer;
    auto &mbp = shared_.get_mbp();
    auto emplace_back = [](auto &result, auto &value) {
      auto mbp_update = MBPUpdate{
          .price = value.price,
          .quantity = value.qty,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      result.emplace_back(std::move(mbp_update));
    };
    for (auto &item : depth_update.bids) {
      emplace_back(mbp.bids, item);
    }
    for (auto &item : depth_update.asks) {
      emplace_back(mbp.asks, item);
    }
    try {
      auto create_update = [&](auto &bids, auto &asks, auto update_type, auto exchange_sequence) -> MarketByPriceUpdate {
        return {
            .stream_id = stream_id_,
            .exchange = shared_.settings.exchange,
            .symbol = symbol,
            .bids = bids,
            .asks = asks,
            .update_type = update_type,
            .exchange_time_utc = depth_update.transaction_time,
            .exchange_sequence = exchange_sequence,
            .sending_time_utc = depth_update.event_time,
            .price_precision = {},
            .quantity_precision = {},
            .checksum = {},
        };
      };
      auto publish_update = [&](auto &bids, auto &asks) {
        auto market_by_price_update = create_update(bids, asks, UpdateType::INCREMENTAL, last_sequence);
        create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
      };
      auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, auto retries, auto delay) {
        log::info(
            R"(DEBUG PUBLISH SNAPSHOT symbol="{}", sequence={}, retries={}, delay={})"sv,
            symbol,
            sequence,
            retries,
            std::chrono::duration_cast<std::chrono::milliseconds>(delay));
        auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT, sequencer.last_sequence());
        auto apply_updates = [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, true); };
        Trace event{trace_info, market_by_price_update};
        shared_(event, true, apply_updates);
      };
      auto request_snapshot = [&](auto retries) {
        log::info(R"(DEBUG REQUEST SNAPSHOT symbol="{}", retries={})"sv, symbol, retries);
        if (shared_.settings.ws.mbp_request_max_retries && shared_.settings.ws.mbp_request_max_retries < retries) {
          log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
        }
        shared_.depth_request_queue.emplace_back(symbol);
      };
      sequencer(mbp.bids, mbp.asks, first_sequence, last_sequence, previous_sequence, publish_update, publish_snapshot, request_snapshot);
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      // XXX FIXME publish stale
      sequencer.clear();
      shared_.depth_request_queue.emplace_back(symbol);
    }
  });
}

void MarketData::operator()(Trace<json::MarkPriceUpdate> const &event) {
  profile_.mark_price_update([&]() {
    auto &[trace_info, mark_price_update] = event;
    log::info<3>(R"(mark_price_update={})"sv, mark_price_update);
    (*connection_).touch(trace_info.source_receive_time);
    auto &mark_price = event.value;
    std::array<Statistics, 4> statistics{{
        {
            .type = StatisticsType::SETTLEMENT_PRICE,
            .value = mark_price.mark_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::PRE_SETTLEMENT_PRICE,
            .value = mark_price.est_settle_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::INDEX_VALUE,
            .value = mark_price.index_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::FUNDING_RATE,
            .value = mark_price.funding_rate,
            .begin_time_utc = utils::safe_cast(mark_price.event_time),
            .end_time_utc = utils::safe_cast(mark_price.next_funding_time),
        },
    }};
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = mark_price.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = mark_price.event_time,
    };
    create_trace_and_dispatch(handler_, event.trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::Kline> const &event) {
  profile_.kline([&]() {
    auto &[trace_info, kline] = event;
    log::info<3>(R"(kline={})"sv, kline);
    (*connection_).touch(trace_info.source_receive_time);
    if (!kline.data.closed && !shared_.settings.time_series.realtime) {
      return;
    }
    auto bar = Bar{
        .begin_time_utc = utils::safe_cast(kline.data.begin_time),
        .confirmed = kline.data.closed,
        .open_price = kline.data.open_price,
        .high_price = kline.data.high_price,
        .low_price = kline.data.low_price,
        .close_price = kline.data.close_price,
        .quantity = NaN,
        .base_amount = kline.data.base_asset_volume,
        .quote_amount = kline.data.quote_asset_volume,
        .number_of_trades = kline.data.number_of_trades,
        .vwap = NaN,
    };
    auto time_series_update = TimeSeriesUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = kline.data.symbol,
        .data_source = DataSource::TRADE_SUMMARY,
        .interval = shared_.settings.time_series.interval,
        .origin = Origin::EXCHANGE,
        .bars = {&bar, 1},
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = kline.event_time,
    };
    create_trace_and_dispatch(handler_, trace_info, time_series_update, true);
  });
}

// request

void MarketData::check_subscribe_queue(std::chrono::nanoseconds now) {
  subscribe_queue_.dispatch([&](auto now) { return shared_.rate_limiter.can_request(now); }, [&](auto &message) { (*connection_).send_text(message); }, now);
}

}  // namespace binance_futures
}  // namespace roq
