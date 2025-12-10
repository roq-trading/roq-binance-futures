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
  // URL

  // user-trades

  static std::string_view user_trades_url(
      std::vector<char> &buffer,
      std::string_view const &symbol,
      std::chrono::milliseconds start_time,
      std::chrono::milliseconds end_time,
      uint32_t limit,
      std::chrono::milliseconds recv_window);

  // order-place

  static std::string_view order_place_url(
      std::vector<char> &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id, std::chrono::milliseconds recv_window);

  // order-modify

  static std::string_view order_modify_url(
      std::vector<char> &buffer,
      roq::ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      bool modify_order_full);

  // order-cancel

  static std::string_view order_cancel_url(
      std::vector<char> &buffer,
      roq::CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window);

  // all-open-orders

  static std::string_view all_open_orders_url(std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds recv_window);

  // countdown

  static std::string_view countdown_cancel_open_orders_url(
      std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds countdown_time, std::chrono::milliseconds recv_window);

  // JSON

  // session-logon

  static std::string_view session_logon_json(
      std::vector<char> &buffer,
      std::string_view const &api_key,
      std::chrono::milliseconds now_utc,
      std::string_view const &signature,
      std::string_view const &id);

  // user-data-stream-start

  static std::string_view user_data_stream_start_json(std::vector<char> &buffer, std::string_view const &api_key, std::string_view const &id);

  // user-data-stream-ping

  static std::string_view user_data_stream_ping_json(std::vector<char> &buffer, std::string_view const &api_key, std::string_view const &id);

  // account-balance

  static std::string_view account_balance_json(std::vector<char> &buffer, std::chrono::milliseconds now_utc, std::string_view const &id);

  // account-status

  static std::string_view account_status_json(std::vector<char> &buffer, std::chrono::milliseconds now_utc, std::string_view const &id);

  // account-position

  static std::string_view account_position_json(std::vector<char> &buffer, std::chrono::milliseconds now_utc, std::string_view const &id);

  // order-status

  static std::string_view order_status_json(
      std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds now_utc, std::string_view const &id);

  // order-place

  static std::string_view order_place_json(
      std::vector<char> &buffer,
      CreateOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::chrono::milliseconds recv_window,
      std::chrono::milliseconds now_utc,
      std::string_view const &id);

  // order-modify

  static std::string_view order_modify_json(
      std::vector<char> &buffer,
      roq::ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      std::chrono::milliseconds now_utc,
      std::string_view const &id);

  // order-cancel

  static std::string_view order_cancel_json(
      std::vector<char> &buffer,
      roq::CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::chrono::milliseconds recv_window,
      std::chrono::milliseconds now_utc,
      std::string_view const &id);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
