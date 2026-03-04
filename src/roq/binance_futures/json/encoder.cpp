/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/json/encoder.hpp"

#include "roq/logging.hpp"

#include "roq/decimal.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/binance_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// URL

// user-trades

std::string_view Encoder::user_trades_url(
    std::string &buffer,
    std::string_view const &symbol,
    std::chrono::milliseconds start_time,
    std::chrono::milliseconds end_time,
    uint32_t limit,
    std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(startTime={}&)"
      R"(endTime={}&)"
      R"(limit={}&)"
      R"(recvWindow={})"sv,
      symbol,
      start_time.count(),
      end_time.count(),
      limit,
      recv_window.count());
  return buffer;
}

// order-place

std::string_view Encoder::order_place_url(
    std::string &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window) {
  auto side = map(create_order.side).template get<Side>();
  auto type = map(create_order.order_type).template get<OrderType>();
  auto reduce_only = create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(side={}&)"
      R"(type={}&)"
      R"(quantity={}&)"
      R"(reduceOnly={}&)"sv,
      create_order.symbol,
      side.as_raw_text(),
      type.as_raw_text(),
      Decimal{create_order.quantity, ref_data.quantity.precision},
      reduce_only);
  switch (create_order.order_type) {
    using enum roq::OrderType;
    case UNDEFINED:
      assert(false);
      break;
    case MARKET:
      assert(std::isnan(create_order.price));
      break;
    case LIMIT: {
      assert(!std::isnan(create_order.price));
      auto time_in_force = map(create_order.time_in_force, create_order.execution_instructions).template get<TimeInForce>();
      fmt::format_to(
          std::back_inserter(buffer),
          R"(timeInForce={}&)"
          R"(price={}&)"sv,
          time_in_force.as_raw_text(),
          Decimal{create_order.price, ref_data.price.precision});
      break;
    }
  }
  if (!std::isnan(create_order.stop_price)) {
    fmt::format_to(std::back_inserter(buffer), R"(stopPrice={}&)"sv, Decimal{create_order.stop_price, ref_data.price.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(newClientOrderId={}&)"
      R"(recvWindow={})"sv,
      request_id,
      recv_window.count());
  return buffer;
}

// order-modify

std::string_view Encoder::order_modify_url(
    std::string &buffer,
    roq::ModifyOrder const &modify_order,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    bool order_modify_full) {
  buffer.clear();
  auto side = map(order.side).template get<Side>();
  if (order_modify_full) {  // fapi
    auto quantity = std::isnan(modify_order.quantity) ? order.quantity : modify_order.quantity;
    auto price = std::isnan(modify_order.price) ? order.price : modify_order.price;
    fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
    if (!std::empty(order.external_order_id)) {
      fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
    }
    fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
    fmt::format_to(
        std::back_inserter(buffer),
        R"(side={}&)"
        R"(quantity={}&)"
        R"(price={}&)"
        R"(recvWindow={})"sv,
        side.as_raw_text(),
        Decimal{quantity, ref_data.quantity.precision},
        Decimal{price, ref_data.price.precision},
        recv_window.count());
  } else {  // dapi
    auto helper = [](auto value, auto last_value) {
      if (!std::isnan(value) && !utils::is_equal(value, last_value)) {
        return value;
      }
      return NaN;
    };
    auto quantity = helper(modify_order.quantity, order.quantity);
    auto price = helper(modify_order.price, order.price);
    if (!std::isnan(quantity) && std::isnan(price)) {
      fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
      if (!std::empty(order.external_order_id)) {
        fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
      }
      fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
      fmt::format_to(
          std::back_inserter(buffer),
          R"(side={}&)"
          R"(quantity={}&)"
          R"(recvWindow={})"sv,
          side.as_raw_text(),
          Decimal{modify_order.quantity, ref_data.quantity.precision},
          recv_window.count());
    } else if (std::isnan(quantity) && !std::isnan(price)) {
      fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
      if (!std::empty(order.external_order_id)) {
        fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
      }
      fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
      fmt::format_to(
          std::back_inserter(buffer),
          R"(side={}&)"
          R"(price={}&)"
          R"(recvWindow={})"sv,
          side.as_raw_text(),
          Decimal{modify_order.price, ref_data.price.precision},
          recv_window.count());
    } else {
      throw server::oms::Rejected{Origin::GATEWAY, Error::INVALID_REQUEST_ARGS, "Missing quantity or price"sv};
    }
  }
  return buffer;
}

// order-cancel

std::string_view Encoder::order_cancel_url(
    std::string &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
  if (!std::empty(order.external_order_id)) {
    fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
  }
  fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
  fmt::format_to(std::back_inserter(buffer), R"(recvWindow={})"sv, recv_window.count());
  return buffer;
}

// all-open-orders

std::string_view Encoder::all_open_orders_url(std::string &buffer, std::string_view const &symbol, std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(recvWindow={})"sv,
      symbol,
      recv_window.count());
  return buffer;
}

// countdown

std::string_view Encoder::countdown_cancel_open_orders_url(
    std::string &buffer, std::string_view const &symbol, std::chrono::milliseconds countdown_time, std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(countdownTime={}&)"
      R"(recvWindow={})"sv,
      symbol,
      countdown_time.count(),
      recv_window.count());
  return buffer;
}

// JSON

// session-logon

std::string_view Encoder::session_logon_json(
    std::string &buffer, std::string_view const &api_key, std::chrono::milliseconds now_utc, std::string_view const &signature, std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"session.logon",)"
      R"("params":{{)"
      R"("apiKey":"{}",)"
      R"("timestamp":{},)"
      R"("signature":"{}")"
      R"(}})"
      R"(}})"sv,
      id,
      api_key,
      now_utc.count(),
      signature);
  return buffer;
}

// user-data-stream-start

std::string_view Encoder::user_data_stream_start_json(std::string &buffer, std::string_view const &api_key, std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"userDataStream.start",)"
      R"("params":{{)"
      R"("apiKey":"{}")"
      R"(}})"
      R"(}})"sv,
      id,
      api_key);
  return buffer;
}

// user-data-stream-ping

std::string_view Encoder::user_data_stream_ping_json(std::string &buffer, std::string_view const &api_key, std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"userDataStream.ping",)"
      R"("params":{{)"
      R"("apiKey":"{}")"
      R"(}})"
      R"(}})"sv,
      id,
      api_key);
  return buffer;
}

// account-balance

std::string_view Encoder::account_balance_json(std::string &buffer, std::chrono::milliseconds now_utc, std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"account.balance",)"
      R"("params":{{)"
      R"("timestamp":"{}")"
      R"(}})"
      R"(}})"sv,
      id,
      now_utc.count());
  return buffer;
}

// account-status

std::string_view Encoder::account_status_json(std::string &buffer, std::chrono::milliseconds now_utc, std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"account.status",)"
      R"("params":{{)"
      R"("timestamp":"{}")"
      R"(}})"
      R"(}})"sv,
      id,
      now_utc.count());
  return buffer;
}

// account-position

std::string_view Encoder::account_position_json(std::string &buffer, std::chrono::milliseconds now_utc, std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"account.position",)"
      R"("params":{{)"
      R"("timestamp":"{}")"
      R"(}})"
      R"(}})"sv,
      id,
      now_utc.count());
  return buffer;
}

// order-status

std::string_view Encoder::order_status_json(
    std::string &buffer, std::string_view const &symbol, std::chrono::milliseconds now_utc, std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"order.status",)"
      R"("params":{{)"
      R"("symbol":"{}",)"
      R"("timestamp":"{}")"
      R"(}})"
      R"(}})"sv,
      id,
      symbol,
      now_utc.count());
  return buffer;
}

// order-place

std::string_view Encoder::order_place_json(
    std::string &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window,
    std::chrono::milliseconds now_utc,
    std::string_view const &id) {
  auto side = map(create_order.side).template get<Side>();
  auto type = map(create_order.order_type).template get<OrderType>();
  auto reduce_only = create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
  auto time_in_force = map(create_order.time_in_force, create_order.execution_instructions).template get<TimeInForce>();
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"order.place",)"
      R"("params":{{)"
      R"("newClientOrderId":"{}")"
      R"(,"symbol":"{}")"
      R"(,"side":"{}")"
      R"(,"type":"{}")"
      R"(,"reduceOnly":"{}")"
      R"(,"quantity":"{}")"sv,
      id,
      request_id,
      create_order.symbol,
      side.as_raw_text(),
      type.as_raw_text(),
      reduce_only,
      Decimal{create_order.quantity, ref_data.quantity.precision});
  if (time_in_force != json::TimeInForce{}) {
    fmt::format_to(std::back_inserter(buffer), R"(,"timeInForce":"{}")"sv, time_in_force.as_raw_text());
  }
  if (!std::isnan(create_order.price)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"price":"{}")"sv, Decimal{create_order.price, ref_data.price.precision});
  }
  if (!std::isnan(create_order.stop_price)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"stopPrice":"{}")"sv, Decimal{create_order.stop_price, ref_data.price.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(,"recvWindow":{})"
      R"(,"timestamp":{})"
      R"(}})"
      R"(}})"sv,
      recv_window.count(),
      now_utc.count());
  return buffer;
}

// order-modify

// note! both quantity and price must be sent (different from dapi/rest)
std::string_view Encoder::order_modify_json(
    std::string &buffer,
    roq::ModifyOrder const &modify_order,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::chrono::milliseconds now_utc,
    std::string_view const &id) {
  auto side = map(order.side).template get<Side>();
  auto quantity = std::isnan(modify_order.quantity) ? order.quantity : modify_order.quantity;
  auto price = std::isnan(modify_order.price) ? order.price : modify_order.price;
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"order.modify",)"
      R"("params":{{)"
      R"("symbol":"{}")"
      R"(,"side":"{}")"sv,
      id,
      order.symbol,
      side.as_raw_text());
  if (std::empty(order.external_order_id)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"origClientOrderId":"{}")"sv, order.client_order_id);
  } else {
    fmt::format_to(std::back_inserter(buffer), R"(,"orderId":{})"sv, order.external_order_id);  // note! integer
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(,"quantity":"{}")"
      R"(,"price":"{}")"
      R"(,"recvWindow":{})"
      R"(,"timestamp":{})"
      R"(}})"
      R"(}})"sv,
      Decimal{quantity, ref_data.quantity.precision},
      Decimal{price, ref_data.price.precision},
      recv_window.count(),
      now_utc.count());
  return buffer;
}

// order-cancel

std::string_view Encoder::order_cancel_json(
    std::string &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::chrono::milliseconds now_utc,
    std::string_view const &id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("method":"order.cancel",)"
      R"("params":{{)"
      R"("symbol":"{}")"sv,
      id,
      order.symbol);
  if (std::empty(order.external_order_id)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"origClientOrderId":"{}")"sv, order.client_order_id);
  } else {
    fmt::format_to(std::back_inserter(buffer), R"(,"orderId":{})"sv, order.external_order_id);  // note! integer
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(,"recvWindow":{})"
      R"(,"timestamp":{})"
      R"(}})"
      R"(}})"sv,
      recv_window.count(),
      now_utc.count());
  return buffer;
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
