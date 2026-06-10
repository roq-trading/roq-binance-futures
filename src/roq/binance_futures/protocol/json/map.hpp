/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/binance_futures/protocol/json/contract_status.hpp"
#include "roq/binance_futures/protocol/json/contract_type.hpp"
#include "roq/binance_futures/protocol/json/order_status.hpp"
#include "roq/binance_futures/protocol/json/order_type.hpp"
#include "roq/binance_futures/protocol/json/side.hpp"
#include "roq/binance_futures/protocol/json/symbol_status.hpp"
#include "roq/binance_futures/protocol/json/time_in_force.hpp"

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
std::optional<TradingStatus> Map<binance_futures::protocol::json::ContractStatus>::helper() const;

template <>
template <>
std::optional<SecurityType> Map<binance_futures::protocol::json::ContractType>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<binance_futures::protocol::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<RequestStatus> Map<binance_futures::protocol::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<binance_futures::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<Side> Map<binance_futures::protocol::json::Side>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<binance_futures::protocol::json::SymbolStatus>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<binance_futures::protocol::json::TimeInForce>::helper() const;

// ===

template <>
template <>
std::optional<binance_futures::protocol::json::OrderStatus> Map<OrderStatus>::helper() const;

template <>
template <>
std::optional<binance_futures::protocol::json::OrderType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<binance_futures::protocol::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<binance_futures::protocol::json::TimeInForce> Map<TimeInForce, Mask<ExecutionInstruction>>::helper() const;

}  // namespace roq
