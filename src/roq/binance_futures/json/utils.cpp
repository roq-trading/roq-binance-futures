/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/json/utils.hpp"

#include "roq/decimal.hpp"

#include "roq/server/oms/exceptions.hpp"

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
