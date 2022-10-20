/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/binance_futures/json/utils.hpp"

#include "roq/utils/number.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

Error guess_error(int32_t code) {
  return Error::UNKNOWN;
}

std::string_view new_order(
    std::vector<char> &buffer,
    CreateOrder const &create_order,
    oms::Order const &order,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window) {
  auto side = map(create_order.side).as_raw_text();
  auto type = map(create_order.order_type).as_raw_text();
  auto time_in_force = map(create_order.time_in_force).as_raw_text();
  auto reduce_only = false;
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(side={}&)"
      R"(type={}&)"
      R"(timeInForce={}&)"
      R"(quantity={}&)"
      R"(reduceOnly={}&)"
      R"(price={}&)"sv,
      create_order.symbol,
      side,
      type,
      time_in_force,
      utils::Number{create_order.quantity, order.quantity_decimals},
      reduce_only,
      utils::Number{create_order.price, order.price_decimals});
  if (!std::isnan(create_order.stop_price))
    fmt::format_to(
        std::back_inserter(buffer), R"(stopPrice={}&)"sv, utils::Number{create_order.stop_price, order.price_decimals});
  fmt::format_to(
      std::back_inserter(buffer),
      R"(newClientOrderId={}&)"
      R"(recvWindow={})"sv,
      request_id,
      recv_window.count());
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

std::string_view cancel_order(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"(symbol={}&)"
      R"(origClientOrderId={}&)"
      R"(recvWindow={})"sv,
      order.symbol,
      previous_request_id,
      recv_window.count());
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
