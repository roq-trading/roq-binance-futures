/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>

namespace roq {
namespace binance_futures {

struct Request final {
  // balance
  std::chrono::nanoseconds request_balance = {};
  std::chrono::nanoseconds respond_balance = {};
  // account
  std::chrono::nanoseconds request_account = {};
  std::chrono::nanoseconds respond_account = {};
  // orders
  std::chrono::nanoseconds request_orders = {};
  std::chrono::nanoseconds respond_orders = {};
  // trades
  std::chrono::nanoseconds request_trades = {};
  std::chrono::nanoseconds respond_trades = {};
};

}  // namespace binance_futures
}  // namespace roq
