/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/drop_copy.hpp"

#include <algorithm>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

#include "roq/binance_futures/flags.hpp"

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

auto create_uri(auto const &listen_key) {
  assert(!std::empty(listen_key));
  auto &uri = Flags::ws_uri();
  auto result = fmt::format("{}://{}{}/{}"sv, uri.get_scheme(), uri.get_host(), uri.get_path(), listen_key);
  return io::web::URI{result};
}

auto create_connection(auto &handler, auto &context, auto const &listen_key) {
  auto uri = create_uri(listen_key);
  auto config = web::socket::Client::Config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = {},
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return web::socket::ClientFactory::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(
    Handler &handler,
    io::Context &context,
    uint16_t stream_id,
    Authenticator &authenticator,
    Shared &shared,
    Request &request,
    std::string_view const &listen_key)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)},
      connection_{create_connection(*this, context, listen_key)}, decode_buffer_{Flags::decode_buffer_size()},
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .order_trade_update = create_metrics(name_, "order_trade_update"sv),
          .account_update = create_metrics(name_, "account_update"sv),
          .margin_call = create_metrics(name_, "margin_call"sv),
          .strategy_update = create_metrics(name_, "strategy_update"sv),
          .grid_update = create_metrics(name_, "grid_update"sv),
          .account_config_update = create_metrics(name_, "account_config_update"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      authenticator_{authenticator}, shared_{shared}, request_{request}, download_{{}, [this](auto state) {
                                                                                     return download(state);
                                                                                   }} {
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
      .account = authenticator_.get_account(),
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
        .account = authenticator_.get_account(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
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
      if (flags::Flags::download_trades() && !std::empty(flags::Flags::download_symbols())) {
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
      core::json::Buffer buffer{decode_buffer_};
      json::UserStreamParser::dispatch(*this, message, buffer, trace_info);
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
    auto order_update = oms::OrderUpdate{
        .account = authenticator_.get_account(),
        .exchange = Flags::exchange(),
        .symbol = execution_report.symbol,
        .side = side,
        .position_effect = {},
        .max_show_quantity = NaN,
        .order_type = order_type,
        .time_in_force = time_in_force,
        .execution_instructions = {},
        .order_template = {},
        .create_time_utc = {},
        .update_time_utc = utils::safe_cast(order_trade_update.event_time),
        .external_account = {},
        .external_order_id = external_order_id,
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
        .update_type = UpdateType::INCREMENTAL,
    };
    auto is_trade = execution_report.execution_type == json::ExecutionType::TRADE;
    auto create_fill = [&](auto &execution_report) {
      auto result = Fill{
          .external_trade_id = {},
          .quantity = execution_report.last_filled_quantity,
          .price = execution_report.last_filled_price,
          .liquidity = liquidity,
      };
      fmt::format_to(std::back_inserter(result.external_trade_id), "{}"sv, execution_report.trade_id);
      return result;
    };
    if (shared_.update_order(execution_report.client_order_id, stream_id_, trace_info, order_update, [&](auto &order) {
          if (is_trade) {
            auto fill = create_fill(execution_report);
            auto trade_update = oms::TradeUpdate{
                .account = order.account,
                .order_id = order.order_id,
                .exchange = order.exchange,
                .symbol = order.symbol,
                .side = order.side,
                .position_effect = order.position_effect,
                .create_time_utc = utils::safe_cast(execution_report.order_trade_time),
                .update_time_utc = utils::safe_cast(execution_report.order_trade_time),
                .external_account = order.external_account,
                .external_order_id = order.external_order_id,
                .fills = {&fill, 1},
                .update_type = {},
            };
            create_trace_and_dispatch(handler_, trace_info, trade_update, stream_id_, true, order.user_id);
          }
        })) {
    } else {
      if (is_trade) {
        auto fill = create_fill(execution_report);
        auto trade_update = oms::TradeUpdate{
            .account = authenticator_.get_account(),
            .order_id = ORDER_ID_NONE,
            .exchange = Flags::exchange(),
            .symbol = execution_report.symbol,
            .side = side,
            .position_effect = {},
            .create_time_utc = utils::safe_cast(execution_report.order_trade_time),
            .update_time_utc = utils::safe_cast(execution_report.order_trade_time),
            .external_account = {},
            .external_order_id = external_order_id,
            .fills = {&fill, 1},
            .update_type = {},
        };
        create_trace_and_dispatch(handler_, trace_info, trade_update, stream_id_, true, SOURCE_SELF);
      } else {
        log::warn("*** EXTERNAL ORDER ***"sv);
        log::warn("execution_report={}"sv, execution_report);
      }
    }
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
          .account = authenticator_.get_account(),
          .currency = item.asset,
          .balance = item.wallet_balance,
          .hold = NaN,  // note! we don't see this
          .external_account = {},
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
          .account = authenticator_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.symbol,
          .external_account{},
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .long_quantity_begin = NaN,
          .short_quantity_begin = NaN,
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
