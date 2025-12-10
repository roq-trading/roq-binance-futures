/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/web_socket.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/binance_futures/json/encoder.hpp"
#include "roq/binance_futures/json/map.hpp"
#include "roq/binance_futures/json/utils.hpp"
#include "roq/binance_futures/json/wsapi_type.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
};

uint32_t const REQUEST_ID = 1'000'000;

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &settings, auto &context, auto &interface) {
  auto uri = settings.ws_api_2.uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = interface,
      .uris = {&uri, 1},
      .host = settings.ws_api_2.host,
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
      .ping_frequency = settings.ws_api_2.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() -> std::string { return {}; });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
  create_metrics(auto &settings, auto &group, auto const &function, auto const &params) : utils::metrics::Factory{settings.app.name, group, function, params} {}
};

auto get_download_trades_lookback(auto const &settings, auto download_trades_is_first) {
  if (download_trades_is_first) {
    if (settings.download.trades_lookback_on_restart.count()) {
      return settings.download.trades_lookback_on_restart;
    }
  }
  return settings.download.trades_lookback;
}
}  // namespace

// === IMPLEMENTATION ===

WebSocket::WebSocket(
    Handler &handler,
    io::Context &context,
    uint16_t stream_id,
    Account &account,
    Shared &shared,
    Request &request,
    bool master,
    std::string_view const &interface)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)}, master_{master},
      connection_{create_connection(*this, shared.settings, context, interface)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .session_logon = create_metrics(shared.settings, name_, "session_logon"sv),
          .session_logon_ack = create_metrics(shared.settings, name_, "session_logon_ack"sv),
          .user_data_stream_start = create_metrics(shared.settings, name_, "user_data_stream_start"sv),
          .user_data_stream_start_ack = create_metrics(shared.settings, name_, "user_data_stream_start_ack"sv),
          .user_data_stream_ping = create_metrics(shared.settings, name_, "user_data_stream_ping"sv),
          .user_data_stream_ping_ack = create_metrics(shared.settings, name_, "user_data_stream_ping_ack"sv),
          .account_balance = create_metrics(shared.settings, name_, "account_balance"sv),
          .account_balance_ack = create_metrics(shared.settings, name_, "account_balance_ack"sv),
          .account_status = create_metrics(shared.settings, name_, "account_status"sv),
          .account_status_ack = create_metrics(shared.settings, name_, "account_status_ack"sv),
          .account_position = create_metrics(shared.settings, name_, "account_position"sv),
          .account_position_ack = create_metrics(shared.settings, name_, "account_position_ack"sv),
          .order_status = create_metrics(shared.settings, name_, "order_status"sv),
          .order_status_ack = create_metrics(shared.settings, name_, "order_status_ack"sv),
          .open_orders_cancel_all = create_metrics(shared.settings, name_, "open_orders_cancel_all"sv),
          .open_orders_cancel_all_ack = create_metrics(shared.settings, name_, "open_orders_cancel_all_ack"sv),
          .order_place = create_metrics(shared.settings, name_, "order_place"sv),
          .order_place_ack = create_metrics(shared.settings, name_, "order_place_ack"sv),
          .order_modify = create_metrics(shared.settings, name_, "order_modify"sv),
          .order_modify_ack = create_metrics(shared.settings, name_, "order_modify_ack"sv),
          .order_cancel = create_metrics(shared.settings, name_, "order_cancel"sv),
          .order_cancel_ack = create_metrics(shared.settings, name_, "order_cancel_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      rate_limiter_{
          .request_weight_1m = create_metrics(shared.settings, name_, "request_weight"sv, "1m"sv),
          .create_order_10s = create_metrics(shared.settings, name_, "create_order"sv, "10s"sv),
          .create_order_1d = create_metrics(shared.settings, name_, "create_order"sv, "1d"sv),
      },
      account_{account}, shared_{shared}, request_{request}, request_id_{REQUEST_ID * stream_id},
      download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
  log::info<5>(R"(stream_id={}, account="{}", master={})"sv, stream_id_, account_.name, master_);
}

void WebSocket::operator()(Event<Start> const &) {
  (*connection_).start();
}

void WebSocket::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void WebSocket::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  user_data_stream_ping(now);
  if (master_ && ready() && !downloading()) {
    if (!downloading() && request_.respond_balance < request_.request_balance) {
      log::info("Download balance..."sv);
      account_balance();
      download_balance_ = true;
    }
    if (!downloading() && request_.respond_account < request_.request_account) {
      log::info("Download account..."sv);
      account_status();
      download_account_ = true;
    }
    /* XXX FIXME this is wrong -- we can only download a single order
    if (!downloading() && request_.respond_orders < request_.request_orders) {
      log::info("Download orders..."sv);
      if (order_status()) {
        download_orders_ = true;
      } else {
      }
    }
    */
  }
}

void WebSocket::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.session_logon, metrics::Type::PROFILE)
      .write(profile_.session_logon_ack, metrics::Type::PROFILE)
      .write(profile_.user_data_stream_start, metrics::Type::PROFILE)
      .write(profile_.user_data_stream_start_ack, metrics::Type::PROFILE)
      .write(profile_.user_data_stream_ping, metrics::Type::PROFILE)
      .write(profile_.user_data_stream_ping_ack, metrics::Type::PROFILE)
      .write(profile_.account_balance, metrics::Type::PROFILE)
      .write(profile_.account_balance_ack, metrics::Type::PROFILE)
      .write(profile_.account_status, metrics::Type::PROFILE)
      .write(profile_.account_status_ack, metrics::Type::PROFILE)
      .write(profile_.account_position, metrics::Type::PROFILE)
      .write(profile_.account_position_ack, metrics::Type::PROFILE)
      .write(profile_.order_status, metrics::Type::PROFILE)
      .write(profile_.order_status_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders_cancel_all, metrics::Type::PROFILE)
      .write(profile_.open_orders_cancel_all_ack, metrics::Type::PROFILE)
      .write(profile_.order_place, metrics::Type::PROFILE)
      .write(profile_.order_place_ack, metrics::Type::PROFILE)
      .write(profile_.order_modify, metrics::Type::PROFILE)
      .write(profile_.order_modify_ack, metrics::Type::PROFILE)
      .write(profile_.order_cancel, metrics::Type::PROFILE)
      .write(profile_.order_cancel_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY)
      // rate limiter
      .write(rate_limiter_.request_weight_1m, metrics::Type::RATE_LIMITER)
      .write(rate_limiter_.create_order_10s, metrics::Type::RATE_LIMITER)
      .write(rate_limiter_.create_order_1d, metrics::Type::RATE_LIMITER);
}

uint16_t WebSocket::operator()(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  order_place(event, order, request_id);
  return stream_id_;
}

uint16_t WebSocket::operator()(
    Event<ModifyOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  order_modify(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t WebSocket::operator()(
    Event<CancelOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  order_cancel(event, order, request_id, previous_request_id);
  return stream_id_;
}

// XXX FIXME not supported
uint16_t WebSocket::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  log::fatal("Unexpected"sv);
  // open_orders_cancel_all(event, request_id);
  // return stream_id_;
}

// session-logon

void WebSocket::session_logon() {
  profile_.session_logon([&]() {
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::SESSION_LOGON,
        .user_id = {},
        .order_id = {},
        .version = {},
        .order_id_2 = {},
    };
    auto request_id = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto signature = account_.create_session_logon_signature(now_utc);
    auto message = json::Encoder::session_logon_json(encode_buffer_, account_.get_key(), now_utc, signature, request_id);
    (*connection_).send_text(message);
    (*this)(ConnectionStatus::LOGIN_SENT);
  });
}

// listen-key

void WebSocket::user_data_stream_start() {
  profile_.user_data_stream_start([&]() {
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::USER_DATA_STREAM_START,
        .user_id = {},
        .order_id = {},
        .version = {},
        .order_id_2 = {},
    };
    auto request_id = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::user_data_stream_start_json(encode_buffer_, account_.get_key(), request_id);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

void WebSocket::user_data_stream_ping(std::chrono::nanoseconds now) {
  profile_.user_data_stream_ping([&]() {
    if (!ready()) {
      return;
    }
    if (std::empty(listen_key_)) {
      return;
    }
    if (listen_key_refresh_.count() == 0 || now < listen_key_refresh_) {
      return;
    }
    log::info<1>("Refreshing listen key..."sv);
    listen_key_refresh_ = now + shared_.settings.rest.listen_key_refresh;
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::USER_DATA_STREAM_PING,
        .user_id = {},
        .order_id = {},
        .version = {},
        .order_id_2 = {},
    };
    auto request_id = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::user_data_stream_ping_json(encode_buffer_, account_.get_key(), request_id);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

// account-balance

void WebSocket::account_balance() {
  profile_.account_balance([&]() {
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::ACCOUNT_BALANCE,
        .user_id = {},
        .order_id = {},
        .version = {},
        .order_id_2 = {},
    };
    auto request_id = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::account_balance_json(encode_buffer_, now_utc, request_id);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

// account-status

void WebSocket::account_status() {
  profile_.account_status([&]() {
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::ACCOUNT_STATUS,
        .user_id = {},
        .order_id = {},
        .version = {},
        .order_id_2 = {},
    };
    auto request_id = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::account_status_json(encode_buffer_, now_utc, request_id);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

// account-position

void WebSocket::account_position() {
  profile_.account_position([&]() {
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::ACCOUNT_POSITION,
        .user_id = {},
        .order_id = {},
        .version = {},
        .order_id_2 = {},
    };
    auto request_id = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::account_position_json(encode_buffer_, now_utc, request_id);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

// order-status
// XXX FIXME following is wrong -- we can only request a single order

bool WebSocket::order_status() {
  auto &symbols = shared_.settings.download.symbols;
  for (auto &item : symbols) {
    order_status(item);
  }
  return !std::empty(symbols);
}

void WebSocket::order_status(std::string_view const &symbol) {
  profile_.order_status([&]() {
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::ORDERS_STATUS,
        .user_id = {},
        .order_id = {},
        .version = {},
        .order_id_2 = {},
    };
    auto request_id = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::order_status_json(encode_buffer_, symbol, now_utc, request_id);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

// XXX my-trades does not exist => rest

// open-orders-cancel-all
// XXX FIXME not supported

void WebSocket::open_orders_cancel_all(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.open_orders_cancel_all([&]() {
    if (!ready()) [[unlikely]] {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &message_info = event.message_info;
    auto &cancel_all_orders = event.value;
    auto send_ack = [&](auto &symbol) {
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = cancel_all_orders.order_id,
          .exchange = cancel_all_orders.exchange,
          .symbol = symbol,
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
          .strategy_id = cancel_all_orders.strategy_id,
      };
      TraceInfo trace_info{event};
      Trace event_2{trace_info, cancel_all_orders_ack};
      shared_(event_2);
    };
    for (auto &symbol : open_orders_symbols_) {
      if (!std::empty(cancel_all_orders.symbol) && symbol != cancel_all_orders.symbol) {
        continue;
      }
      auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
      auto request = json::WSAPIRequest{
          .sequence = ++request_id_,
          .type = json::WSAPIType::OPEN_ORDERS_CANCEL_ALL,
          .user_id = message_info.source,
          .order_id = {},
          .version = {},
          .order_id_2 = {},
      };
      auto request_id_2 = json::WSAPIRequest::encode(request_encode_buffer_, request);  // XXX FIXME here we lose request_id
      auto message = fmt::format(
          R"({{)"
          R"("id":"{}",)"
          R"("method":"openOrders.cancelAll",)"
          R"("params":{{)"
          R"("symbol":"{}",)"
          R"("timestamp":"{}")"
          R"(}})"
          R"(}})"sv,
          request_id_2,
          symbol,
          now_utc.count());
      (*connection_).send_text(message);
      send_ack(symbol);
    }
  });
}

// order-place

void WebSocket::order_place(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  profile_.order_place([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, create_order] = event;
    open_orders_symbols_.emplace(create_order.symbol);
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::ORDER_PLACE,
        .user_id = message_info.source,
        .order_id = create_order.order_id,
        .version = 1,
        .order_id_2 = {},
    };
    auto request_id_2 = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::order_place_json(encode_buffer_, create_order, order, request_id, recv_window, now_utc, request_id_2);
    log::info<5>(R"(message="{}")"sv, message);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

// order-modify

void WebSocket::order_modify(
    Event<ModifyOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  profile_.order_modify([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, modify_order] = event;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::ORDER_MODIFY,
        .user_id = message_info.source,
        .order_id = modify_order.order_id,
        .version = modify_order.version,
        .order_id_2 = {},
    };
    auto request_id_2 = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::order_modify_json(encode_buffer_, modify_order, order, request_id, previous_request_id, recv_window, now_utc, request_id_2);
    log::info<5>(R"(message="{}")"sv, message);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

// order-cancel

void WebSocket::order_cancel(
    Event<CancelOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  profile_.order_cancel([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_order] = event;
    auto recv_window = std::chrono::duration_cast<std::chrono::milliseconds>(shared_.settings.rest.order_recv_window);
    auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
    auto request = json::WSAPIRequest{
        .sequence = ++request_id_,
        .type = json::WSAPIType::ORDER_CANCEL,
        .user_id = message_info.source,
        .order_id = cancel_order.order_id,
        .version = cancel_order.version,
        .order_id_2 = {},
    };
    auto request_id_2 = json::WSAPIRequest::encode(request_encode_buffer_, request);
    auto message = json::Encoder::order_cancel_json(encode_buffer_, cancel_order, order, request_id, previous_request_id, recv_window, now_utc, request_id_2);
    log::info<5>(R"(message="{}")"sv, message);
    log::warn(R"(DEBUG {})"sv, message);
    (*connection_).send_text(message);
  });
}

void WebSocket::operator()(web::socket::Client::Connected const &) {
}

void WebSocket::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  // XXX FIXME also reset the download_* latches?
}

void WebSocket::operator()(web::socket::Client::Ready const &) {
  download_.begin();
  (*this)(ConnectionStatus::DOWNLOADING);
}

void WebSocket::operator()(web::socket::Client::Close const &) {
}

void WebSocket::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void WebSocket::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void WebSocket::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(ConnectionStatus status) {
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

uint32_t WebSocket::download(WebSocketState state) {
  switch (state) {
    using enum WebSocketState;
    case UNDEFINED:
      assert(false);
      break;
    case SESSION_LOGON:
      session_logon();
      return 1;
    case USER_DATA_STREAM_START:
      user_data_stream_start();
      return 1;
    case ACCOUNT_POSITION:
      account_position();  // just testing -- not used
      return 0;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

void WebSocket::parse(std::string_view const &message) {
  profile_.parse([&]() {
    log::debug("{}"sv, message);
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::WSAPIParser::dispatch(*this, message, decode_buffer_, trace_info, shared_.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

// json::WSAPIParser::Handler

void WebSocket::operator()(Trace<json::WSAPIError> const &event) {
  auto &[trace_info, error] = event;
  log::fatal("error={}"sv, error);
}

void WebSocket::operator()(Trace<json::WSAPISessionLogon> const &event) {
  auto const STATE = WebSocketState::SESSION_LOGON;
  profile_.session_logon([&]() {
    auto &[trace_info, session_logon] = event;
    log::info<2>("session_logon={}"sv, session_logon);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&]([[maybe_unused]] auto &result) { download_.check_relaxed(STATE); };
    if (session_logon.status == 200) {
      handle_success(session_logon.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(session_logon.error.code), session_logon.error.msg);
    }
    update_rate_limits(event);
  });
}

void WebSocket::operator()(Trace<json::WSAPIListenKey> const &event) {
  auto const STATE = WebSocketState::USER_DATA_STREAM_START;
  profile_.user_data_stream_start_ack([&]() {
    auto &[trace_info, listen_key] = event;
    log::info<2>("listen_key={}"sv, listen_key);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &result) {
      listen_key_ = result.listen_key;
      log::info<1>(R"(Listen key has been acquired (value="{}"))"sv, listen_key_);
      auto listen_key_update = ListenKeyUpdate{
          .account = account_.name,
          .listen_key = listen_key_,
      };
      create_trace_and_dispatch(handler_, trace_info, listen_key_update);
      download_.check_relaxed(STATE);
      auto now = clock::get_system();
      listen_key_refresh_ = now + shared_.settings.rest.listen_key_refresh;
    };
    if (listen_key.status == 200) {
      handle_success(listen_key.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(listen_key.error.code), listen_key.error.msg);
    }
    update_rate_limits(event);
  });
}

void WebSocket::operator()(Trace<json::WSAPIAccountBalance> const &event) {
  profile_.account_balance_ack([&]() {
    auto &[trace_info, account_balance] = event;
    log::info<2>("account_balance={}"sv, account_balance);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
    };
    auto handle_success = [&](auto &result) {
      for (auto &item : result) {
        auto funds_update = FundsUpdate{
            .stream_id = stream_id_,
            .account = account_.name,
            .currency = item.asset,
            .margin_mode = {},
            .balance = item.available_balance,
            .hold = NaN,
            .borrowed = NaN,
            .external_account = {},
            .update_type = UpdateType::SNAPSHOT,
            .exchange_time_utc = item.update_time,
            .sending_time_utc = item.update_time,
        };
        create_trace_and_dispatch(handler_, trace_info, funds_update, true);
      }
    };
    if (account_balance.status == 200) {
      handle_success(account_balance.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(account_balance.error.code), account_balance.error.msg);
    }
    update_rate_limits(event);
    log::info<1>("Balance download has completed!"sv);
    request_.respond_balance = clock::get_system();
    download_balance_ = false;
  });
}

void WebSocket::operator()(Trace<json::WSAPIAccountStatus> const &event) {
  profile_.account_status_ack([&]() {
    auto &[trace_info, account_status] = event;
    log::info<2>("account_status={}"sv, account_status);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
    };
    auto handle_success = [&](auto &result) {
      for (auto &item : result.positions) {
        if (shared_.discard_symbol(item.symbol)) {
          continue;
        }
        log::info<2>("item={}"sv, item);
        auto margin_mode = item.isolated ? MarginMode::ISOLATED : MarginMode::CROSS;
        auto long_quantity = std::max(0.0, item.position_amt);
        auto short_quantity = std::max(0.0, -item.position_amt);
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
            .exchange_time_utc = result.update_time,
            .sending_time_utc = {},
        };
        create_trace_and_dispatch(handler_, trace_info, position_update, true);
      }
    };
    if (account_status.status == 200) {
      handle_success(account_status.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(account_status.error.code), account_status.error.msg);
    }
    update_rate_limits(event);
    // completion
    log::info<1>("Account download has completed!"sv);
    request_.respond_account = clock::get_system();
    download_account_ = false;
  });
}

void WebSocket::operator()(Trace<json::WSAPIAccountPosition> const &event) {
  profile_.account_position_ack([&]() {
    auto &[trace_info, account_position] = event;
    log::info<2>("account_position={}"sv, account_position);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
    };
    auto handle_success = [&]([[maybe_unused]] auto &result) {
      // XXX FIXME TODO
    };
    if (account_position.status == 200) {
      handle_success(account_position.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(account_position.error.code), account_position.error.msg);
    }
    update_rate_limits(event);
    // completion
    // request_.respond_position = clock::get_system();
    // download_position_ = false;
  });
}

void WebSocket::operator()(Trace<json::WSAPIOpenOrders> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<json::WSAPITrades> const &) {
  log::fatal("Unexpected"sv);
}

void WebSocket::operator()(Trace<json::WSAPIOpenOrdersCancelAll> const &event, json::WSAPIRequest const &request) {
  profile_.open_orders_cancel_all_ack([&]() {
    auto &[trace_info, open_orders_cancel_all] = event;
    log::info<2>("open_orders_cancel_all={}, request={}"sv, open_orders_cancel_all, request);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      // XXX FIXME TODO ack
    };
    auto handle_success = [&](auto &result) {
      for (auto &item : result) {
        log::info<2>("item={}"sv, item);
        if (std::empty(item.client_order_id)) {
          continue;
        }
        auto external_order_id = fmt::format("{}"sv, item.order_id);  // alloc
        auto order_update = server::oms::OrderUpdate{
            .account = account_.name,
            .exchange = shared_.settings.exchange,
            .symbol = item.symbol,
            .side = map(item.side),
            .position_effect = {},
            .margin_mode = {},
            .max_show_quantity = NaN,
            .order_type = map(item.type),
            .time_in_force = map(item.time_in_force),
            .execution_instructions = {},
            .create_time_utc = item.time,
            .update_time_utc = item.update_time,
            .external_account = {},
            .external_order_id = external_order_id,
            .client_order_id = {},
            .order_status = map(item.status),
            .quantity = item.orig_qty,
            .price = item.price,
            .stop_price = item.stop_price,
            .leverage = NaN,
            .remaining_quantity = NaN,
            .traded_quantity = item.executed_qty,
            .average_traded_price = {},
            .last_traded_quantity = {},
            .last_traded_price = {},
            .last_liquidity = {},
            .routing_id = {},
            .max_request_version = {},
            .max_response_version = {},
            .max_accepted_version = {},
            .update_type = UpdateType::INCREMENTAL,
            .sending_time_utc = {},
        };
        shared_.update_order(item.client_order_id, stream_id_, trace_info, order_update, []([[maybe_unused]] auto &order) {});
      }
    };
    if (open_orders_cancel_all.status == 200) {
      handle_success(open_orders_cancel_all.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(open_orders_cancel_all.error.code), open_orders_cancel_all.error.msg);
    }
    update_rate_limits(event);
  });
}

void WebSocket::operator()(Trace<json::WSAPIOrderPlace> const &event, json::WSAPIRequest const &request) {
  profile_.order_place_ack([&]() {
    auto &[trace_info, order_place] = event;
    log::info<2>("order_place={}, request={}"sv, order_place, request);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::REJECTED,
          .error = error,
          .text = text,
          .version = request.version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, request.user_id, request.order_id);
    };
    auto handle_success = [&]([[maybe_unused]] auto &result) {
      auto external_order_id = fmt::format("{}"sv, result.order_id);  // alloc
      auto order_status = map(result.status).template get<OrderStatus>();
      // LIMIT_MAKER orders do not return any order state + we only end up here if we receive HTTP status OK
      if (order_status == OrderStatus{}) {
        order_status = OrderStatus::WORKING;
      }
      auto remaining_quantity = result.orig_qty - result.executed_qty;
      /*
      auto average_traded_price =
          utils::is_zero(result.executed_qty) ? NaN : (result.cummulative_quote_qty / result.executed_qty);
      */
      auto average_traded_price = NaN;
      auto last_traded_quantity = 0.0;  // note! could also use result.executed_qty
      auto tmp = 0.0;
      /*
      for (auto &item : result.fills) {
        last_traded_quantity += item.qty;
        tmp += item.price * item.qty;
      }
      */
      auto last_traded_price = NaN;  // note! could also use average_traded_price
      if (utils::is_greater(last_traded_quantity, 0.0)) {
        last_traded_price = tmp / last_traded_quantity;
      }
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::ACCEPTED,
          .error = {},
          .text = {},
          .version = request.version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      auto order_update = server::oms::OrderUpdate{
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = result.symbol,
          .side = map(result.side),
          .position_effect = {},
          .margin_mode = {},
          .max_show_quantity = NaN,
          .order_type = map(result.type),
          .time_in_force = map(result.time_in_force),
          .execution_instructions = {},
          .create_time_utc = {},
          .update_time_utc = {},  // result.transact_time,
          .external_account = {},
          .external_order_id = external_order_id,
          .client_order_id = {},
          .order_status = order_status,
          .quantity = result.orig_qty,
          .price = result.price,
          .stop_price = NaN,
          .leverage = NaN,
          .remaining_quantity = remaining_quantity,
          .traded_quantity = result.executed_qty,
          .average_traded_price = average_traded_price,
          .last_traded_quantity = last_traded_quantity,
          .last_traded_price = last_traded_price,
          .last_liquidity = {},
          .routing_id = {},
          .max_request_version = {},
          .max_response_version = {},
          .max_accepted_version = {},
          .update_type = UpdateType::INCREMENTAL,
          .sending_time_utc = {},
      };
      Trace event_2{trace_info, response};
      (*this)(event_2, request.user_id, request.order_id, order_update);
    };
    if (order_place.status == 200) {
      handle_success(order_place.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(order_place.error.code), order_place.error.msg);
    }
    update_rate_limits(event);
  });
}

void WebSocket::operator()(Trace<json::WSAPIOrderModify> const &event, json::WSAPIRequest const &request) {
  profile_.order_modify_ack([&]() {
    auto &[trace_info, order_modify] = event;
    log::info<2>("order_modify={}, request={}"sv, order_modify, request);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::MODIFY_ORDER,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::REJECTED,
          .error = error,
          .text = text,
          .version = request.version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, request.user_id, request.order_id);
    };
    auto handle_success = [&](auto &result) {
      auto external_order_id = fmt::format("{}"sv, result.order_id);  // alloc
      auto response = server::oms::Response{
          .request_type = RequestType::MODIFY_ORDER,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::ACCEPTED,
          .error = {},
          .text = {},
          .version = request.version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      auto order_update = server::oms::OrderUpdate{
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = result.symbol,
          .side = map(result.side),
          .position_effect = {},
          .margin_mode = {},
          .max_show_quantity = NaN,
          .order_type = map(result.type),
          .time_in_force = map(result.time_in_force),
          .execution_instructions = {},
          .create_time_utc = {},
          .update_time_utc = {},  // result.transact_time,
          .external_account = {},
          .external_order_id = external_order_id,
          .client_order_id = {},
          .order_status = map(result.status),
          .quantity = result.orig_qty,
          .price = result.price,
          .stop_price = NaN,
          .leverage = NaN,
          .remaining_quantity = NaN,
          .traded_quantity = result.executed_qty,
          .average_traded_price = NaN,
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
      (*this)(event_2, request.user_id, request.order_id, order_update);
    };
    if (order_modify.status == 200) {
      handle_success(order_modify.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(order_modify.error.code), order_modify.error.msg);
    }
    update_rate_limits(event);
  });
}

void WebSocket::operator()(Trace<json::WSAPIOrderCancel> const &event, json::WSAPIRequest const &request) {
  profile_.order_cancel_ack([&]() {
    auto &[trace_info, order_cancel] = event;
    log::info<2>("order_cancel={}, request={}"sv, order_cancel, request);
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(account="{}", origin={}, error={}, status={}, text="{}")"sv, account_.name, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CANCEL_ORDER,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::REJECTED,
          .error = error,
          .text = text,
          .version = request.version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, request.user_id, request.order_id);
    };
    auto handle_success = [&](auto &result) {
      auto external_order_id = fmt::format("{}"sv, result.order_id);  // alloc
      auto response = server::oms::Response{
          .request_type = RequestType::CANCEL_ORDER,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::ACCEPTED,
          .error = {},
          .text = {},
          .version = request.version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      auto order_update = server::oms::OrderUpdate{
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = result.symbol,
          .side = map(result.side),
          .position_effect = {},
          .margin_mode = {},
          .max_show_quantity = NaN,
          .order_type = map(result.type),
          .time_in_force = map(result.time_in_force),
          .execution_instructions = {},
          .create_time_utc = {},
          .update_time_utc = {},  // result.transact_time,
          .external_account = {},
          .external_order_id = external_order_id,
          .client_order_id = {},
          .order_status = map(result.status),
          .quantity = result.orig_qty,
          .price = result.price,
          .stop_price = NaN,
          .leverage = NaN,
          .remaining_quantity = NaN,
          .traded_quantity = result.executed_qty,
          .average_traded_price = NaN,
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
      (*this)(event_2, request.user_id, request.order_id, order_update);
    };
    if (order_cancel.status == 200) {
      handle_success(order_cancel.result);
    } else {
      handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(order_cancel.error.code), order_cancel.error.msg);
    }
    update_rate_limits(event);
  });
}

// helpers

void WebSocket::update_rate_limits(auto &event) {
  auto &[trace_info, message] = event;
  shared_.rate_limits.clear();
  for (auto &item : message.rate_limits) {
    auto type = [&]() -> RateLimitType {
      switch (item.rate_limit_type) {
        using enum json::RateLimitType::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          break;
        case ORDERS:
          return RateLimitType::CREATE_ORDER;
        case REQUEST_WEIGHT:
          return RateLimitType::REQUEST_WEIGHT;
        case RAW_REQUESTS:
          return RateLimitType::REQUEST;
      }
      return {};
    }();
    if (type == RateLimitType{}) {
      continue;
    }
    auto period = [&]() -> std::chrono::seconds {
      switch (item.interval) {
        using enum json::Interval::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          break;
        case SECOND:
          return item.interval_num * 1s;
        case MINUTE:
          return item.interval_num * 1min;
        case DAY:
          return item.interval_num * 24h;
      };
      return {};
    }();
    switch (type) {
      using enum RateLimitType;
      case UNDEFINED:
        break;
      case ORDER_ACTION:
        break;
      case CREATE_ORDER:
        if (period == 10s) {
          rate_limiter_.create_order_10s.set(item.count);
        } else if (period == 24h) {
          rate_limiter_.create_order_1d.set(item.count);
        }
        break;
      case REQUEST:
        break;
      case REQUEST_WEIGHT:
        if (period == 1min) {
          rate_limiter_.request_weight_1m.set(item.count);
        }
        break;
    }
    auto rate_limit = RateLimit{
        .type = type,
        .period = period,
        .end_time_utc = {},
        .limit = item.limit,
        .value = item.count,
    };
    shared_.rate_limits.emplace_back(rate_limit);
  }
  if (!std::empty(shared_.rate_limits)) {
    auto rate_limits_update = RateLimitsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .origin = Origin::EXCHANGE,
        .rate_limits = shared_.rate_limits,
    };
    Trace event_2{trace_info, rate_limits_update};
    handler_(event_2);
  }
  shared_.rate_limits.clear();
}

template <typename... Args>
void WebSocket::operator()(Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(user_id, order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

void WebSocket::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

}  // namespace binance_futures
}  // namespace roq
