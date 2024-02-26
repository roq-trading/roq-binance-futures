/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/order_entry_simple.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/utils/charconv.hpp"
#include "roq/utils/compare.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/binance_futures/json/error.hpp"
#include "roq/binance_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
    SupportType::POSITION,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.uri;
  auto ping_path = fmt::format("/{}{}"sv, settings.app.api, settings.rest.ping_path);
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = settings.rest.host,
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = false,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = true,
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = ping_path,
      // implementation
      .decode_buffer_size = settings.common.decode_buffer_size,
      .encode_buffer_size = settings.common.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public core::metrics::Factory {
  create_metrics(auto &settings, auto const &group, auto const &function)
      : core::metrics::Factory(settings.app.name, group, function) {}
  create_metrics(auto &settings, auto const &group, auto const &function, auto const &period)
      : core::metrics::Factory(settings.app.name, group, function, period) {}
};

auto get_download_trades_lookback(auto const &settings, auto download_trades_is_first) {
  if (download_trades_is_first) {
    if (settings.common.download_trades_lookback_on_restart.count())
      return settings.common.download_trades_lookback_on_restart;
  }
  return settings.common.download_trades_lookback;
}
}  // namespace

// === IMPLEMENTATION ===

OrderEntrySimple::OrderEntrySimple(
    Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared, Request &request)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)},
      connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_(shared.settings.common.decode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .listen_key = create_metrics(shared.settings, name_, "listen_key"sv),
          .listen_key_ack = create_metrics(shared.settings, name_, "listen_key_ack"sv),
          .balance = create_metrics(shared.settings, name_, "balance"sv),
          .balance_ack = create_metrics(shared.settings, name_, "balance_ack"sv),
          .account = create_metrics(shared.settings, name_, "account"sv),
          .account_ack = create_metrics(shared.settings, name_, "account_ack"sv),
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .open_orders_ack = create_metrics(shared.settings, name_, "open_orders_ack"sv),
          .trades = create_metrics(shared.settings, name_, "trades"sv),
          .trades_ack = create_metrics(shared.settings, name_, "trades_ack"sv),
          .new_order = create_metrics(shared.settings, name_, "new_order"sv),
          .new_order_ack = create_metrics(shared.settings, name_, "new_order_ack"sv),
          .modify_order = create_metrics(shared.settings, name_, "modify_order"sv),
          .modify_order_ack = create_metrics(shared.settings, name_, "modify_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
          .cancel_all_open_orders = create_metrics(shared.settings, name_, "cancel_all_open_orders"sv),
          .cancel_all_open_orders_ack = create_metrics(shared.settings, name_, "cancel_all_open_orders_ack"sv),
          .auto_cancel_all_open_orders = create_metrics(shared.settings, name_, "auto_cancel_all_open_orders"sv),
          .auto_cancel_all_open_orders_ack =
              create_metrics(shared.settings, name_, "auto_cancel_all_open_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      rate_limiter_{
          .request_weight_1m = create_metrics(shared.settings, name_, "request_weight"sv, "1m"sv),
          .create_order_1m = create_metrics(shared.settings, name_, "create_order"sv, "1m"sv),
      },
      account_{account}, shared_{shared}, request_{request},
      download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void OrderEntrySimple::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntrySimple::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntrySimple::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  refresh_listen_key();
  if (shared_.settings.rest.cancel_on_disconnect && shared_.settings.rest.order_countdown.count() &&
      next_auto_cancel_ < now) {
    next_auto_cancel_ = now + shared_.settings.rest.order_countdown / 4;
    auto_cancel_all_open_orders();
  }
  if (ready() && !downloading()) {
    if (!downloading() && request_.respond_balance < request_.request_balance) {
      log::info<1>("Download balance..."sv);
      get_balance();
      download_balance_ = true;
    }
    if (!downloading() && request_.respond_account < request_.request_account) {
      log::info<1>("Download account..."sv);
      get_account();
      download_account_ = true;
    }
    if (!downloading() && request_.respond_orders < request_.request_orders) {
      log::info<1>("Download orders..."sv);
      get_open_orders();
      download_orders_ = true;
    }
    if (!downloading() && request_.respond_trades < request_.request_trades) {
      log::info<1>("Download trades..."sv);
      get_trades();
      download_trades_ = true;
    }
  }
}

void OrderEntrySimple::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.listen_key, metrics::Type::PROFILE)
      .write(profile_.listen_key_ack, metrics::Type::PROFILE)
      .write(profile_.balance, metrics::Type::PROFILE)
      .write(profile_.balance_ack, metrics::Type::PROFILE)
      .write(profile_.account, metrics::Type::PROFILE)
      .write(profile_.account_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.open_orders_ack, metrics::Type::PROFILE)
      .write(profile_.trades, metrics::Type::PROFILE)
      .write(profile_.trades_ack, metrics::Type::PROFILE)
      .write(profile_.new_order, metrics::Type::PROFILE)
      .write(profile_.new_order_ack, metrics::Type::PROFILE)
      .write(profile_.modify_order, metrics::Type::PROFILE)
      .write(profile_.modify_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_all_open_orders, metrics::Type::PROFILE)
      .write(profile_.cancel_all_open_orders_ack, metrics::Type::PROFILE)
      .write(profile_.auto_cancel_all_open_orders, metrics::Type::PROFILE)
      .write(profile_.auto_cancel_all_open_orders_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      // rate limiter
      .write(rate_limiter_.request_weight_1m, metrics::Type::RATE_LIMITER)
      .write(rate_limiter_.create_order_1m, metrics::Type::RATE_LIMITER);
}

uint16_t OrderEntrySimple::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  new_order(event, order, request_id);
  return stream_id_;
}

uint16_t OrderEntrySimple::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  modify_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntrySimple::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntrySimple::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  cancel_all_open_orders(event, request_id);
  return stream_id_;
}

void OrderEntrySimple::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntrySimple::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
  download_balance_ = false;
  download_account_ = false;
  download_orders_ = false;
  download_trades_ = false;
}

void OrderEntrySimple::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntrySimple::operator()(Trace<web::rest::Client::MessageBegin> const &) {
  shared_.rate_limits.clear();
}

void OrderEntrySimple::operator()(Trace<web::rest::Client::Header> const &event) {
  auto &header = event.value;
  if (utils::case_insensitive_compare(header.name, "x-mbx-used-weight-1m"sv) == 0) {
    try {
      auto value = utils::from_string_relaxed<uint32_t>(header.value);
      auto rate_limit = RateLimit{
          .type = RateLimitType::REQUEST_WEIGHT,
          .period = 1min,
          .end_time_utc = {},
          .limit = shared_.limits.request_weight_1m,
          .value = value,
      };
      shared_.rate_limits.emplace_back(std::move(rate_limit));
      rate_limiter_.request_weight_1m.set(value);
    } catch (RuntimeError &) {
      log::warn<5>(R"(Failed to parse text="{}")"sv, header.value);
    }
  }
  if (utils::case_insensitive_compare(header.name, "x-mbx-order-count-1m"sv) == 0) {
    try {
      auto value = utils::from_string_relaxed<uint32_t>(header.value);
      auto rate_limit = RateLimit{
          .type = RateLimitType::CREATE_ORDER,
          .period = 1min,
          .end_time_utc = {},
          .limit = shared_.limits.create_order_1m,
          .value = value,
      };
      shared_.rate_limits.emplace_back(std::move(rate_limit));
      rate_limiter_.create_order_1m.set(value);
    } catch (RuntimeError &) {
      log::warn<5>(R"(Failed to parse text="{}")"sv, header.value);
    }
  }
}

void OrderEntrySimple::operator()(Trace<web::rest::Client::MessageEnd> const &event) {
  auto &trace_info = event.trace_info;
  if (std::empty(shared_.rate_limits))
    return;
  auto rate_limits_update = RateLimitsUpdate{
      .stream_id = stream_id_,
      .account = account_.name,
      .origin = Origin::EXCHANGE,
      .rate_limits = shared_.rate_limits,
  };
  create_trace_and_dispatch(handler_, trace_info, rate_limits_update);
  shared_.rate_limits.clear();
}

void OrderEntrySimple::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
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

uint32_t OrderEntrySimple::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case LISTEN_KEY:
      get_listen_key();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// listen-key

void OrderEntrySimple::get_listen_key() {
  profile_.listen_key([&]() {
    auto headers = account_.create_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = shared_.api.get_listen_key,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_listen_key_ack(event, sequence);
    };
    (*connection_)("listen_key"sv, request, callback);
  });
}

void OrderEntrySimple::get_listen_key_ack(Trace<web::rest::Response> const &event, [[maybe_unused]] uint32_t sequence) {
  constexpr auto const STATE = OrderEntryState::LISTEN_KEY;
  profile_.listen_key_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::ListenKey listen_key{body};
      log::debug("listen_key={}"sv, listen_key);
      Trace event_2{event, listen_key};
      (*this)(event_2);
      download_.check_relaxed(STATE);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      if (download_.downloading())
        download_.retry(STATE);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(Trace<json::ListenKey> const &event) {
  auto &[trace_info, listen_key] = event;
  log::info<2>("listen_key={}"sv, listen_key);
  bool initial = std::empty(listen_key_);
  if (utils::update(listen_key_, listen_key.listen_key)) {
    if (initial) {
      log::info(R"(Listen key has been acquired (value="{}"))"sv, listen_key_);
      auto listen_key_update = ListenKeyUpdate{
          .account = account_.name,
          .listen_key = listen_key.listen_key,
      };
      create_trace_and_dispatch(handler_, trace_info, listen_key_update);
    } else {
      if (!initial) [[unlikely]]
        log::info("Listen key has been refreshed!"sv);
    }
  }
  auto now = clock::get_system();
  listen_key_refresh_ = now + shared_.settings.rest.listen_key_refresh;
}

// balance

void OrderEntrySimple::get_balance() {
  profile_.balance([&]() {
    auto query = account_.create_query();
    auto headers = account_.create_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.get_balance,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_balance_ack(event);
    };
    (*connection_)("balance"sv, request, callback);
  });
}

void OrderEntrySimple::get_balance_ack(Trace<web::rest::Response> const &event) {
  profile_.balance_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::Balance balance{body, decode_buffer_};
      Trace event_2{event, balance};
      (*this)(event_2);
      request_.respond_balance = clock::get_system();  // completion
      download_balance_ = false;
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      download_balance_ = false;
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(Trace<json::Balance> const &event) {
  auto &[trace_info, balance] = event;
  log::info<2>("balance={}"sv, balance);
  for (auto &item : balance.data) {
    log::debug("item={}"sv, item);
    auto hold = item.balance - item.available_balance;
    auto funds_update = FundsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .currency = item.asset,
        .margin_mode = {},
        .balance = item.balance,
        .hold = hold,
        .external_account = {},
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = item.update_time,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    if (!std::isnan(item.cross_wallet_balance)) {
      auto funds_update = FundsUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .currency = item.asset,
          .margin_mode = MarginMode::CROSS,
          .balance = item.cross_wallet_balance,
          .hold = NaN,  // ???
          .external_account = {},
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = item.update_time,
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    }
  }
}

// account

void OrderEntrySimple::get_account() {
  profile_.account([&]() {
    auto query = account_.create_query();
    auto headers = account_.create_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.get_account,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_account_ack(event);
    };
    (*connection_)("account"sv, request, callback);
  });
}

void OrderEntrySimple::get_account_ack(Trace<web::rest::Response> const &event) {
  profile_.account_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::Account account{body, decode_buffer_};
      Trace event_2{event, account};
      (*this)(event_2);
      request_.respond_account = clock::get_system();  // completion
      download_account_ = false;
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      download_account_ = false;
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(Trace<json::Account> const &event) {
  auto &[trace_info, account] = event;
  log::info<2>("account={}"sv, account);
  for (auto &item : account.positions) {
    if (shared_.discard_symbol(item.symbol))
      continue;
    log::debug("item={}"sv, item);
    auto margin_mode = item.isolated ? MarginMode::ISOLATED : MarginMode::CROSS;
    auto long_quantity = std::max(0.0, item.notional);
    auto short_quantity = std::max(0.0, -item.notional);
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .margin_mode = margin_mode,
        .external_account{},
        .long_quantity = long_quantity,
        .short_quantity = short_quantity,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = account.update_time,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

// open-orders

void OrderEntrySimple::get_open_orders() {
  profile_.open_orders([&]() {
    auto query = account_.create_query();
    auto headers = account_.create_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.get_open_orders,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_open_orders_ack(event);
    };
    (*connection_)("open_orders"sv, request, callback);
  });
}

void OrderEntrySimple::get_open_orders_ack(Trace<web::rest::Response> const &event) {
  profile_.open_orders_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::OpenOrders open_orders{body, decode_buffer_};
      Trace event_2{event, open_orders};
      (*this)(event_2);
      request_.respond_orders = clock::get_system();  // completion
      download_orders_ = false;
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      download_orders_ = false;
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(Trace<json::OpenOrders> const &event) {
  auto &[trace_info, open_orders] = event;
  log::info<2>("open_orders={}"sv, open_orders);
  for (auto &order : open_orders.data) {
    log::debug("order={}"sv, order);
    log::info<2>("order={}"sv, order);
    if (std::empty(order.client_order_id))
      continue;
    open_orders_symbols_.emplace(order.symbol);
    auto side = json::map(order.side);
    auto order_type = json::map(order.type);
    auto time_in_force = json::map(order.time_in_force);
    auto external_order_id = fmt::format("{}"sv, order.order_id);  // alloc
    auto order_status = json::map(order.status);
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = order.symbol,
        .side = side,
        .position_effect = {},
        .margin_mode = {},
        .max_show_quantity = NaN,
        .order_type = order_type,
        .time_in_force = time_in_force,
        .execution_instructions = {},
        .create_time_utc = order.time,
        .update_time_utc = order.update_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = order.client_order_id,
        .order_status = order_status,
        .quantity = order.orig_qty,
        .price = order.price,
        .stop_price = order.stop_price,
        .remaining_quantity = NaN,
        .traded_quantity = order.executed_qty,
        .average_traded_price = order.avg_price,
        .last_traded_quantity = {},
        .last_traded_price = {},
        .last_liquidity = {},
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = {},
    };
    Trace event_2{trace_info, order_update};
    (*this)(event_2, order.client_order_id);
  }
}

// trades

// XXX FIXME download_trades_count
void OrderEntrySimple::get_trades() {
  profile_.trades([&]() {
    auto &symbols = shared_.settings.common.download_symbols;
    for (auto &symbol : symbols) {
      auto lookback = get_download_trades_lookback(shared_.settings, download_trades_is_first_);
      log::info<1>("Download trades: lookback={}"sv, lookback);
      auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
      auto end_time = clock::get_realtime<std::chrono::milliseconds>();
      auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - lookback);
      auto limit = shared_.settings.common.download_trades_limit;
      auto body = json::trades(encode_buffer_, symbol, start_time, end_time, limit, recv_window);
      auto query = account_.create_query_2(body);
      auto headers = account_.create_headers();
      log::debug(R"(path="{}")"sv, shared_.api.get_trades);
      log::debug(R"(body="{}")"sv, body);
      log::debug(R"(query="{}")"sv, query);
      log::debug(R"(headers="{}")"sv, headers);
      auto request = web::rest::Request{
          .method = web::http::Method::GET,
          .path = shared_.api.get_trades,
          .query = query,
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
          .headers = headers,
          .body = {},  // body,
          .quality_of_service = {},
      };
      auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
        TraceInfo trace_info;
        Trace event{trace_info, response};
        get_trades_ack(event);
      };
      (*connection_)("trades"sv, request, callback);
    }
  });
}

void OrderEntrySimple::get_trades_ack(Trace<web::rest::Response> const &event) {
  profile_.trades_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::Trades trades{body, decode_buffer_};
      Trace event_2{event, trades};
      (*this)(event_2);
      request_.respond_trades = clock::get_system();  // completion
      download_trades_ = false;
      download_trades_is_first_ = false;
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      download_trades_ = false;
    };
    process_response(event, handle_success, handle_error);
  });
}

// note! always external because we don't get ClOrdID
void OrderEntrySimple::operator()(Trace<json::Trades> const &event) {
  auto &[trace_info, trades] = event;
  log::info<2>("trades={}"sv, trades);
  for (auto &trade : trades.data) {
    log::info<2>("trade={}"sv, trade);
    auto liquidity = trade.maker ? Liquidity::MAKER : Liquidity::TAKER;
    auto fill = Fill{
        .exchange_time_utc = trade.time,
        .external_trade_id = {},
        .quantity = trade.qty,
        .price = trade.price,
        .liquidity = liquidity,
    };
    fmt::format_to(std::back_inserter(fill.external_trade_id), "{}"sv, trade.id);
    auto external_order_id = fmt::format("{}"sv, trade.order_id);
    auto side = json::map(trade.side);
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = {},
        .exchange = shared_.settings.exchange,
        .symbol = trade.symbol,
        .side = side,
        .position_effect = {},
        .margin_mode = {},
        .create_time_utc = trade.time,
        .update_time_utc = trade.time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .fills = {&fill, 1},
        .routing_id = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = {},
        .user = {},
        .strategy_id = {},
    };
    std::string_view client_order_id;  // note! unavailable
    create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, client_order_id);
  }
}

// ...

void OrderEntrySimple::refresh_listen_key() {
  if (!ready())
    return;
  auto now = clock::get_system();
  if (listen_key_refresh_ == listen_key_refresh_.zero() || now < listen_key_refresh_)
    return;
  log::info("Refreshing listen key..."sv);
  listen_key_refresh_ = now + shared_.settings.rest.listen_key_refresh;
  get_listen_key();
}

// new-order

void OrderEntrySimple::new_order(
    Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  profile_.new_order([&]() {
    if (!ready())
      throw server::oms::NotReady{"not ready"sv};
    auto &[message_info, create_order] = event;
    open_orders_symbols_.emplace(create_order.symbol);
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto body = json::new_order(encode_buffer_, create_order, order, request_id, recv_window);
    log::debug(R"(body="{}")"sv, body);
    auto query = account_.create_query(body);
    auto headers = account_.create_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = shared_.api.order,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
        .headers = headers,
        .body = body,
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this, user_id = message_info.source, order_id = create_order.order_id](
                        [[maybe_unused]] auto &request_id, auto &response) {
      auto version = uint32_t{1};
      TraceInfo trace_info;
      Trace event{trace_info, response};
      new_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntrySimple::new_order_ack(
    Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.new_order_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::NewOrder new_order{body};
      log::debug("new_order={}"sv, new_order);
      Trace event_2{event, new_order};
      (*this)(event_2, user_id, order_id, version);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto text) {
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(
    Trace<json::NewOrder> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, new_order] = event;
  log::info<2>("new_order={}, user_id={}, order_id={}, version={}"sv, new_order, user_id, order_id, version);
  auto side = json::map(new_order.side);
  auto order_type = json::map(new_order.type);
  auto time_in_force = json::map(new_order.time_in_force);
  auto order_status = json::map(new_order.status);
  auto external_order_id = fmt::format("{}"sv, new_order.order_id);  // alloc
  auto response = server::oms::Response{
      .request_type = RequestType::CREATE_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = {},
      .quantity = new_order.orig_qty,
      .price = new_order.price,
  };
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = new_order.symbol,
      .side = side,
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = time_in_force,
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = new_order.update_time,
      .external_account = {},
      .external_order_id = external_order_id,
      .client_order_id = {},
      .order_status = order_status,
      .quantity = new_order.orig_qty,
      .price = new_order.price,
      .stop_price = new_order.stop_price,
      .remaining_quantity = NaN,
      .traded_quantity = new_order.executed_qty,
      .average_traded_price = new_order.avg_price,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = UpdateType::INCREMENTAL,
      .sending_time_utc = {},
  };
  Trace event_2{trace_info, response};
  (*this)(event_2, user_id, order_id, order_update);
}

// modify-order

void OrderEntrySimple::modify_order(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw server::oms::NotReady{"not ready"sv};
    auto &[message_info, modify_order] = event;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto body = json::modify_order(
        encode_buffer_,
        modify_order,
        order,
        request_id,
        previous_request_id,
        recv_window,
        shared_.api.modify_order_full);
    log::debug(R"(body="{}")"sv, body);
    auto query = account_.create_query(body);
    auto headers = account_.create_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::PUT,
        .path = shared_.api.order,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
        .headers = headers,
        .body = body,
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback =
        [this, user_id = message_info.source, order_id = modify_order.order_id, version = modify_order.version](
            [[maybe_unused]] auto &request_id, auto &response) {
          TraceInfo trace_info;
          Trace event{trace_info, response};
          modify_order_ack(event, user_id, order_id, version);
        };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntrySimple::modify_order_ack(
    Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.modify_order_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::ModifyOrder modify_order{body};
      Trace event_2{event, modify_order};
      (*this)(event_2, user_id, order_id, version);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto text) {
      auto response = server::oms::Response{
          .request_type = RequestType::MODIFY_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(
    Trace<json::ModifyOrder> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, modify_order] = event;
  log::info<2>("modify_order={}, user_id={}, order_id={}, version={}"sv, modify_order, user_id, order_id, version);
  auto side = json::map(modify_order.side);
  auto order_type = json::map(modify_order.type);
  auto time_in_force = json::map(modify_order.time_in_force);
  auto external_order_id = fmt::format("{}"sv, modify_order.order_id);  // alloc
  auto order_status = json::map(modify_order.status);
  auto response = server::oms::Response{
      .request_type = RequestType::MODIFY_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = {},
      .quantity = modify_order.orig_qty,
      .price = modify_order.price,
  };
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = modify_order.symbol,
      .side = side,
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = time_in_force,
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = modify_order.update_time,
      .external_account = {},
      .external_order_id = external_order_id,
      .client_order_id = {},
      .order_status = order_status,
      .quantity = modify_order.orig_qty,
      .price = modify_order.price,
      .stop_price = modify_order.stop_price,
      .remaining_quantity = NaN,
      .traded_quantity = modify_order.executed_qty,
      .average_traded_price = modify_order.avg_price,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = UpdateType::INCREMENTAL,
      .sending_time_utc = {},
  };
  Trace event_2{trace_info, response};
  (*this)(event_2, user_id, order_id, order_update);
}

// cancel-order

void OrderEntrySimple::cancel_order(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw server::oms::NotReady{"not ready"sv};
    auto &[message_info, cancel_order] = event;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto body = json::cancel_order(encode_buffer_, cancel_order, order, request_id, previous_request_id, recv_window);
    log::debug(R"(body="{}")"sv, body);
    auto query = account_.create_query(body);
    auto headers = account_.create_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::DELETE,
        .path = shared_.api.order,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
        .headers = headers,
        .body = body,
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback =
        [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
            [[maybe_unused]] auto &request_id, auto &response) {
          TraceInfo trace_info;
          Trace event{trace_info, response};
          cancel_order_ack(event, user_id, order_id, version);
        };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntrySimple::cancel_order_ack(
    Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::CancelOrder cancel_order{body};
      Trace event_2{event, cancel_order};
      (*this)(event_2, user_id, order_id, version);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto text) {
      auto response = server::oms::Response{
          .request_type = RequestType::CANCEL_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(
    Trace<json::CancelOrder> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, cancel_order] = event;
  log::info<2>("cancel_order={}, user_id={}, order_id={}, version={}"sv, cancel_order, user_id, order_id, version);
  auto side = json::map(cancel_order.side);
  auto order_type = json::map(cancel_order.type);
  auto time_in_force = json::map(cancel_order.time_in_force);
  auto external_order_id = fmt::format("{}"sv, cancel_order.order_id);  // alloc
  auto order_status = json::map(cancel_order.status);
  auto response = server::oms::Response{
      .request_type = RequestType::CANCEL_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = {},
      .quantity = cancel_order.orig_qty,
      .price = cancel_order.price,
  };
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = cancel_order.symbol,
      .side = side,
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = time_in_force,
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = cancel_order.update_time,
      .external_account = {},
      .external_order_id = external_order_id,
      .client_order_id = {},
      .order_status = order_status,
      .quantity = cancel_order.orig_qty,
      .price = cancel_order.price,
      .stop_price = cancel_order.stop_price,
      .remaining_quantity = NaN,
      .traded_quantity = cancel_order.executed_qty,
      .average_traded_price = cancel_order.avg_price,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = UpdateType::INCREMENTAL,
      .sending_time_utc = {},
  };
  Trace event_2{trace_info, response};
  (*this)(event_2, user_id, order_id, order_update);
}

// cancel-all-orders

void OrderEntrySimple::cancel_all_open_orders(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.cancel_all_open_orders([&]() {
    auto &cancel_all_orders = event.value;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    for (auto &symbol : open_orders_symbols_) {
      if (!std::empty(cancel_all_orders.symbol) && symbol != cancel_all_orders.symbol)
        continue;
      auto body = json::cancel_all_open_orders(encode_buffer_, symbol, recv_window);
      log::debug(R"(body="{}")"sv, body);
      auto query = account_.create_query(body);
      auto headers = account_.create_headers();
      auto request = web::rest::Request{
          .method = web::http::Method::DELETE,
          .path = shared_.api.all_open_orders,
          .query = query,
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
          .headers = headers,
          .body = body,
          .quality_of_service = io::QualityOfService::IMMEDIATE,
      };
      auto callback = [this](auto &request_id, auto &response) {
        TraceInfo trace_info;
        Trace event{trace_info, response};
        cancel_all_open_orders_ack(event, request_id);
      };
      (*connection_)(request_id, request, callback);
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = cancel_all_orders.exchange,
          .symbol = cancel_all_orders.symbol,
          .side = cancel_all_orders.side,
          .origin = Origin::GATEWAY,
          .request_status = RequestStatus::FORWARDED,
          .error = {},
          .text = {},
          .request_id = request_id,
          .external_account = {},
          .number_of_affected_orders = {},
          .round_trip_latency = {},
          .user = {},
          .strategy_id = {},
      };
      TraceInfo trace_info;
      Trace event_2{trace_info, cancel_all_orders_ack};
      shared_(event_2);
    }
  });
}

void OrderEntrySimple::cancel_all_open_orders_ack(
    Trace<web::rest::Response> const &event, std::string_view const &request_id) {
  profile_.cancel_all_open_orders_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::CancelAllOpenOrders cancel_all_open_orders{body};
      log::debug("cancel_all_open_orders={}"sv, cancel_all_open_orders);
      Trace event_2{event, cancel_all_open_orders};
      (*this)(event_2, request_id);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(Failed to cancel all open orders: error={}, text="{}")"sv, error, text);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(Trace<json::CancelAllOpenOrders> const &event, std::string_view const &request_id) {
  auto &[trace_info, cancel_all_open_orders] = event;
  log::debug("cancel_all_open_orders={}"sv, cancel_all_open_orders);
  auto status = [&]() {
    if (cancel_all_open_orders.code == 200)
      return RequestStatus::ACCEPTED;
    return RequestStatus::REJECTED;
  }();
  auto error = json::guess_error(cancel_all_open_orders.code);
  auto cancel_all_orders_ack = CancelAllOrdersAck{
      .stream_id = stream_id_,
      .account = account_.name,
      .order_id = {},
      .exchange = {},
      .symbol = {},
      .side = {},
      .origin = Origin::EXCHANGE,
      .request_status = status,
      .error = error,
      .text = cancel_all_open_orders.msg,
      .request_id = request_id,
      .external_account = {},
      .number_of_affected_orders = {},
      .round_trip_latency = {},
      .user = {},
      .strategy_id = {},
  };
  Trace event_2{trace_info, cancel_all_orders_ack};
  shared_(event_2);
}

// auto-cancel-all-orders

void OrderEntrySimple::auto_cancel_all_open_orders() {
  profile_.auto_cancel_all_open_orders([&]() {
    for (auto &symbol : open_orders_symbols_) {
      auto countdown_time =
          std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_countdown);
      auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
      auto body = json::countdown_cancel_all_open_orders(encode_buffer_, symbol, countdown_time, recv_window);
      log::debug(R"(body="{}")"sv, body);
      auto query = account_.create_query(body);
      auto headers = account_.create_headers();
      auto request = web::rest::Request{
          .method = web::http::Method::POST,
          .path = shared_.api.countdown_cancel_all,
          .query = query,
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
          .headers = headers,
          .body = body,
          .quality_of_service = io::QualityOfService::IMMEDIATE,
      };
      auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
        TraceInfo trace_info;
        Trace event{trace_info, response};
        auto_cancel_all_open_orders_ack(event);
      };
      (*connection_)("auto_cancel"sv, request, callback);
    }
  });
}

void OrderEntrySimple::auto_cancel_all_open_orders_ack(Trace<web::rest::Response> const &event) {
  profile_.auto_cancel_all_open_orders_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::AutoCancelAllOpenOrders auto_cancel_all_open_orders{body};
      log::debug("auto_cancel_all_open_orders={}"sv, auto_cancel_all_open_orders);
      Trace event_2{event, auto_cancel_all_open_orders};
      (*this)(event_2);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(Failed to auto cancel all open orders: error={}, text="{}")"sv, error, text);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntrySimple::operator()(Trace<json::AutoCancelAllOpenOrders> const &event) {
  auto &[trace_info, auto_cancel_all_open_orders] = event;
  log::info<2>("auto_cancel_all_open_orders={}"sv, auto_cancel_all_open_orders);
}

namespace {
auto get_retry_after(auto &response) {
  std::chrono::nanoseconds result = {};
  response.dispatch(web::http::Header::RETRY_AFTER, [&](auto &value) {
    try {
      // XXX FIXME could also be a datetime (see https://datatracker.ietf.org/doc/html/rfc7231)
      auto seconds = utils::from_string_relaxed<int64_t>(value);
      result = std::chrono::seconds{seconds};
    } catch (RuntimeError &) {
      log::warn<5>(R"(Failed to parse text="{}")"sv, value);
    }
  });
  return result;
}
}  // namespace

template <typename SuccessHandler, typename ErrorHandler>
void OrderEntrySimple::process_response(
    web::rest::Response const &response, SuccessHandler success_handler, ErrorHandler error_handler) {
  try {
    auto [status, category, body] = response.result();
    log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
    switch (category) {
      using enum web::http::Category;
      case SUCCESS:  // 2xx
        success_handler(body);
        break;
      case CLIENT_ERROR:  // 4xx
        switch (status) {
          using enum web::http::Status;
          case FORBIDDEN:           // 403
            waf_limit_violation();  // note! this is *very* serious
            [[fallthrough]];
          case I_AM_A_TEAPOT:        // 418
          case TOO_MANY_REQUESTS: {  // 429
            auto retry_after = get_retry_after(response);
            if (retry_after.count())
              (*connection_).suspend(retry_after);
            auto text = fmt::format("{}"sv, status);
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::REQUEST_RATE_LIMIT_REACHED, text);
            break;
          }
          case CONFLICT:  // 409
            assert(false);
            [[fallthrough]];
          default: {
            json::Error error{body};
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(error.code), error.msg);
          }
        }
        break;
      case SERVER_ERROR: {  // 5xx
        auto text = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, text);
        break;
      }
      default:
        response.expect(web::http::Status::OK);  // throws
    }
  } catch (server::oms::Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(e.origin, e.status, e.error, e.what());
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

template <typename... Args>
void OrderEntrySimple::operator()(
    Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(
          user_id,
          order_id,
          stream_id_,
          trace_info,
          response,
          std::forward<Args>(args)...,
          []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

void OrderEntrySimple::operator()(
    Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(
          client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

void OrderEntrySimple::waf_limit_violation() {
  if (shared_.settings.rest.terminate_on_403) {
    log::fatal("WAF limit violation"sv);
  } else {
    log::warn("WAF limit violation"sv);
    (*connection_).suspend(shared_.settings.rest.back_off_delay);
  }
}

}  // namespace binance_futures
}  // namespace roq
