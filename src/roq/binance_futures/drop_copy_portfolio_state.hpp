/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace binance_futures {

enum class DropCopyPortfolioState : uint8_t {
  UNDEFINED = 0,
  BALANCE,
  ACCOUNT,
  POSITION,
  ORDERS,
  TRADES,
  DONE,
};

}  // namespace binance_futures
}  // namespace roq
