/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/encoder.hpp"

#include "roq/logging.hpp"

#include "roq/decimal.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/utils/text/writer.hpp"

#include "roq/binance_futures/json/map.hpp"

#define WS_USE_FMT

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// URL

// user-trades

std::string_view Encoder::user_trades_url(
    std::vector<char> &buffer,
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// order-place

std::string_view Encoder::order_place_url(
    std::vector<char> &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window) {
  auto side = map(create_order.side).template get<Side>();
  auto type = map(create_order.order_type).template get<OrderType>();
  auto reduce_only = false;
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
      Decimal{create_order.quantity, order.quantity_precision.precision},
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
      auto time_in_force = map(create_order.time_in_force).template get<TimeInForce>();
      fmt::format_to(
          std::back_inserter(buffer),
          R"(timeInForce={}&)"
          R"(price={}&)"sv,
          time_in_force.as_raw_text(),
          Decimal{create_order.price, order.price_precision.precision});
      break;
    }
  }
  if (!std::isnan(create_order.stop_price)) {
    fmt::format_to(std::back_inserter(buffer), R"(stopPrice={}&)"sv, Decimal{create_order.stop_price, order.price_precision.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(newClientOrderId={}&)"
      R"(recvWindow={})"sv,
      request_id,
      recv_window.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// order-modify

std::string_view Encoder::order_modify_url(
    std::vector<char> &buffer,
    roq::ModifyOrder const &modify_order,
    server::oms::Order const &order,
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
        Decimal{quantity, order.quantity_precision.precision},
        Decimal{price, order.price_precision.precision},
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
          Decimal{modify_order.quantity, order.quantity_precision.precision},
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
          Decimal{modify_order.price, order.price_precision.precision},
          recv_window.count());
    } else {
      throw server::oms::Rejected{Origin::GATEWAY, Error::INVALID_REQUEST_ARGS, "Missing quantity or price"sv};
    }
  }
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// order-cancel

std::string_view Encoder::order_cancel_url(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// all-open-orders

std::string_view Encoder::all_open_orders_url(std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(recvWindow={})"sv,
      symbol,
      recv_window.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// countdown

std::string_view Encoder::countdown_cancel_open_orders_url(
    std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds countdown_time, std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(countdownTime={}&)"
      R"(recvWindow={})"sv,
      symbol,
      countdown_time.count(),
      recv_window.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// JSON

// session-logon

std::string_view Encoder::session_logon_json(
    std::vector<char> &buffer,
    std::string_view const &api_key,
    std::chrono::milliseconds now_utc,
    std::string_view const &signature,
    std::string_view const &id) {
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// user-data-stream-start

std::string_view Encoder::user_data_stream_start_json(std::vector<char> &buffer, std::string_view const &api_key, std::string_view const &id) {
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// user-data-stream-ping

std::string_view Encoder::user_data_stream_ping_json(std::vector<char> &buffer, std::string_view const &api_key, std::string_view const &id) {
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// account-balance

std::string_view Encoder::account_balance_json(std::vector<char> &buffer, std::chrono::milliseconds now_utc, std::string_view const &id) {
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// account-status

std::string_view Encoder::account_status_json(std::vector<char> &buffer, std::chrono::milliseconds now_utc, std::string_view const &id) {
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// account-position

std::string_view Encoder::account_position_json(std::vector<char> &buffer, std::chrono::milliseconds now_utc, std::string_view const &id) {
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// order-status

std::string_view Encoder::order_status_json(
    std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds now_utc, std::string_view const &id) {
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

// order-place

std::string_view Encoder::order_place_json(
    std::vector<char> &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window,
    std::chrono::milliseconds now_utc,
    std::string_view const &id) {
  auto side = map(create_order.side).template get<Side>();
  auto type = map(create_order.order_type).template get<OrderType>();
  auto time_in_force = map(create_order.time_in_force).template get<TimeInForce>();
#if defined(WS_USE_FMT)
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
      R"(,"quantity":"{}")"sv,
      id,
      request_id,
      create_order.symbol,
      side.as_raw_text(),
      type.as_raw_text(),
      Decimal{create_order.quantity, order.quantity_precision.precision});
  if (time_in_force != json::TimeInForce{}) {
    fmt::format_to(std::back_inserter(buffer), R"(,"timeInForce":"{}")"sv, time_in_force.as_raw_text());
  }
  if (!std::isnan(create_order.price)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"price":"{}")"sv, Decimal{create_order.price, order.price_precision.precision});
  }
  if (!std::isnan(create_order.stop_price)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"stopPrice":"{}")"sv, Decimal{create_order.stop_price, order.price_precision.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(,"recvWindow":{})"
      R"(,"timestamp":{})"
      R"(}})"
      R"(}})"sv,
      recv_window.count(),
      now_utc.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
#else
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("{"sv);
  writer.write(R"("newClientOrderId":")"sv).write(request_id).write(R"(")"sv);
  if (!std::isnan(create_order.price)) {
    writer.write(R"(,"price":")"sv).write(Decimal{create_order.price, order.price_precision.precision}).write(R"(")"sv);
  }
  writer.write(R"(,"quantity":")"sv).write(Decimal{create_order.quantity, order.quantity_precision.precision}).write(R"(")"sv);
  writer.write(R"(,"recvWindow":)"sv).write(recv_window.count());
  writer.write(R"(,"side":")"sv).write(side.as_raw_text()).write(R"(")"sv);
  if (!std::isnan(create_order.stop_price)) {
    writer.write(R"(,"stopPrice":")"sv).write(Decimal{create_order.stop_price, order.price_precision.precision}).write(R"(")"sv);
  }
  writer.write(R"(,"symbol":")"sv).write(create_order.symbol).write(R"(")"sv);
  if (time_in_force != json::TimeInForce{}) {
    writer.write(R"(,"timeInForce":")"sv).write(time_in_force.as_raw_text()).write(R"(")"sv);
  }
  writer.write(R"(,"timestamp":)"sv).write(now_utc.count());
  writer.write(R"(,"type":")"sv).write(type.as_raw_text()).write(R"(")"sv);
  writer.write("}"sv);
  return writer.finish();
#endif
}

// order-modify

// note! both quantity and price must be sent (different from dapi/rest)
std::string_view Encoder::order_modify_json(
    std::vector<char> &buffer,
    roq::ModifyOrder const &modify_order,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::chrono::milliseconds now_utc,
    std::string_view const &id) {
  auto side = map(order.side).template get<Side>();
  auto quantity = std::isnan(modify_order.quantity) ? order.quantity : modify_order.quantity;
  auto price = std::isnan(modify_order.price) ? order.price : modify_order.price;
#if defined(WS_USE_FMT)
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
      Decimal{quantity, order.quantity_precision.precision},
      Decimal{price, order.price_precision.precision},
      recv_window.count(),
      now_utc.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
#else
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("{"sv);
  writer.write(R"("newClientOrderId":")"sv).write(request_id).write(R"(")"sv);
  if (!std::empty(order.external_order_id)) {
    writer.write(R"(,"orderId":)"sv).write(order.external_order_id);  // note! integer
  }
  writer.write(R"(,"origClientOrderId":")"sv).write(previous_request_id).write(R"(")"sv);
  writer.write(R"(,"recvWindow":)"sv).write(recv_window.count());
  if (!std::isnan(modify_order.price)) {
    writer.write(R"(,"price":")"sv).write(Decimal{modify_order.price, order.price_precision.precision}).write(R"(")"sv);
  }
  if (!std::isnan(modify_order.quantity)) {
    writer.write(R"(,"quantity":")"sv).write(Decimal{modify_order.quantity, order.quantity_precision.precision}).write(R"(")"sv);
  }
  writer.write(R"(,"side":")"sv).write(side.as_raw_text()).write(R"(")"sv);
  writer.write(R"(,"symbol":")"sv).write(order.symbol).write(R"(")"sv);
  writer.write(R"(,"timestamp":)"sv).write(now_utc.count());
  writer.write("}"sv);
  return writer.finish();
#endif
}

// order-cancel

std::string_view Encoder::order_cancel_json(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::chrono::milliseconds now_utc,
    std::string_view const &id) {
#if defined(WS_USE_FMT)
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
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
#else
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("{"sv);
  writer.write(R"("newClientOrderId":")"sv).write(request_id).write(R"(")"sv);
  if (!std::empty(order.external_order_id)) {
    writer.write(R"(,"orderId":)"sv).write(order.external_order_id);  // note! integer
  }
  writer.write(R"(,"origClientOrderId":")"sv).write(previous_request_id).write(R"(")"sv);
  writer.write(R"(,"recvWindow":)"sv).write(recv_window.count());
  writer.write(R"(,"symbol":")"sv).write(order.symbol).write(R"(")"sv);
  writer.write(R"(,"timestamp":)"sv).write(now_utc.count());
  writer.write("}"sv);
  return writer.finish();
#endif
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
