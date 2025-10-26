/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "roq/cancel_all_orders.hpp"
#include "roq/cancel_order.hpp"
#include "roq/create_order.hpp"
#include "roq/modify_order.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace binance_futures {
namespace json {

struct Encoder final {
  static std::string_view trades(
      std::vector<char> &buffer,
      std::string_view const &symbol,
      std::chrono::milliseconds start_time,
      std::chrono::milliseconds end_time,
      uint32_t limit,
      std::chrono::milliseconds recv_window);

  // new

  static std::string_view new_order(
      std::vector<char> &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id, std::chrono::milliseconds recv_window);

  static std::string_view new_order_ws_url(
      std::vector<char> &buffer,
      CreateOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::chrono::milliseconds recv_window,
      std::string_view const &api_key,
      std::chrono::milliseconds now);

  static std::string_view new_order_ws_json(
      std::vector<char> &buffer,
      CreateOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::chrono::milliseconds recv_window,
      std::chrono::milliseconds now);

  // modify

  static std::string_view modify_order(
      std::vector<char> &buffer,
      roq::ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      bool modify_order_full);

  static std::string_view modify_order_ws_url(
      std::vector<char> &buffer,
      roq::ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      std::string_view const &api_key,
      std::chrono::milliseconds now);

  static std::string_view modify_order_ws_json(
      std::vector<char> &buffer,
      roq::ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      std::chrono::milliseconds now);

  // cancel

  static std::string_view cancel_order(
      std::vector<char> &buffer,
      roq::CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window);

  static std::string_view cancel_order_ws_url(
      std::vector<char> &buffer,
      roq::CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      std::string_view const &api_key,
      std::chrono::milliseconds now);

  static std::string_view cancel_order_ws_json(
      std::vector<char> &buffer,
      roq::CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      std::chrono::milliseconds now);

  static std::string_view cancel_all_open_orders(std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds recv_window);

  static std::string_view countdown_cancel_all_open_orders(
      std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds countdown_time, std::chrono::milliseconds recv_window);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
