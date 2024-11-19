/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <vector>

#include "roq/server/oms/order.hpp"

#include "roq/utils/patterns.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/core/json/parser.hpp"

namespace roq {
namespace binance_futures {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  return std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = result_type{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = result_type{value}; },
          [&](double value) { result = result_type{static_cast<int64_t>(value)}; },
          [&](std::string_view const &value) { result = utils::charconv::from_chars<result_type>(value, utils::charconv::Format::DATETIME); },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

extern roq::Error guess_error(int32_t code);

extern std::string_view trades(
    std::vector<char> &buffer,
    std::string_view const &symbol,
    std::chrono::milliseconds start_time,
    std::chrono::milliseconds end_time,
    uint32_t limit,
    std::chrono::milliseconds recv_window);

// new

extern std::string_view new_order(
    std::vector<char> &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id, std::chrono::milliseconds recv_window);

extern std::string_view new_order_ws_url(
    std::vector<char> &buffer,
    CreateOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now);

extern std::string_view new_order_ws_json(
    std::vector<char> &buffer,
    CreateOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now,
    std::string_view const &signature);

// modify

extern std::string_view modify_order(
    std::vector<char> &buffer,
    roq::ModifyOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    bool modify_order_full);

extern std::string_view modify_order_ws_url(
    std::vector<char> &buffer,
    roq::ModifyOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now);

extern std::string_view modify_order_ws_json(
    std::vector<char> &buffer,
    roq::ModifyOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now,
    std::string_view const &signature);

// cancel

extern std::string_view cancel_order(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window);

extern std::string_view cancel_order_ws_url(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now);

extern std::string_view cancel_order_ws_json(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    std::chrono::milliseconds recv_window,
    std::string_view const &api_key,
    std::chrono::milliseconds now,
    std::string_view const &signature);

extern std::string_view cancel_all_open_orders(std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds recv_window);

extern std::string_view countdown_cancel_all_open_orders(
    std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds countdown_time, std::chrono::milliseconds recv_window);

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
