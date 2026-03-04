/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/contract_status.hpp"
#include "roq/binance_futures/json/symbol_status.hpp"

namespace roq {
namespace binance_futures {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  std::visit(
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

extern TradingStatus trading_status_helper(SymbolStatus, ContractStatus);

inline double get_unrealized_pnl(double um_unrealized_pnl, double cm_unrealized_pnl) {
  auto has_um = !std::isnan(um_unrealized_pnl);
  auto has_cm = !std::isnan(cm_unrealized_pnl);
  if (!has_um && !has_cm) {
    return NaN;
  }
  return (has_um ? um_unrealized_pnl : 0.0) + (has_cm ? cm_unrealized_pnl : 0.0);
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
