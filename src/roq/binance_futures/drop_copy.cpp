/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/drop_copy.hpp"

#include <algorithm>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

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

auto create_uri(auto &settings, auto const &listen_key) {
  assert(!std::empty(listen_key));
  auto &uri = settings.ws.uri;
  auto result = fmt::format("{}://{}{}/{}"sv, uri.get_scheme(), uri.get_host(), uri.get_path(), listen_key);
  return io::web::URI{result};
}

auto create_connection(auto &handler, auto &settings, auto &context, auto const &listen_key) {
  auto uri = create_uri(settings, listen_key);
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
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
      .decode_buffer_size = settings.common.decode_buffer_size,
      .encode_buffer_size = settings.common.encode_buffer_size,
  };
  return web::socket::ClientFactory::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto &settings, auto const &group, auto const &function)
      : core::metrics::Factory(settings.app.name, group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(
    Handler &handler,
    io::Context &context,
    uint16_t stream_id,
    Account &account,
    Shared &shared,
    Request &request,
    std::string_view const &listen_key)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)},
      connection_{create_connection(*this, shared.settings, context, listen_key)},
      decode_buffer_(shared.settings.common.decode_buffer_size),
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
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared}, request_{request},
      download_{{}, [this](auto state) { return download(state); }} {
}

bool DropCopy::ready() const {
  return (*connection_).ready();
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
  check_response_balance();
  check_response_account();
  check_response_orders();
  check_response_trades();
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.order_trade_update, metrics::PROFILE)
      .write(profile_.account_update, metrics::PROFILE)
      .write(profile_.margin_call, metrics::PROFILE)
      .write(profile_.strategy_update, metrics::PROFILE)
      .write(profile_.grid_update, metrics::PROFILE)
      .write(profile_.account_config_update, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.get_name(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.get_name(),
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

uint32_t DropCopy::download(DropCopyState state) {
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
      if (shared_.settings.common.download_trades_lookback.count() &&
          !std::empty(shared_.settings.common.download_symbols)) {
        request_trades();
        return 1;
      } else {
        return {};
      }
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    try {
      log::debug(R"(message="{}")"sv, message);
      TraceInfo trace_info;
      json::UserStreamParser::dispatch(
          *this, message, decode_buffer_, trace_info, shared_.settings.common.continue_with_unknown_event_type);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(Trace<json::OrderTradeUpdate> const &event) {
  profile_.order_trade_update([&]() {
    auto &trace_info = event.trace_info;
    auto &order_trade_update = event.value;
    log::debug("order_trade_update={}"sv, order_trade_update);
    log::info<3>("order_trade_update={}"sv, order_trade_update);
    auto &execution_report = order_trade_update.execution_report;
    auto side = json::map(execution_report.side);
    auto external_order_id = fmt::format("{}"sv, execution_report.order_id);
    auto order_status = json::map(execution_report.order_status);
    auto order_type = json::map(execution_report.order_type);
    auto time_in_force = json::map(execution_report.time_in_force);
    auto liquidity = execution_report.is_trade_maker ? Liquidity::MAKER : Liquidity::TAKER;
    // XXX HANS execution_report.execution_type ==> OrderAck ???
    auto order_update = oms::OrderUpdate{
        .account = account_.get_name(),
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = side,
        .position_effect = {},
        .max_show_quantity = NaN,
        .order_type = order_type,
        .time_in_force = time_in_force,
        .execution_instructions = {},
        .create_time_utc = {},
        .update_time_utc = order_trade_update.transaction_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .status = order_status,
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
    };
    fmt::format_to(std::back_inserter(fill.external_trade_id), "{}"sv, execution_report.trade_id);
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.get_name(),
        .order_id = order_id,
        .exchange = shared_.settings.exchange,
        .symbol = execution_report.symbol,
        .side = side,
        .position_effect = {},
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

void DropCopy::operator()(Trace<json::AccountUpdate> const &event) {
  profile_.account_update([&]() {
    auto &[trace_info, account_update] = event;
    log::info<2>("account_update={}"sv, account_update);
    for (auto &item : account_update.data.balances) {
      log::debug("item={}"sv, item);
      auto funds_update = FundsUpdate{
          .stream_id = stream_id_,
          .account = account_.get_name(),
          .margin_mode = {},
          .currency = item.asset,
          .balance = item.wallet_balance,
          .hold = NaN,  // note! we don't see this
          .external_account = {},
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = account_update.transaction_time,
          .sending_time_utc = account_update.event_time,
      };
      create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    }
    for (auto &item : account_update.data.positions) {
      if (shared_.discard_symbol(item.symbol))
        continue;
      log::debug("item={}"sv, item);
      auto long_quantity = std::max(0.0, item.position_amount);
      auto short_quantity = std::max(0.0, -item.position_amount);
      auto position_update = PositionUpdate{
          .stream_id = stream_id_,
          .account = account_.get_name(),
          .margin_mode = {},
          .exchange = shared_.settings.exchange,
          .symbol = item.symbol,
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

void DropCopy::operator()(Trace<json::MarginCall> const &event) {
  profile_.margin_call([&]() {
    auto &[trace_info, margin_call] = event;
    log::debug("margin_call={}"sv, margin_call);
  });
}

void DropCopy::operator()(Trace<json::StrategyUpdate> const &event) {
  profile_.strategy_update([&]() {
    auto &[trace_info, strategy_update] = event;
    log::debug("strategy_update={}"sv, strategy_update);
  });
}

void DropCopy::operator()(Trace<json::GridUpdate> const &event) {
  profile_.grid_update([&]() {
    auto &[trace_info, grid_update] = event;
    log::debug("grid_update={}"sv, grid_update);
  });
}

void DropCopy::operator()(Trace<json::AccountConfigUpdate> const &event) {
  profile_.account_config_update([&]() {
    auto &[trace_info, account_config_update] = event;
    log::debug("account_config_update={}"sv, account_config_update);
  });
}
void DropCopy::request_balance() {
  log::info("Requesting balance download..."sv);
  request_.request_balance = clock::get_system();
}

void DropCopy::check_response_balance() {
  if (download_.state() != DropCopyState::BALANCE)
    return;
  if (request_.request_balance < request_.respond_balance) {
    log::info("Balance download has completed!"sv);
    download_.check(DropCopyState::BALANCE);
  }
}

void DropCopy::request_account() {
  log::info("Requesting account download..."sv);
  request_.request_account = clock::get_system();
}

void DropCopy::check_response_account() {
  if (download_.state() != DropCopyState::ACCOUNT)
    return;
  if (request_.request_account < request_.respond_account) {
    log::info("Account download has completed!"sv);
    download_.check(DropCopyState::ACCOUNT);
  }
}

void DropCopy::request_orders() {
  log::info("Requesting order download..."sv);
  request_.request_orders = clock::get_system();
}

void DropCopy::check_response_orders() {
  if (download_.state() != DropCopyState::ORDERS)
    return;
  if (request_.request_orders < request_.respond_orders) {
    log::info("Order download has completed!"sv);
    download_.check(DropCopyState::ORDERS);
  }
}

void DropCopy::request_trades() {
  log::info("Requesting trades download..."sv);
  request_.request_trades = clock::get_system();
}

void DropCopy::check_response_trades() {
  if (download_.state() != DropCopyState::TRADES)
    return;
  if (request_.request_trades < request_.respond_trades) {
    log::info("Trades download has completed!"sv);
    download_.check(DropCopyState::TRADES);
  }
}

}  // namespace binance_futures
}  // namespace roq
