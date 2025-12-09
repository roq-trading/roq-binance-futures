/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/order_entry_portfolio.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/common.hpp"
#include "roq/utils/compare.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/binance_futures/json/encoder.hpp"
#include "roq/binance_futures/json/error.hpp"
#include "roq/binance_futures/json/map.hpp"
#include "roq/binance_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "omm"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
    SupportType::POSITION,
};

auto const X_MBX_USED_WEIGHT_1M = "x-mbx-used-weight-1m"sv;
auto const X_MBX_ORDER_COUNT_1M = "x-mbx-order-count-1m"sv;

size_t const MAX_DECODE_BUFFER_DEPTH = 1;

size_t const DOWNLOAD_TRADES_LIMIT = 1000;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.pm_uri;
  auto ping_path = fmt::format("/papi{}"sv, settings.rest.ping_path);
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = settings.rest.pm_host,
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = true,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = true,
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = ping_path,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
  create_metrics(auto &settings, auto &group, auto const &function, auto const &period) : utils::metrics::Factory{settings.app.name, group, function, period} {}
};

auto get_download_trades_lookback(auto &settings, auto download_trades_is_first) {
  if (download_trades_is_first) {
    if (settings.download.trades_lookback_on_restart.count()) {
      return settings.download.trades_lookback_on_restart;
    }
  }
  return settings.download.trades_lookback;
}

auto get_retry_after(auto &response) {
  std::chrono::nanoseconds result = {};
  response.dispatch(web::http::Header::RETRY_AFTER, [&](auto &value) {
    try {
      // XXX FIXME could also be a datetime (see https://datatracker.ietf.org/doc/html/rfc7231)
      auto seconds = utils::charconv::from_string_relaxed<int64_t>(value);
      result = std::chrono::seconds{seconds};
    } catch (RuntimeError &) {
      log::warn<5>(R"(Failed to parse text="{}")"sv, value);
    }
  });
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

OrderEntryPortfolio::OrderEntryPortfolio(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared, Request &request)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .listen_key = create_metrics(shared.settings, name_, "listen_key"sv),
          .listen_key_ack = create_metrics(shared.settings, name_, "listen_key_ack"sv),
          .account_balance = create_metrics(shared.settings, name_, "account_balance"sv),
          .account_balance_ack = create_metrics(shared.settings, name_, "account_balance_ack"sv),
          .account_status = create_metrics(shared.settings, name_, "account_status"sv),
          .account_status_ack = create_metrics(shared.settings, name_, "account_status_ack"sv),
          .position = create_metrics(shared.settings, name_, "position"sv),
          .position_ack = create_metrics(shared.settings, name_, "position_ack"sv),
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .open_orders_ack = create_metrics(shared.settings, name_, "open_orders_ack"sv),
          .trades = create_metrics(shared.settings, name_, "trades"sv),
          .trades_ack = create_metrics(shared.settings, name_, "trades_ack"sv),
          .order_place = create_metrics(shared.settings, name_, "order_place"sv),
          .order_place_ack = create_metrics(shared.settings, name_, "order_place_ack"sv),
          .order_modify = create_metrics(shared.settings, name_, "order_modify"sv),
          .order_modify_ack = create_metrics(shared.settings, name_, "order_modify_ack"sv),
          .order_cancel = create_metrics(shared.settings, name_, "order_cancel"sv),
          .order_cancel_ack = create_metrics(shared.settings, name_, "order_cancel_ack"sv),
          .open_orders_cancel_all = create_metrics(shared.settings, name_, "open_orders_cancel_all"sv),
          .open_orders_cancel_all_ack = create_metrics(shared.settings, name_, "open_orders_cancel_all_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      rate_limiter_{
          .request_weight_1m = create_metrics(shared.settings, name_, "request_weight"sv, "1m"sv),
          .create_order_1m = create_metrics(shared.settings, name_, "create_order"sv, "1m"sv),
      },
      account_{account}, shared_{shared}, request_{request}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void OrderEntryPortfolio::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntryPortfolio::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntryPortfolio::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  refresh_listen_key(now);
  refresh_balance(now);
  if (ready() && !downloading()) {
    if (!downloading() && request_.respond_balance < request_.request_balance) {
      log::info<1>("Download balance..."sv);
      get_account_balance(false);
      download_balance_ = true;
    }
    if (!downloading() && request_.respond_account < request_.request_account) {
      log::info<1>("Download account..."sv);
      get_account_status();
      download_account_ = true;
    }
    if (!downloading() && request_.respond_position < request_.request_position) {
      log::info<1>("Download position..."sv);
      get_position();
      download_position_ = true;
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

void OrderEntryPortfolio::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.listen_key, metrics::Type::PROFILE)
      .write(profile_.listen_key_ack, metrics::Type::PROFILE)
      .write(profile_.account_balance, metrics::Type::PROFILE)
      .write(profile_.account_balance_ack, metrics::Type::PROFILE)
      .write(profile_.account_status, metrics::Type::PROFILE)
      .write(profile_.account_status_ack, metrics::Type::PROFILE)
      .write(profile_.position, metrics::Type::PROFILE)
      .write(profile_.position_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.open_orders_ack, metrics::Type::PROFILE)
      .write(profile_.trades, metrics::Type::PROFILE)
      .write(profile_.trades_ack, metrics::Type::PROFILE)
      .write(profile_.order_place, metrics::Type::PROFILE)
      .write(profile_.order_place_ack, metrics::Type::PROFILE)
      .write(profile_.order_modify, metrics::Type::PROFILE)
      .write(profile_.order_modify_ack, metrics::Type::PROFILE)
      .write(profile_.order_cancel, metrics::Type::PROFILE)
      .write(profile_.order_cancel_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders_cancel_all, metrics::Type::PROFILE)
      .write(profile_.open_orders_cancel_all_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      // rate limiter
      .write(rate_limiter_.request_weight_1m, metrics::Type::RATE_LIMITER)
      .write(rate_limiter_.create_order_1m, metrics::Type::RATE_LIMITER);
}

uint16_t OrderEntryPortfolio::operator()(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  order_place(event, order, request_id);
  return stream_id_;
}

uint16_t OrderEntryPortfolio::operator()(
    Event<ModifyOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  order_modify(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntryPortfolio::operator()(
    Event<CancelOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  order_cancel(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntryPortfolio::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  open_orders_cancel_all(event, request_id);
  return stream_id_;
}

void OrderEntryPortfolio::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntryPortfolio::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
  download_balance_ = false;
  download_account_ = false;
  download_position_ = false;
  download_orders_ = false;
  download_trades_ = false;
}

void OrderEntryPortfolio::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntryPortfolio::operator()(Trace<web::rest::Client::MessageBegin> const &) {
  shared_.rate_limits.clear();
}

void OrderEntryPortfolio::operator()(Trace<web::rest::Client::Header> const &event) {
  auto &header = event.value;
  if (utils::case_insensitive_compare(header.name, X_MBX_USED_WEIGHT_1M) == 0) {
    try {
      auto value = utils::charconv::from_string_relaxed<uint32_t>(header.value);
      auto rate_limit = RateLimit{
          .type = RateLimitType::REQUEST_WEIGHT,
          .period = 1min,
          .end_time_utc = {},
          .limit = shared_.limits.request_weight_1m,
          .value = value,
      };
      shared_.rate_limits.emplace_back(rate_limit);
      rate_limiter_.request_weight_1m.set(value);
    } catch (RuntimeError &) {
      log::warn<5>(R"(Failed to parse text="{}")"sv, header.value);
    }
  }
  if (utils::case_insensitive_compare(header.name, X_MBX_ORDER_COUNT_1M) == 0) {
    try {
      auto value = utils::charconv::from_string_relaxed<uint32_t>(header.value);
      auto rate_limit = RateLimit{
          .type = RateLimitType::CREATE_ORDER,
          .period = 1min,
          .end_time_utc = {},
          .limit = shared_.limits.create_order_1m,
          .value = value,
      };
      shared_.rate_limits.emplace_back(rate_limit);
      rate_limiter_.create_order_1m.set(value);
    } catch (RuntimeError &) {
      log::warn<5>(R"(Failed to parse text="{}")"sv, header.value);
    }
  }
}

void OrderEntryPortfolio::operator()(Trace<web::rest::Client::MessageEnd> const &event) {
  auto &trace_info = event.trace_info;
  if (std::empty(shared_.rate_limits)) {
    return;
  }
  auto rate_limits_update = RateLimitsUpdate{
      .stream_id = stream_id_,
      .account = account_.name,
      .origin = Origin::EXCHANGE,
      .rate_limits = shared_.rate_limits,
  };
  create_trace_and_dispatch(handler_, trace_info, rate_limits_update);
  shared_.rate_limits.clear();
}

void OrderEntryPortfolio::operator()(ConnectionStatus status) {
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

uint32_t OrderEntryPortfolio::download(OrderEntryState state) {
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
      return 0;
  }
  assert(false);
  return 0;
}

// listen-key

void OrderEntryPortfolio::get_listen_key() {
  profile_.listen_key([&]() {
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = shared_.api.papi.listen_key,
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
    log::info<1>("Download listen-key..."sv);
    (*connection_)("listen-key"sv, request, callback);
  });
}

void OrderEntryPortfolio::get_listen_key_ack(Trace<web::rest::Response> const &event, [[maybe_unused]] uint32_t sequence) {
  auto const STATE = OrderEntryState::LISTEN_KEY;
  profile_.listen_key_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(Download listen-key has FAILED: account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &body) {
      json::ListenKeyAck listen_key_ack{body};
      Trace event_2{event, listen_key_ack};
      (*this)(event_2);
      log::info<1>("Download listen-key has COMPLETED!"sv);
      download_.check_relaxed(STATE);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::ListenKeyAck> const &event) {
  auto &[trace_info, listen_key_ack] = event;
  log::info<2>("listen_key_ack={}"sv, listen_key_ack);
  bool initial = std::empty(listen_key_);
  if (utils::update(listen_key_, listen_key_ack.listen_key)) {
    if (initial) {
      log::info(R"(Listen key has been acquired (value="{}"))"sv, listen_key_);
      auto listen_key_update = ListenKeyUpdate{
          .account = account_.name,
          .listen_key = listen_key_ack.listen_key,
      };
      create_trace_and_dispatch(handler_, trace_info, listen_key_update);
    } else {
      if (!initial) [[unlikely]] {
        log::info("Listen key has been refreshed!"sv);
      }
    }
  }
  auto now = clock::get_system();
  listen_key_refresh_ = now + shared_.settings.rest.listen_key_refresh;
}

// account-balance

void OrderEntryPortfolio::get_account_balance(bool polling) {
  profile_.account_balance([&]() {
    auto query = account_.create_rest_signature();
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.papi.account_balance,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, polling = polling]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_account_balance_ack(event, polling);
    };
    log::info<1>("Download balance... (polling={})"sv, polling);
    (*connection_)("account-balance"sv, request, callback);
  });
}

void OrderEntryPortfolio::get_account_balance_ack(Trace<web::rest::Response> const &event, bool polling) {
  profile_.account_balance_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(Download balance has FAILED: account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      download_balance_ = false;
    };
    auto handle_success = [&](auto &body) {
      json::AccountBalanceAck account_balance_ack{body, decode_buffer_};
      Trace event_2{event, account_balance_ack};
      (*this)(event_2);
      if (!polling) {
        // completion
        request_.respond_balance = clock::get_system();
        download_balance_ = false;
      }
      log::info<1>("Download balance has COMPLETED!"sv);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::AccountBalanceAck> const &event) {
  auto &[trace_info, account_balance_ack] = event;
  log::info<2>("account_balance_ack={}"sv, account_balance_ack);
  for (auto &item : account_balance_ack.data) {
    log::info<2>("item={}"sv, item);
    auto hold = item.balance - item.available_balance;
    auto funds_update = FundsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .currency = item.asset,
        .margin_mode = MarginMode::PORTFOLIO,
        .balance = item.total_wallet_balance,
        .hold = hold,
        .borrowed = NaN,
        .external_account = {},
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = item.update_time,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    /*
    if (!std::isnan(item.cross_wallet_balance)) {
      auto funds_update = FundsUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .currency = item.asset,
          .margin_mode = MarginMode::PORTFOLIO,
          .balance = item.cross_wallet_balance,
          .hold = NaN,  // ???
          .borrowed = NaN,
          .external_account = {},
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = item.update_time,
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    }
    */
  }
}

// account-status

void OrderEntryPortfolio::get_account_status() {
  profile_.account_status([&]() {
    auto query = account_.create_rest_signature();
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.papi.account_status,
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
      get_account_status_ack(event);
    };
    log::info<1>("Download account..."sv);
    (*connection_)("account"sv, request, callback);
  });
}

void OrderEntryPortfolio::get_account_status_ack(Trace<web::rest::Response> const &event) {
  profile_.account_status_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(Download account has FAILED: account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      download_account_ = false;
    };
    auto handle_success = [&](auto &body) {
      json::AccountStatusAck account_status_ack{body, decode_buffer_};
      Trace event_2{event, account_status_ack};
      (*this)(event_2);
      // completion
      request_.respond_account = clock::get_system();
      download_account_ = false;
      log::info<1>("Download account has COMPLETED!"sv);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::AccountStatusAck> const &event) {
  auto &[trace_info, account_status_ack] = event;
  log::info<2>("account_status_ack={}"sv, account_status_ack);
  for (auto &item : account_status_ack.positions) {
    if (shared_.discard_symbol(item.symbol)) {
      continue;
    }
    log::info<2>("item={}"sv, item);
    auto margin_mode = item.isolated ? MarginMode::ISOLATED : MarginMode::PORTFOLIO;
    auto long_quantity = std::max(0.0, item.notional);
    auto short_quantity = std::max(0.0, -item.notional);
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .margin_mode = margin_mode,
        .external_account = {},
        .long_quantity = long_quantity,
        .short_quantity = short_quantity,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = account_status_ack.update_time,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

// position

void OrderEntryPortfolio::get_position() {
  profile_.position([&]() {
    auto query = account_.create_rest_signature();
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.papi.position_risk,
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
      get_position_ack(event);
    };
    log::info<1>("Download position..."sv);
    (*connection_)("position"sv, request, callback);
  });
}

void OrderEntryPortfolio::get_position_ack(Trace<web::rest::Response> const &event) {
  profile_.position_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(Download position has FAILED: account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      download_position_ = false;
    };
    auto handle_success = [&](auto &body) {
      json::PositionList position{body, decode_buffer_};
      Trace event_2{event, position};
      (*this)(event_2);
      // completion
      request_.respond_position = clock::get_system();
      download_position_ = false;
      log::info<1>("Download position has COMPLETED!"sv);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::PositionList> const &event) {
  auto &[trace_info, position] = event;
  log::info<2>("position={}"sv, position);
  for (auto &item : position.data) {
    if (shared_.discard_symbol(item.symbol)) {
      continue;
    }
    log::info<2>("item={}"sv, item);
    auto long_quantity = std::max(0.0, item.position_amt);
    auto short_quantity = std::max(0.0, -item.position_amt);
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .margin_mode = MarginMode::PORTFOLIO,
        .external_account = {},
        .long_quantity = long_quantity,
        .short_quantity = short_quantity,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = item.update_time,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

// open-orders

void OrderEntryPortfolio::get_open_orders() {
  profile_.open_orders([&]() {
    auto query = account_.create_rest_signature();
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.papi.open_orders,
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
    log::info<1>("Download open-orders..."sv);
    (*connection_)("open-orders"sv, request, callback);
  });
}

void OrderEntryPortfolio::get_open_orders_ack(Trace<web::rest::Response> const &event) {
  profile_.open_orders_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(Download open-orders has FAILED: account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      download_orders_ = false;
    };
    auto handle_success = [&](auto &body) {
      json::OpenOrdersAck open_orders_ack{body, decode_buffer_};
      Trace event_2{event, open_orders_ack};
      (*this)(event_2);
      // completion
      request_.respond_orders = clock::get_system();
      download_orders_ = false;
      log::info<1>("Download open-orders has COMPLETED!"sv);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::OpenOrdersAck> const &event) {
  auto &[trace_info, open_orders_ack] = event;
  log::info<2>("open_orders_ack={}"sv, open_orders_ack);
  for (auto &item : open_orders_ack.data) {
    log::info<2>("item={}"sv, item);
    if (std::empty(item.client_order_id)) {
      continue;
    }
    open_orders_symbols_.emplace(item.symbol);
    auto external_order_id = fmt::format("{}"sv, item.order_id);  // alloc
    auto remaining_quantity = item.orig_qty - item.executed_qty;
    auto average_traded_price = utils::compare(item.executed_qty, 0.0) == 0 ? NaN : item.avg_price;
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .side = map(item.side),
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
        .max_show_quantity = NaN,
        .order_type = map(item.type),
        .time_in_force = map(item.time_in_force),
        .execution_instructions = {},
        .create_time_utc = item.time,
        .update_time_utc = item.update_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = item.client_order_id,
        .order_status = map(item.status),
        .quantity = item.orig_qty,
        .price = item.price,
        .stop_price = item.stop_price,
        .leverage = NaN,
        .remaining_quantity = remaining_quantity,
        .traded_quantity = item.executed_qty,
        .average_traded_price = average_traded_price,
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
    (*this)(event_2, item.client_order_id);
  }
}

// trades

// XXX FIXME download_trades_count
void OrderEntryPortfolio::get_trades() {
  profile_.trades([&]() {
    auto &symbols = shared_.settings.download.symbols;
    for (auto &symbol : symbols) {
      auto lookback = get_download_trades_lookback(shared_.settings, download_trades_is_first_);
      log::info<1>("Download trades: lookback={}"sv, lookback);
      auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
      auto end_time = clock::get_realtime<std::chrono::milliseconds>();
      auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - lookback);
      auto limit = shared_.settings.download.trades_limit ? shared_.settings.download.trades_limit : DOWNLOAD_TRADES_LIMIT;
      auto body = json::Encoder::user_trades_url(encode_buffer_, symbol, start_time, end_time, limit, recv_window);
      auto query = account_.create_rest_signature_query(body);  // XXX
      auto headers = account_.get_rest_headers();
      auto request = web::rest::Request{
          .method = web::http::Method::GET,
          .path = shared_.api.papi.user_trades,
          .query = query,
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
          .headers = headers,
          .body = {},  // note! can't use body with GET
          .quality_of_service = {},
      };
      auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
        TraceInfo trace_info;
        Trace event{trace_info, response};
        get_trades_ack(event);
      };
      log::info<1>("Download trades..."sv);
      (*connection_)("trades"sv, request, callback);
    }
  });
}

void OrderEntryPortfolio::get_trades_ack(Trace<web::rest::Response> const &event) {
  profile_.trades_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(Download trades has FAILED: account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      download_trades_ = false;
    };
    auto handle_success = [&](auto &body) {
      json::TradesAck trades_ack{body, decode_buffer_};
      Trace event_2{event, trades_ack};
      (*this)(event_2);
      // completion
      request_.respond_trades = clock::get_system();
      download_trades_ = false;
      download_trades_is_first_ = false;
      log::info<1>("Download trades has COMPLETED!"sv);
    };
    process_response(event, handle_error, handle_success);
  });
}

// note! always external because we don't get ClOrdID
void OrderEntryPortfolio::operator()(Trace<json::TradesAck> const &event) {
  auto &[trace_info, trades_ack] = event;
  for (auto &trade : trades_ack.data) {
    log::info<2>("trade={}"sv, trade);
    auto liquidity = trade.maker ? Liquidity::MAKER : Liquidity::TAKER;
    auto side = map(trade.side).template get<Side>();
    auto ref_data = shared_.get_ref_data(shared_.settings.exchange, trade.symbol);
    auto profit_loss_amount = utils::compute_profit_loss_amount(side, trade.qty, trade.price, ref_data.multiplier);
    auto fill = Fill{
        .exchange_time_utc = trade.time,
        .external_trade_id = {},
        .quantity = trade.qty,
        .price = trade.price,
        .liquidity = liquidity,
        .commission_amount = trade.commission,
        .commission_currency = trade.commission_asset,
        .base_amount = NaN,
        .quote_amount = NaN,
        .profit_loss_amount = profit_loss_amount,
    };
    fmt::format_to(std::back_inserter(fill.external_trade_id), "{}"sv, trade.id);
    auto external_order_id = fmt::format("{}"sv, trade.order_id);
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = {},
        .exchange = shared_.settings.exchange,
        .symbol = trade.symbol,
        .side = side,
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
        .quantity_type = {},
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

void OrderEntryPortfolio::refresh_listen_key(std::chrono::nanoseconds now) {
  if (!ready()) {
    return;
  }
  if (listen_key_refresh_.count() == 0 || now < listen_key_refresh_) {
    return;
  }
  log::info("Refreshing listen key..."sv);
  listen_key_refresh_ = now + shared_.settings.rest.listen_key_refresh;
  get_listen_key();
}

void OrderEntryPortfolio::refresh_balance(std::chrono::nanoseconds now) {
  if (!ready()) {
    return;
  }
  if (shared_.settings.misc.test_pm_balance_freq.count() == 0) {
    return;
  }
  if (now < balance_refresh_) {
    return;
  }
  log::info("Refreshing balance..."sv);
  balance_refresh_ = now + shared_.settings.misc.test_pm_balance_freq;
  get_account_balance(true);
}

// order-place

void OrderEntryPortfolio::order_place(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  profile_.order_place([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, create_order] = event;
    open_orders_symbols_.emplace(create_order.symbol);
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto body = json::Encoder::order_place_url(encode_buffer_, create_order, order, request_id, recv_window);
    auto query = account_.create_rest_signature_body(body);
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = shared_.api.papi.order,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
        .headers = headers,
        .body = body,
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this, user_id = message_info.source, order_id = create_order.order_id]([[maybe_unused]] auto &request_id, auto &response) {
      uint32_t version = 1;
      TraceInfo trace_info;
      Trace event{trace_info, response};
      order_place_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntryPortfolio::order_place_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.order_place_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
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
    auto handle_success = [&](auto &body) {
      json::OrderPlaceAck order_place_ack{body};
      Trace event_2{event, order_place_ack};
      (*this)(event_2, user_id, order_id, version);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::OrderPlaceAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, order_place_ack] = event;
  log::info<2>("order_place_ack={}, user_id={}, order_id={}, version={}"sv, order_place_ack, user_id, order_id, version);
  auto external_order_id = fmt::format("{}"sv, order_place_ack.order_id);  // alloc
  auto response = server::oms::Response{
      .request_type = RequestType::CREATE_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = {},
      .quantity = order_place_ack.orig_qty,
      .price = order_place_ack.price,
  };
  if (shared_.settings.rest.drop_order_update) {
    Trace event_2{trace_info, response};
    (*this)(event_2, user_id, order_id);
  } else {
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = order_place_ack.symbol,
        .side = map(order_place_ack.side),
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
        .max_show_quantity = NaN,
        .order_type = map(order_place_ack.type),
        .time_in_force = map(order_place_ack.time_in_force),
        .execution_instructions = {},
        .create_time_utc = {},
        .update_time_utc = order_place_ack.update_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .order_status = map(order_place_ack.status),
        .quantity = order_place_ack.orig_qty,
        .price = order_place_ack.price,
        .stop_price = order_place_ack.stop_price,
        .leverage = NaN,
        .remaining_quantity = NaN,
        .traded_quantity = order_place_ack.executed_qty,
        .average_traded_price = order_place_ack.avg_price,
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
}

// order-modify

void OrderEntryPortfolio::order_modify(
    Event<ModifyOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  profile_.order_modify([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, modify_order] = event;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto body = json::Encoder::order_modify_url(encode_buffer_, modify_order, order, request_id, previous_request_id, recv_window, true);
    auto query = account_.create_rest_signature_body(body);
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::PUT,
        .path = shared_.api.papi.order,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
        .headers = headers,
        .body = body,
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this, user_id = message_info.source, order_id = modify_order.order_id, version = modify_order.version](
                        [[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      order_modify_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

// testing document
// autogen error mapping from code
// optional override map in .toml
void OrderEntryPortfolio::order_modify_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.order_modify_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
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
    auto handle_success = [&](auto &body) {
      json::OrderModifyAck order_modify_ack{body};
      Trace event_2{event, order_modify_ack};
      (*this)(event_2, user_id, order_id, version);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::OrderModifyAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, order_modify_ack] = event;
  log::info<2>("order_modify_ack={}, user_id={}, order_id={}, version={}"sv, order_modify_ack, user_id, order_id, version);
  auto external_order_id = fmt::format("{}"sv, order_modify_ack.order_id);  // alloc
  auto response = server::oms::Response{
      .request_type = RequestType::MODIFY_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = {},
      .quantity = order_modify_ack.orig_qty,
      .price = order_modify_ack.price,
  };
  if (shared_.settings.rest.drop_order_update) {
    Trace event_2{trace_info, response};
    (*this)(event_2, user_id, order_id);
  } else {
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = order_modify_ack.symbol,
        .side = map(order_modify_ack.side),
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
        .max_show_quantity = NaN,
        .order_type = map(order_modify_ack.type),
        .time_in_force = map(order_modify_ack.time_in_force),
        .execution_instructions = {},
        .create_time_utc = {},
        .update_time_utc = order_modify_ack.update_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .order_status = map(order_modify_ack.status),
        .quantity = order_modify_ack.orig_qty,
        .price = order_modify_ack.price,
        .stop_price = order_modify_ack.stop_price,
        .leverage = NaN,
        .remaining_quantity = NaN,
        .traded_quantity = order_modify_ack.executed_qty,
        .average_traded_price = order_modify_ack.avg_price,
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
}

// order-cancel

void OrderEntryPortfolio::order_cancel(
    Event<CancelOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  profile_.order_cancel([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_order] = event;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto body = json::Encoder::order_cancel_url(encode_buffer_, cancel_order, order, request_id, previous_request_id, recv_window);
    auto query = account_.create_rest_signature_body(body);
    auto headers = account_.get_rest_headers();
    auto request = web::rest::Request{
        .method = web::http::Method::DELETE,
        .path = shared_.api.papi.order,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_X_WWW_FORM_URLENCODED,
        .headers = headers,
        .body = body,
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
                        [[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      order_cancel_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntryPortfolio::order_cancel_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.order_cancel_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
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
    auto handle_success = [&](auto &body) {
      json::OrderCancelAck order_cancel_ack{body};
      Trace event_2{event, order_cancel_ack};
      (*this)(event_2, user_id, order_id, version);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::OrderCancelAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, order_cancel_ack] = event;
  log::info<2>("order_cancel_ack={}, user_id={}, order_id={}, version={}"sv, order_cancel_ack, user_id, order_id, version);
  auto external_order_id = fmt::format("{}"sv, order_cancel_ack.order_id);  // alloc
  auto response = server::oms::Response{
      .request_type = RequestType::CANCEL_ORDER,
      .origin = Origin::EXCHANGE,
      .request_status = RequestStatus::ACCEPTED,
      .error = {},
      .text = {},
      .version = version,
      .request_id = {},
      .quantity = order_cancel_ack.orig_qty,
      .price = order_cancel_ack.price,
  };
  if (shared_.settings.rest.drop_order_update) {
    Trace event_2{trace_info, response};
    (*this)(event_2, user_id, order_id);
  } else {
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = order_cancel_ack.symbol,
        .side = map(order_cancel_ack.side),
        .position_effect = {},
        .margin_mode = MarginMode::PORTFOLIO,
        .max_show_quantity = NaN,
        .order_type = map(order_cancel_ack.type),
        .time_in_force = map(order_cancel_ack.time_in_force),
        .execution_instructions = {},
        .create_time_utc = {},
        .update_time_utc = order_cancel_ack.update_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .order_status = map(order_cancel_ack.status),
        .quantity = order_cancel_ack.orig_qty,
        .price = order_cancel_ack.price,
        .stop_price = order_cancel_ack.stop_price,
        .leverage = NaN,
        .remaining_quantity = NaN,
        .traded_quantity = order_cancel_ack.executed_qty,
        .average_traded_price = order_cancel_ack.avg_price,
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
}

// open-orders-cancel-all

void OrderEntryPortfolio::open_orders_cancel_all(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.open_orders_cancel_all([&]() {
    auto &[trace_info, cancel_all_orders] = event;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    for (auto &symbol : open_orders_symbols_) {
      if (!std::empty(cancel_all_orders.symbol) && symbol != cancel_all_orders.symbol) {
        continue;
      }
      auto body = json::Encoder::all_open_orders_url(encode_buffer_, symbol, recv_window);
      auto query = account_.create_rest_signature_body(body);
      auto headers = account_.get_rest_headers();
      auto request = web::rest::Request{
          .method = web::http::Method::DELETE,
          .path = shared_.api.papi.all_open_orders,
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
        open_orders_cancel_all_ack(event, request_id);
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

void OrderEntryPortfolio::open_orders_cancel_all_ack(Trace<web::rest::Response> const &event, std::string_view const &request_id) {
  profile_.open_orders_cancel_all_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
    };
    auto handle_success = [&](auto &body) {
      json::OpenOrdersCancelAllAck open_orders_cancel_all_ack{body};
      Trace event_2{event, open_orders_cancel_all_ack};
      (*this)(event_2, request_id);
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryPortfolio::operator()(Trace<json::OpenOrdersCancelAllAck> const &event, std::string_view const &request_id) {
  auto &[trace_info, open_orders_cancel_all_ack] = event;
  auto status = [&]() {
    if (open_orders_cancel_all_ack.code == 200) {
      return RequestStatus::ACCEPTED;
    }
    return RequestStatus::REJECTED;
  }();
  auto error = json::guess_error(open_orders_cancel_all_ack.code);
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
      .text = open_orders_cancel_all_ack.msg,
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

// helpers

void OrderEntryPortfolio::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case UNKNOWN:
      case INFORMATIONAL_RESPONSE:
        response.expect(web::http::Status::OK);  // throws
        break;
      case SUCCESS:
        success_handler(body);
        break;
      case REDIRECTION:
        log::fatal("Unexpected: URL is being redirected"sv);
      case CLIENT_ERROR:
        switch (status) {
          using enum web::http::Status;
          case FORBIDDEN:           // 403
            waf_limit_violation();  // note! this is *very* serious
            [[fallthrough]];
          case I_AM_A_TEAPOT:        // 418
          case TOO_MANY_REQUESTS: {  // 429
            auto retry_after = get_retry_after(response);
            if (retry_after.count()) {
              (*connection_).suspend(retry_after);
            }
            auto message = fmt::format("{}"sv, status);
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::REQUEST_RATE_LIMIT_REACHED, message);
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
      case SERVER_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
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
void OrderEntryPortfolio::operator()(Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(user_id, order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

void OrderEntryPortfolio::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

void OrderEntryPortfolio::waf_limit_violation() {
  if (shared_.settings.rest.terminate_on_403) {
    log::fatal("WAF limit violation"sv);
  } else {
    log::warn("WAF limit violation"sv);
    (*connection_).suspend(shared_.settings.rest.back_off_delay);
  }
}

}  // namespace binance_futures
}  // namespace roq
