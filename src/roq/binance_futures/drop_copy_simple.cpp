/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/drop_copy_simple.hpp"

#include <algorithm>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/binance_futures/json/map.hpp"
#include "roq/binance_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::FUNDS,
    SupportType::POSITION,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_uri(auto &settings, auto &listen_key) {
  assert(!std::empty(listen_key));
  auto &uri = settings.ws.uri;
  auto result = fmt::format("{}://{}{}/{}"sv, uri.get_scheme(), uri.get_host(), uri.get_path(), listen_key);
  return io::web::URI{result};
}

auto create_connection(auto &handler, auto &settings, auto &context, auto &listen_key) {
  auto uri = create_uri(settings, listen_key);
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = settings.ws.host,
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
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
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory(settings.app.name, group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopySimple::DropCopySimple(
    Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared, Request &request, std::string_view const &listen_key)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context, listen_key)},
      decode_buffer_(shared.settings.misc.decode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .order_trade_update = create_metrics(shared.settings, name_, "order_trade_update"sv),
          .account_update = create_metrics(shared.settings, name_, "account_update"sv),
          .margin_call = create_metrics(shared.settings, name_, "margin_call"sv),
          .strategy_update = create_metrics(shared.settings, name_, "strategy_update"sv),
          .grid_update = create_metrics(shared.settings, name_, "grid_update"sv),
          .account_config_update = create_metrics(shared.settings, name_, "account_config_update"sv),
          .trade_lite = create_metrics(shared.settings, name_, "trade_lite"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared}, request_{request}, download_{{}, [this](auto state) { return download(state); }} {
}

bool DropCopySimple::ready() const {
  return (*connection_).ready();
}

void DropCopySimple::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopySimple::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopySimple::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
  check_response_balance();
  check_response_account();
  check_response_orders();
  check_response_trades();
}

void DropCopySimple::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.order_trade_update, metrics::Type::PROFILE)
      .write(profile_.account_update, metrics::Type::PROFILE)
      .write(profile_.margin_call, metrics::Type::PROFILE)
      .write(profile_.strategy_update, metrics::Type::PROFILE)
      .write(profile_.grid_update, metrics::Type::PROFILE)
      .write(profile_.account_config_update, metrics::Type::PROFILE)
      .write(profile_.trade_lite, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopySimple::operator()(web::socket::Client::Connected const &) {
}

void DropCopySimple::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void DropCopySimple::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopySimple::operator()(web::socket::Client::Close const &) {
}

void DropCopySimple::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopySimple::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopySimple::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopySimple::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
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

uint32_t DropCopySimple::download(DropCopyState state) {
  switch (state) {
    using enum DropCopyState;
    case UNDEFINED:
      assert(false);
      break;
    case BALANCE:
      request_balance();
      return 1;
    case ACCOUNT:
      request_account();
      return 1;
    case ORDERS:
      request_orders();
      return 1;
    case TRADES:
      if (shared_.settings.download.trades_lookback.count() && !std::empty(shared_.settings.download.symbols)) {
        request_trades();
        return 1;
      } else {
        return 0;
      }
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return 0;
  }
  assert(false);
  return 0;
}

void DropCopySimple::parse(std::string_view const &message) {
  auto log_message = [&]() { log::warn(R"(message="{}")"sv, message); };
  profile_.parse([&]() {
    try {
      TraceInfo trace_info;
      if (!json::UserStreamParser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.misc.continue_with_unknown_event_type))
        log_message();
    } catch (...) {
      log_message();
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopySimple::operator()(Trace<json::OrderTradeUpdate> const &event) {
  profile_.order_trade_update([&]() {
    auto &trace_info = event.trace_info;
    auto &order_trade_update = event.value;
    log::info<3>("order_trade_update={}"sv, order_trade_update);
    auto &execution_report = order_trade_update.execution_report;
    auto external_order_id = fmt::format("{}"sv, execution_report.order_id);  // alloc
    auto liquidity = execution_report.is_trade_maker ? Liquidity::MAKER : Liquidity::TAKER;
    // XXX HANS execution_report.execution_type ==> OrderAck ???
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = map(execution_report.side),
        .position_effect = {},
        .margin_mode = {},
        .max_show_quantity = NaN,
        .order_type = map(execution_report.order_type),
        .time_in_force = map(execution_report.time_in_force),
        .execution_instructions = {},
        .create_time_utc = {},
        .update_time_utc = order_trade_update.transaction_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = execution_report.client_order_id,
        .order_status = map(execution_report.order_status),
        .quantity = execution_report.original_quantity,
        .price = execution_report.original_price,
        .stop_price = execution_report.stop_price,
        .remaining_quantity = NaN,
        .traded_quantity = execution_report.order_filled_accumulated_quantity,
        .average_traded_price = execution_report.average_price,
        .last_traded_quantity = execution_report.last_filled_quantity,
        .last_traded_price = execution_report.last_filled_price,
        .last_liquidity = liquidity,
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::INCREMENTAL,
        .sending_time_utc = order_trade_update.event_time,
    };
    auto user_id = SOURCE_NONE;
    auto order_id = ORDER_ID_NONE;
    auto strategy_id = STRATEGY_ID_NONE;
    if (shared_.update_order(execution_report.client_order_id, stream_id_, trace_info, order_update, [&](auto &order) {
          user_id = order.user_id;
          order_id = order.order_id;
          strategy_id = order.strategy_id;
        })) {
    } else {
      log::warn("*** EXTERNAL ORDER ***"sv);
      log::warn("execution_report={}"sv, execution_report);
    }
    if (execution_report.execution_type != json::ExecutionType::TRADE)
      return;
    auto fill = Fill{
        .exchange_time_utc = execution_report.order_trade_time,
        .external_trade_id = {},
        .quantity = execution_report.last_filled_quantity,
        .price = execution_report.last_filled_price,
        .liquidity = liquidity,
        .quote_quantity = NaN,
        .commission_quantity = execution_report.commission,
        .commission_currency = execution_report.commission_asset,
    };
    fmt::format_to(std::back_inserter(fill.external_trade_id), "{}"sv, execution_report.trade_id);
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = order_id,
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = map(execution_report.side),
        .position_effect = {},
        .margin_mode = {},
        .quantity_type = {},
        .create_time_utc = execution_report.order_trade_time,
        .update_time_utc = execution_report.order_trade_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .fills = {&fill, 1},
        .routing_id = {},
        .update_type = UpdateType::INCREMENTAL,
        .sending_time_utc = order_trade_update.event_time,
        .user = {},
        .strategy_id = strategy_id,
    };
    create_trace_and_dispatch(handler_, trace_info, trade_update, true, user_id, execution_report.client_order_id);
  });
}

void DropCopySimple::operator()(Trace<json::AccountUpdate> const &event) {
  profile_.account_update([&]() {
    auto &[trace_info, account_update] = event;
    log::info<2>("account_update={}"sv, account_update);
    for (auto &item : account_update.data.balances) {
      log::info<2>("item={}"sv, item);
      auto funds_update = FundsUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .currency = item.asset,
          .margin_mode = {},  // XXX TODO maybe this should be MarginMode::ISOLATED?
          .balance = item.wallet_balance,
          .hold = NaN,  // note! we don't see this
          .external_account = {},
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = account_update.transaction_time,
          .sending_time_utc = account_update.event_time,
      };
      create_trace_and_dispatch(handler_, trace_info, funds_update, true);
      if (!std::isnan(item.cross_wallet_balance)) {
        auto funds_update = FundsUpdate{
            .stream_id = stream_id_,
            .account = account_.name,
            .currency = item.asset,
            .margin_mode = MarginMode::CROSS,
            .balance = item.cross_wallet_balance,
            .hold = NaN,  // note! we don't see this
            .external_account = {},
            .update_type = UpdateType::INCREMENTAL,
            .exchange_time_utc = account_update.transaction_time,
            .sending_time_utc = account_update.event_time,
        };
        create_trace_and_dispatch(handler_, trace_info, funds_update, true);
      }
    }
    for (auto &item : account_update.data.positions) {
      if (shared_.discard_symbol(item.symbol))
        continue;
      log::info<2>("item={}"sv, item);
      auto margin_mode = [&]() {
        switch (item.margin_type) {
          using enum json::MarginType::type_t;
          case UNDEFINED__:
          case UNKNOWN__:
            break;
          case ISOLATED:
            return MarginMode::ISOLATED;
          case CROSS:
            return MarginMode::CROSS;
        }
        return MarginMode{};
      }();
      auto long_quantity = std::max(0.0, item.position_amount);
      auto short_quantity = std::max(0.0, -item.position_amount);
      auto position_update = PositionUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .margin_mode = margin_mode,
          .external_account{},
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = account_update.transaction_time,
          .sending_time_utc = account_update.event_time,
      };
      create_trace_and_dispatch(handler_, trace_info, position_update, true);
    }
  });
}

void DropCopySimple::operator()(Trace<json::MarginCall> const &event) {
  profile_.margin_call([&]() {
    auto &[trace_info, margin_call] = event;
    log::info<2>("margin_call={}"sv, margin_call);
  });
}

void DropCopySimple::operator()(Trace<json::StrategyUpdate> const &event) {
  profile_.strategy_update([&]() {
    auto &[trace_info, strategy_update] = event;
    log::info<2>("strategy_update={}"sv, strategy_update);
  });
}

void DropCopySimple::operator()(Trace<json::GridUpdate> const &event) {
  profile_.grid_update([&]() {
    auto &[trace_info, grid_update] = event;
    log::info<2>("grid_update={}"sv, grid_update);
  });
}

void DropCopySimple::operator()(Trace<json::AccountConfigUpdate> const &event) {
  profile_.account_config_update([&]() {
    auto &[trace_info, account_config_update] = event;
    log::info<2>("account_config_update={}"sv, account_config_update);
  });
}

void DropCopySimple::operator()(Trace<json::TradeLite> const &event) {
  profile_.trade_lite([&]() {
    auto &[trace_info, trade_lite] = event;
    log::info<2>("trade_lite={}"sv, trade_lite);
    log::warn("DEBUG trade_lite={}"sv, trade_lite);
  });
}

// request

void DropCopySimple::request_balance() {
  log::info("Requesting balance download..."sv);
  request_.request_balance = clock::get_system();
}

void DropCopySimple::request_account() {
  log::info("Requesting account download..."sv);
  request_.request_account = clock::get_system();
}

void DropCopySimple::request_orders() {
  log::info("Requesting order download..."sv);
  request_.request_orders = clock::get_system();
}

void DropCopySimple::request_trades() {
  log::info("Requesting trades download..."sv);
  request_.request_trades = clock::get_system();
}

// response

void DropCopySimple::check_response_balance() {
  if (download_.state() != DropCopyState::BALANCE)
    return;
  if (request_.request_balance < request_.respond_balance) {
    log::info("Balance download has completed!"sv);
    download_.check(DropCopyState::BALANCE);
  }
}

void DropCopySimple::check_response_account() {
  if (download_.state() != DropCopyState::ACCOUNT)
    return;
  if (request_.request_account < request_.respond_account) {
    log::info("Account download has completed!"sv);
    download_.check(DropCopyState::ACCOUNT);
  }
}

void DropCopySimple::check_response_orders() {
  if (download_.state() != DropCopyState::ORDERS)
    return;
  if (request_.request_orders < request_.respond_orders) {
    log::info("Order download has completed!"sv);
    download_.check(DropCopyState::ORDERS);
  }
}

void DropCopySimple::check_response_trades() {
  if (download_.state() != DropCopyState::TRADES)
    return;
  if (request_.request_trades < request_.respond_trades) {
    log::info("Trades download has completed!"sv);
    download_.check(DropCopyState::TRADES);
  }
}

}  // namespace binance_futures
}  // namespace roq
