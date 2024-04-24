/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <vector>

#include "roq/server/oms/order.hpp"

#include "roq/utils/patterns.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/charconv/datetime.hpp"

#include "roq/binance_futures/json/contract_type.hpp"
#include "roq/binance_futures/json/order_status.hpp"
#include "roq/binance_futures/json/order_type.hpp"
#include "roq/binance_futures/json/side.hpp"
#include "roq/binance_futures/json/symbol_status.hpp"
#include "roq/binance_futures/json/time_in_force.hpp"

namespace roq {
namespace binance_futures {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  return std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = std::chrono::milliseconds{value}; },
          [&](double value) { result = std::chrono::milliseconds{static_cast<int64_t>(value)}; },
          [&](std::string_view const &value) {
            result = core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(value);
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

inline roq::SecurityType map(json::ContractType value) {
  switch (value) {
    using enum json::ContractType::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case PERPETUAL:
      return roq::SecurityType::SWAP;
    case CURRENT_QUARTER:
      return roq::SecurityType::FUTURES;
    case NEXT_QUARTER:
      return roq::SecurityType::FUTURES;
    case CURRENT_QUARTER_DELIVERING:
      return roq::SecurityType::FUTURES;
    case PERPETUAL_DELIVERING:
      return roq::SecurityType::FUTURES;
  }
  return {};
}

inline roq::OrderStatus map(json::OrderStatus side) {
  switch (side) {
    using enum json::OrderStatus::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case NEW:
      return roq::OrderStatus::WORKING;
    case PARTIALLY_FILLED:
      return roq::OrderStatus::WORKING;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
    case EXPIRED:
      break;
    case NEW_INSURANCE:
      break;
    case NEW_ADL:
      break;
  }
  return {};
}

inline json::OrderStatus map(roq::OrderStatus side) {
  switch (side) {
    using enum roq::OrderStatus;
    case UNDEFINED:
      break;
    case SENT:
      break;
    case ACCEPTED:
      break;
    case SUSPENDED:
      break;
    case WORKING:
      return json::OrderStatus::NEW;
      // return json::OrderStatus::PARTIALLY_FILLED;
    case STOPPED:
      break;
    case COMPLETED:
      return json::OrderStatus::FILLED;
    case EXPIRED:
      return json::OrderStatus::EXPIRED;
    case CANCELED:
      return json::OrderStatus::CANCELED;
    case REJECTED:
      break;
  }
  return {};
}

inline roq::OrderType map(json::OrderType side) {
  switch (side) {
    using enum json::OrderType::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case MARKET:
      return roq::OrderType::MARKET;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case STOP:
      break;
    case TAKE_PROFIT:
      break;
    case LIQUIDATION:
      break;
  }
  return {};
}

inline json::OrderType map(roq::OrderType side) {
  switch (side) {
    using enum roq::OrderType;
    case UNDEFINED:
      break;
    case MARKET:
      return json::OrderType::MARKET;
    case LIMIT:
      return json::OrderType::LIMIT;
  }
  return {};
}

inline roq::Side map(json::Side side) {
  switch (side) {
    using enum json::Side::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

inline json::Side map(roq::Side side) {
  switch (side) {
    using enum roq::Side;
    case UNDEFINED:
      break;
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  return {};
}

inline roq::TradingStatus map(json::SymbolStatus symbol_status) {
  switch (symbol_status) {
    using enum json::SymbolStatus::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case TRADING:
      return roq::TradingStatus::OPEN;
    case HALT:
      return roq::TradingStatus::HALT;
    case BREAK:
      return roq::TradingStatus::CLOSE;
    case END_OF_DAY:
      return roq::TradingStatus::END_OF_DAY;
      // note! following probably not used (not sure if also applies to futures)
      // - https://dev.binance.vision/t/explanation-on-symbol-status/118
    case PRE_TRADING:
      return roq::TradingStatus::PRE_OPEN;
    case AUCTION_MATCH:
      return roq::TradingStatus::PRE_OPEN;
    case POST_TRADING:
      return roq::TradingStatus::CLOSE;
      // note! have found no documentation
    case SETTLING:                           // note! no idea what this is for
      return roq::TradingStatus::PRE_CLOSE;  // XXX REVIEW
    case PENDING_TRADING:                    // note! no idea what this is for
      return roq::TradingStatus::PRE_OPEN;   // XXX REVIEW
    case DELIVERING:
      return roq::TradingStatus::UNDEFINED;
  }
  return {};
}

inline roq::TimeInForce map(json::TimeInForce time_in_force) {
  switch (time_in_force) {
    using enum json::TimeInForce::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case GTC:
      return roq::TimeInForce::GTC;
    case IOC:
      return roq::TimeInForce::IOC;
    case FOK:
      return roq::TimeInForce::FOK;
    case GTX:
      return roq::TimeInForce::GTX;
  }
  return {};
}

inline json::TimeInForce map(roq::TimeInForce time_in_force) {
  switch (time_in_force) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      break;
    case GFD:
      break;
    case GTC:
      return json::TimeInForce::GTC;
    case OPG:
      break;
    case IOC:
      return json::TimeInForce::IOC;
    case FOK:
      return json::TimeInForce::FOK;
    case GTX:
      return json::TimeInForce::GTX;
    case GTD:
      break;
    case AT_THE_CLOSE:
      break;
    case GOOD_THROUGH_CROSSING:
      break;
    case AT_CROSSING:
      break;
    case GOOD_FOR_TIME:
      break;
    case GFA:
      break;
    case GFM:
      break;
  }
  return {};
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
    std::vector<char> &buffer,
    CreateOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::chrono::milliseconds recv_window);

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

extern std::string_view cancel_all_open_orders(
    std::vector<char> &buffer, std::string_view const &symbol, std::chrono::milliseconds recv_window);

extern std::string_view countdown_cancel_all_open_orders(
    std::vector<char> &buffer,
    std::string_view const &symbol,
    std::chrono::milliseconds countdown_time,
    std::chrono::milliseconds recv_window);

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
