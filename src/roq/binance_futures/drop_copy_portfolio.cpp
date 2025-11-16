/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/drop_copy_portfolio.hpp"

#include <algorithm>

#include "roq/mask.hpp"

#include "roq/utils/common.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/binance_futures/json/map.hpp"
#include "roq/binance_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

#define NEW_FUNDS_UPDATE 1

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

size_t const MAX_DECODE_BUFFER_DEPTH = 2;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_uri(auto &settings, auto &listen_key) {
  assert(!std::empty(listen_key));
  auto &uri = settings.ws.pm_uri;
  auto result = fmt::format("{}://{}{}/{}"sv, uri.get_scheme(), uri.get_host(), uri.get_path(), listen_key);
  return io::web::URI{result};
}

auto create_connection(auto &handler, auto &settings, auto &context, auto &listen_key) {
  auto uri = create_uri(settings, listen_key);
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = settings.ws.pm_host,
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
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopyPortfolio::DropCopyPortfolio(
    Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared, Request &request, std::string_view const &listen_key)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context, listen_key)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
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
          .execution_report = create_metrics(shared.settings, name_, "execution_report"sv),
          .balance_update = create_metrics(shared.settings, name_, "balance_update"sv),
          .liability_change = create_metrics(shared.settings, name_, "liability_change"sv),
          .outbound_account_position = create_metrics(shared.settings, name_, "outbound_account_position"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared}, request_{request}, download_{{}, [this](auto state) { return download(state); }} {
}

bool DropCopyPortfolio::ready() const {
  return (*connection_).ready();
}

void DropCopyPortfolio::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopyPortfolio::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopyPortfolio::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
  check_response_balance();
  check_response_account();
  check_response_position();
  check_response_orders();
  check_response_trades();
}

void DropCopyPortfolio::operator()(metrics::Writer &writer) const {
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
      .write(profile_.execution_report, metrics::Type::PROFILE)
      .write(profile_.balance_update, metrics::Type::PROFILE)
      .write(profile_.liability_change, metrics::Type::PROFILE)
      .write(profile_.outbound_account_position, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopyPortfolio::operator()(web::socket::Client::Connected const &) {
}

void DropCopyPortfolio::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void DropCopyPortfolio::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopyPortfolio::operator()(web::socket::Client::Close const &) {
}

void DropCopyPortfolio::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopyPortfolio::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopyPortfolio::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopyPortfolio::operator()(ConnectionStatus status) {
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

uint32_t DropCopyPortfolio::download(DropCopyPortfolioState state) {
  switch (state) {
    using enum DropCopyPortfolioState;
    case UNDEFINED:
      assert(false);
      break;
    case BALANCE:
      request_balance();
      return 1;
    case ACCOUNT:
      request_account();
      return 1;
    case POSITION:
      request_position();
      return 1;
    case ORDERS:
      request_orders();
      return 1;
    case TRADES:
      if (shared_.settings.download.trades_lookback.count() != 0 && !std::empty(shared_.settings.download.symbols)) {
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

void DropCopyPortfolio::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::UserStreamParser::dispatch(*this, message, decode_buffer_, trace_info, shared_.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void DropCopyPortfolio::operator()(Trace<json::OrderTradeUpdate> const &event) {
  profile_.order_trade_update([&]() {
    auto &[trace_info, order_trade_update] = event;
    log::info<3>("order_trade_update={}"sv, order_trade_update);
    auto &execution_report = order_trade_update.execution_report;
    auto external_order_id = fmt::format("{}"sv, execution_report.order_id);
    auto liquidity = execution_report.is_trade_maker ? Liquidity::MAKER : Liquidity::TAKER;
    // XXX HANS execution_report.execution_type ==> OrderAck ???
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = map(execution_report.side),
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
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
        .leverage = NaN,
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
      log::warn<2>("DEBUG: execution_report={}"sv, execution_report);
    }
    if (execution_report.execution_type != json::ExecutionType::TRADE) {
      return;
    }
    auto side = map(execution_report.side).template get<Side>();
    auto ref_data = shared_.get_ref_data(shared_.settings.exchange, execution_report.symbol);
    auto profit_loss_amount =
        utils::compute_profit_loss_amount(side, execution_report.last_filled_quantity, execution_report.last_filled_price, ref_data.multiplier);
    auto fill = Fill{
        .exchange_time_utc = execution_report.order_trade_time,
        .external_trade_id = {},
        .quantity = execution_report.last_filled_quantity,
        .price = execution_report.last_filled_price,
        .liquidity = liquidity,
        .commission_amount = execution_report.commission,
        .commission_currency = execution_report.commission_asset,
        .base_amount = NaN,
        .quote_amount = NaN,
        .profit_loss_amount = profit_loss_amount,
    };
    fmt::format_to(std::back_inserter(fill.external_trade_id), "{}"sv, execution_report.trade_id);
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = order_id,
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = side,
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
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

void DropCopyPortfolio::operator()(Trace<json::AccountUpdate> const &event) {
  profile_.account_update([&]() {
    auto &[trace_info, account_update] = event;
    log::info<2>("account_update={}"sv, account_update);
    log::warn("DEBUG account_update={}"sv, account_update);
    if (!shared_.settings.misc.test_alt_funds_update) {
      for (auto &item : account_update.data.balances) {
        log::info<2>("item={}"sv, item);
        auto funds_update = FundsUpdate{
            .stream_id = stream_id_,
            .account = account_.name,
            .currency = item.asset,
            .margin_mode = MarginMode::PORTFOLIO,
            .balance = item.wallet_balance,
            .hold = NaN,  // note! we don't see this
            .borrowed = NaN,
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
              .borrowed = NaN,
              .external_account = {},
              .update_type = UpdateType::INCREMENTAL,
              .exchange_time_utc = account_update.transaction_time,
              .sending_time_utc = account_update.event_time,
          };
          create_trace_and_dispatch(handler_, trace_info, funds_update, true);
        }
      }
    }
    for (auto &item : account_update.data.positions) {
      if (shared_.discard_symbol(item.symbol)) {
        continue;
      }
      log::info<2>("item={}"sv, item);
      auto long_quantity = std::max(0.0, item.position_amount);
      auto short_quantity = std::max(0.0, -item.position_amount);
      auto position_update = PositionUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
          .margin_mode = MarginMode::PORTFOLIO,
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

void DropCopyPortfolio::operator()(Trace<json::MarginCall> const &event) {
  profile_.margin_call([&]() {
    auto &[trace_info, margin_call] = event;
    log::info<2>("margin_call={}"sv, margin_call);
  });
}

void DropCopyPortfolio::operator()(Trace<json::StrategyUpdate> const &event) {
  profile_.strategy_update([&]() {
    auto &[trace_info, strategy_update] = event;
    log::info<2>("strategy_update={}"sv, strategy_update);
  });
}

void DropCopyPortfolio::operator()(Trace<json::GridUpdate> const &event) {
  profile_.grid_update([&]() {
    auto &[trace_info, grid_update] = event;
    log::info<2>("grid_update={}"sv, grid_update);
  });
}

void DropCopyPortfolio::operator()(Trace<json::AccountConfigUpdate> const &event) {
  profile_.account_config_update([&]() {
    auto &[trace_info, account_config_update] = event;
    log::info<2>("account_config_update={}"sv, account_config_update);
  });
}

void DropCopyPortfolio::operator()(Trace<json::TradeLite> const &event) {
  profile_.trade_lite([&]() {
    auto &[trace_info, trade_lite] = event;
    log::info<2>("trade_lite={}"sv, trade_lite);
    log::warn("DEBUG trade_lite={}"sv, trade_lite);
  });
}

void DropCopyPortfolio::operator()(Trace<json::ExecutionReport2> const &event) {
  profile_.execution_report([&]() {
    auto &[trace_info, execution_report] = event;
    log::info<2>("execution_report={}"sv, execution_report);
    auto external_order_id = fmt::format("{}"sv, execution_report.order_id);
    auto liquidity = execution_report.is_trade_maker ? Liquidity::MAKER : Liquidity::TAKER;
    // XXX HANS execution_report.execution_type ==> OrderAck ???
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = map(execution_report.side),
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
        .max_show_quantity = NaN,
        .order_type = map(execution_report.order_type),
        .time_in_force = map(execution_report.time_in_force),
        .execution_instructions = {},
        .create_time_utc = execution_report.order_creation_time,
        .update_time_utc = execution_report.transaction_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = execution_report.client_order_id,
        .order_status = map(execution_report.order_status),
        .quantity = execution_report.original_quantity,
        .price = execution_report.original_price,
        .stop_price = execution_report.stop_price,
        .leverage = NaN,
        .remaining_quantity = NaN,
        .traded_quantity = execution_report.order_filled_accumulated_quantity,
        .average_traded_price = NaN,
        .last_traded_quantity = execution_report.last_filled_quantity,
        .last_traded_price = execution_report.last_filled_price,
        .last_liquidity = liquidity,
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::INCREMENTAL,
        .sending_time_utc = execution_report.event_time,
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
      log::warn<2>("DEBUG: execution_report={}"sv, execution_report);
    }
    if (execution_report.execution_type != json::ExecutionType::TRADE) {
      return;
    }
    auto side = map(execution_report.side).template get<Side>();
    auto ref_data = shared_.get_ref_data(shared_.settings.exchange, execution_report.symbol);
    auto profit_loss_amount =
        utils::compute_profit_loss_amount(side, execution_report.last_filled_quantity, execution_report.last_filled_price, ref_data.multiplier);
    auto fill = Fill{
        .exchange_time_utc = execution_report.transaction_time,
        .external_trade_id = {},
        .quantity = execution_report.last_filled_quantity,
        .price = execution_report.last_filled_price,
        .liquidity = liquidity,
        .commission_amount = execution_report.commission,
        .commission_currency = execution_report.commission_asset,
        .base_amount = NaN,
        .quote_amount = NaN,
        .profit_loss_amount = profit_loss_amount,
    };
    fmt::format_to(std::back_inserter(fill.external_trade_id), "{}"sv, execution_report.trade_id);
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = order_id,
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = side,
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
        .quantity_type = {},
        .create_time_utc = execution_report.transaction_time,
        .update_time_utc = execution_report.transaction_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .fills = {&fill, 1},
        .routing_id = {},
        .update_type = UpdateType::INCREMENTAL,
        .sending_time_utc = execution_report.event_time,
        .user = {},
        .strategy_id = strategy_id,
    };
    create_trace_and_dispatch(handler_, trace_info, trade_update, true, user_id, execution_report.client_order_id);
  });
}

void DropCopyPortfolio::operator()(Trace<json::BalanceUpdate> const &event) {
  profile_.balance_update([&]() {
    auto &[trace_info, balance_update] = event;
    log::info<2>("balance_update={}"sv, balance_update);
    log::warn("DEBUG balance_update={}"sv, balance_update);
  });
}

void DropCopyPortfolio::operator()(Trace<json::LiabilityChange> const &event) {
  profile_.liability_change([&]() {
    auto &[trace_info, liability_change] = event;
    log::info<2>("liability_change={}"sv, liability_change);
    log::warn("DEBUG liability_change={}"sv, liability_change);
  });
}

void DropCopyPortfolio::operator()(Trace<json::OutboundAccountPosition> const &event) {
  profile_.outbound_account_position([&]() {
    auto &[trace_info, outbound_account_position] = event;
    log::info<2>("outbound_account_position={}"sv, outbound_account_position);
    log::warn("DEBUG outbound_account_position={}"sv, outbound_account_position);
    if (shared_.settings.misc.test_alt_funds_update) {
      for (auto &item : outbound_account_position.balances) {
        log::info<2>("item={}"sv, item);
        auto funds_update = FundsUpdate{
            .stream_id = stream_id_,
            .account = account_.name,
            .currency = item.asset,
            .margin_mode = MarginMode::PORTFOLIO,
            .balance = item.free,
            .hold = item.locked,
            .borrowed = NaN,
            .external_account = {},
            .update_type = UpdateType::INCREMENTAL,
            .exchange_time_utc = outbound_account_position.last_account_update,  // ???
            .sending_time_utc = outbound_account_position.event_time,
        };
        create_trace_and_dispatch(handler_, trace_info, funds_update, true);
      }
    }
  });
}

// request

void DropCopyPortfolio::request_balance() {
  log::info("Requesting balance download..."sv);
  request_.request_balance = clock::get_system();
}

void DropCopyPortfolio::request_account() {
  log::info("Requesting account download..."sv);
  request_.request_account = clock::get_system();
}

void DropCopyPortfolio::request_position() {
  log::info("Requesting position download..."sv);
  request_.request_position = clock::get_system();
}

void DropCopyPortfolio::request_orders() {
  log::info("Requesting order download..."sv);
  request_.request_orders = clock::get_system();
}

void DropCopyPortfolio::request_trades() {
  log::info("Requesting trades download..."sv);
  request_.request_trades = clock::get_system();
}

// response

void DropCopyPortfolio::check_response_balance() {
  if (download_.state() != DropCopyPortfolioState::BALANCE) {
    return;
  }
  if (request_.request_balance < request_.respond_balance) {
    log::info("Balance download has completed!"sv);
    download_.check(DropCopyPortfolioState::BALANCE);
  }
}

void DropCopyPortfolio::check_response_account() {
  if (download_.state() != DropCopyPortfolioState::ACCOUNT) {
    return;
  }
  if (request_.request_account < request_.respond_account) {
    log::info("Account download has completed!"sv);
    download_.check(DropCopyPortfolioState::ACCOUNT);
  }
}

void DropCopyPortfolio::check_response_position() {
  if (download_.state() != DropCopyPortfolioState::POSITION) {
    return;
  }
  if (request_.request_position < request_.respond_position) {
    log::info("Position download has completed!"sv);
    download_.check(DropCopyPortfolioState::POSITION);
  }
}

void DropCopyPortfolio::check_response_orders() {
  if (download_.state() != DropCopyPortfolioState::ORDERS) {
    return;
  }
  if (request_.request_orders < request_.respond_orders) {
    log::info("Order download has completed!"sv);
    download_.check(DropCopyPortfolioState::ORDERS);
  }
}

void DropCopyPortfolio::check_response_trades() {
  if (download_.state() != DropCopyPortfolioState::TRADES) {
    return;
  }
  if (request_.request_trades < request_.respond_trades) {
    log::info("Trades download has completed!"sv);
    download_.check(DropCopyPortfolioState::TRADES);
  }
}

}  // namespace binance_futures
}  // namespace roq
