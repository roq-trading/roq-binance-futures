/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/json/utils.hpp"

#include "roq/decimal.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/utils/text/writer.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

Error guess_error([[maybe_unused]] int32_t code) {
  return Error::UNKNOWN;
}

std::string_view trades(
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

// new

std::string_view new_order(
    std::vector<char> &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window) {
  auto side = map(create_order.side).as_raw_text();
  auto type = map(create_order.order_type).as_raw_text();
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
      side,
      type,
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
      auto time_in_force = map(create_order.time_in_force).as_raw_text();
      fmt::format_to(
          std::back_inserter(buffer),
          R"(timeInForce={}&)"
          R"(price={}&)"sv,
          time_in_force,
          Decimal{create_order.price, order.price_precision.precision});
      break;
    }
  }
  if (!std::isnan(create_order.stop_price))
    fmt::format_to(
        std::back_inserter(buffer),
        R"(stopPrice={}&)"sv,
        Decimal{create_order.stop_price, order.price_precision.precision});
  fmt::format_to(
      std::back_inserter(buffer),
      R"(newClientOrderId={}&)"
      R"(recvWindow={})"sv,
      request_id,
      recv_window.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

std::string_view new_order_ws_url(
    std::vector<char> &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now) {
  auto side = map(create_order.side);
  auto type = map(create_order.order_type);
  auto time_in_force = map(create_order.time_in_force);
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  if (!std::empty(api_key))
    writer.write("apiKey="sv).write(api_key).write("&"sv);
  writer.write("newClientOrderId="sv).write(request_id);
  if (!std::isnan(create_order.price))
    writer.write("&price="sv).write(Decimal{create_order.price, order.price_precision.precision});
  writer.write("&quantity="sv).write(Decimal{create_order.quantity, order.quantity_precision.precision});
  writer.write("&recvWindow="sv).write(recv_window.count());
  writer.write("&side="sv).write(side.as_raw_text());
  if (!std::isnan(create_order.stop_price))
    writer.write("&stopPrice="sv).write(Decimal{create_order.stop_price, order.price_precision.precision});
  writer.write("&symbol="sv).write(create_order.symbol);
  if (time_in_force != json::TimeInForce{})
    writer.write("&timeInForce="sv).write(time_in_force.as_raw_text());
  writer.write("&timestamp="sv).write(now.count());
  writer.write("&type="sv).write(type.as_raw_text());
  return writer.finish();
}

std::string_view new_order_ws_json(
    std::vector<char> &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now,
    std::string_view const &signature) {
  auto side = map(create_order.side);
  auto type = map(create_order.order_type);
  auto time_in_force = map(create_order.time_in_force);
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("{"sv);
  writer.write(R"("apiKey":")"sv).write(api_key).write(R"(")"sv);
  writer.write(R"(,"newClientOrderId":")"sv).write(request_id).write(R"(")"sv);
  if (!std::isnan(create_order.price))
    writer.write(R"(,"price":")"sv).write(Decimal{create_order.price, order.price_precision.precision}).write(R"(")"sv);
  writer.write(R"(,"quantity":")"sv)
      .write(Decimal{create_order.quantity, order.quantity_precision.precision})
      .write(R"(")"sv);
  writer.write(R"(,"recvWindow":)"sv).write(recv_window.count());
  writer.write(R"(,"side":")"sv).write(side.as_raw_text()).write(R"(")"sv);
  if (!std::isnan(create_order.stop_price))
    writer.write(R"(,"stopPrice":")"sv)
        .write(Decimal{create_order.stop_price, order.price_precision.precision})
        .write(R"(")"sv);
  writer.write(R"(,"symbol":")"sv).write(create_order.symbol).write(R"(")"sv);
  if (time_in_force != json::TimeInForce{})
    writer.write(R"(,"timeInForce":")"sv).write(time_in_force.as_raw_text()).write(R"(")"sv);
  writer.write(R"(,"timestamp":)"sv).write(now.count());
  writer.write(R"(,"type":")"sv).write(type.as_raw_text()).write(R"(")"sv);
  writer.write(R"(,"signature":")"sv).write(signature).write(R"(")"sv);
  writer.write("}"sv);
  return writer.finish();
}

// modify

std::string_view modify_order(
    std::vector<char> &buffer,
    roq::ModifyOrder const &modify_order,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    bool modify_order_full) {
  buffer.clear();
  auto side = map(order.side).as_raw_text();
  if (modify_order_full) {  // fapi
    auto quantity = std::isnan(modify_order.quantity) ? order.quantity : modify_order.quantity;
    auto price = std::isnan(modify_order.price) ? order.price : modify_order.price;
    fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
    if (!std::empty(order.client_order_id)) {
      fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
    } else {
      fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
    }
    fmt::format_to(
        std::back_inserter(buffer),
        R"(side={}&)"
        R"(quantity={}&)"
        R"(price={}&)"
        R"(recvWindow={})"sv,
        side,
        Decimal{quantity, order.quantity_precision.precision},
        Decimal{price, order.price_precision.precision},
        recv_window.count());
  } else {  // dapi
    if (std::isnan(modify_order.price)) {
      fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
      if (!std::empty(order.client_order_id)) {
        fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
      } else {
        fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
      }
      fmt::format_to(
          std::back_inserter(buffer),
          R"(side={}&)"
          R"(quantity={}&)"
          R"(recvWindow={})"sv,
          side,
          Decimal{modify_order.quantity, order.quantity_precision.precision},
          recv_window.count());
    } else if (std::isnan(modify_order.quantity)) {
      fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
      if (!std::empty(order.client_order_id)) {
        fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
      } else {
        fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
      }
      fmt::format_to(
          std::back_inserter(buffer),
          R"(side={}&)"
          R"(price={}&)"
          R"(recvWindow={})"sv,
          side,
          Decimal{modify_order.price, order.price_precision.precision},
          recv_window.count());
    } else {
      throw server::oms::Rejected{Origin::GATEWAY, Error::INVALID_REQUEST_ARGS, "Missing quantity or price"sv};
    }
  }
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

std::string_view modify_order_ws_url(
    std::vector<char> &buffer,
    roq::ModifyOrder const &modify_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now) {
  auto side = map(order.side).as_raw_text();
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("apiKey="sv).write(api_key);
  writer.write("&newClientOrderId="sv).write(request_id);
  if (!std::empty(order.external_order_id))
    writer.write("&orderId="sv).write(order.external_order_id);
  writer.write("&origClientOrderId="sv).write(previous_request_id);
  if (!std::isnan(modify_order.price))
    writer.write("&price="sv).write(Decimal{modify_order.price, order.price_precision.precision});
  if (!std::isnan(modify_order.quantity))
    writer.write("&quantity="sv).write(Decimal{modify_order.quantity, order.quantity_precision.precision});
  writer.write("&recvWindow="sv).write(recv_window.count());
  writer.write("&side="sv).write(side);
  writer.write("&symbol="sv).write(order.symbol);
  writer.write("&timestamp="sv).write(now.count());
  return writer.finish();
}

std::string_view modify_order_ws_json(
    std::vector<char> &buffer,
    roq::ModifyOrder const &modify_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now,
    std::string_view const &signature) {
  auto side = map(order.side).as_raw_text();
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("{"sv);
  writer.write(R"("apiKey":")"sv).write(api_key).write(R"(")"sv);
  writer.write(R"(,"newClientOrderId":")"sv).write(request_id).write(R"(")"sv);
  if (!std::empty(order.external_order_id))
    writer.write(R"(,"orderId":)"sv).write(order.external_order_id);  // note! integer
  writer.write(R"(,"origClientOrderId":")"sv).write(previous_request_id).write(R"(")"sv);
  writer.write(R"(,"recvWindow":)"sv).write(recv_window.count());
  if (!std::isnan(modify_order.price))
    writer.write(R"(,"price":")"sv).write(Decimal{modify_order.price, order.price_precision.precision}).write(R"(")"sv);
  if (!std::isnan(modify_order.quantity))
    writer.write(R"(,"quantity":")"sv)
        .write(Decimal{modify_order.quantity, order.quantity_precision.precision})
        .write(R"(")"sv);
  writer.write(R"(,"side":")"sv).write(side).write(R"(")"sv);
  writer.write(R"(,"symbol":")"sv).write(order.symbol).write(R"(")"sv);
  writer.write(R"(,"timestamp":)"sv).write(now.count());
  writer.write(R"(,"signature":")"sv).write(signature).write(R"(")"sv);
  writer.write("}"sv);
  return writer.finish();
}

// cancel

std::string_view cancel_order(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(std::back_inserter(buffer), R"(symbol={}&)"sv, order.symbol);
  if (!std::empty(order.client_order_id)) {
    fmt::format_to(std::back_inserter(buffer), R"(origClientOrderId={}&)"sv, order.client_order_id);
  } else {
    fmt::format_to(std::back_inserter(buffer), R"(orderId={}&)"sv, order.external_order_id);
  }
  fmt::format_to(std::back_inserter(buffer), R"(recvWindow={})"sv, recv_window.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

std::string_view cancel_order_ws_url(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now) {
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("apiKey="sv).write(api_key);
  writer.write("&newClientOrderId="sv).write(request_id);
  if (!std::empty(order.external_order_id))
    writer.write("&orderId="sv).write(order.external_order_id);
  writer.write("&origClientOrderId="sv).write(previous_request_id);
  writer.write("&recvWindow="sv).write(recv_window.count());
  writer.write("&symbol="sv).write(order.symbol);
  writer.write("&timestamp="sv).write(now.count());
  return writer.finish();
}

std::string_view cancel_order_ws_json(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now,
    std::string_view const &signature) {
  buffer.resize(512);
  std::span buffer_2{reinterpret_cast<std::byte *>(std::data(buffer)), std::size(buffer)};
  utils::text::Writer writer{buffer_2};
  writer.write("{"sv);
  writer.write(R"("apiKey":")"sv).write(api_key).write(R"(")"sv);
  writer.write(R"(,"newClientOrderId":")"sv).write(request_id).write(R"(")"sv);
  if (!std::empty(order.external_order_id))
    writer.write(R"(,"orderId":)"sv).write(order.external_order_id);  // note! integer
  writer.write(R"(,"origClientOrderId":")"sv).write(previous_request_id).write(R"(")"sv);
  writer.write(R"(,"recvWindow":)"sv).write(recv_window.count());
  writer.write(R"(,"symbol":")"sv).write(order.symbol).write(R"(")"sv);
  writer.write(R"(,"timestamp":)"sv).write(now.count());
  writer.write(R"(,"signature":")"sv).write(signature).write(R"(")"sv);
  writer.write("}"sv);
  return writer.finish();
}

std::string_view cancel_all_open_orders(
    std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds recv_window) {
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

std::string_view countdown_cancel_all_open_orders(
    std::vector<char> &buffer,
    std::string_view const &symbol,
    std::chrono::milliseconds countdown_time,
    std::chrono::milliseconds recv_window) {
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

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
