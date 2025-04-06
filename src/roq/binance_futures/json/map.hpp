/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/binance_futures/json/contract_status.hpp"
#include "roq/binance_futures/json/contract_type.hpp"
#include "roq/binance_futures/json/order_status.hpp"
#include "roq/binance_futures/json/order_type.hpp"
#include "roq/binance_futures/json/side.hpp"
#include "roq/binance_futures/json/symbol_status.hpp"
#include "roq/binance_futures/json/time_in_force.hpp"

#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/security_type.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"
#include "roq/trading_status.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<TradingStatus> Map<binance_futures::json::ContractStatus>::helper() const;

template <>
template <>
std::optional<SecurityType> Map<binance_futures::json::ContractType>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<binance_futures::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<binance_futures::json::OrderType>::helper() const;

template <>
template <>
std::optional<Side> Map<binance_futures::json::Side>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<binance_futures::json::SymbolStatus>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<binance_futures::json::TimeInForce>::helper() const;

// ===

template <>
template <>
std::optional<binance_futures::json::OrderStatus> Map<OrderStatus>::helper() const;

template <>
template <>
std::optional<binance_futures::json::OrderType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<binance_futures::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<binance_futures::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
