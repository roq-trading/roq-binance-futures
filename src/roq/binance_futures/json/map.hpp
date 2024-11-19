/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <tuple>

#include "roq/api.hpp"

#include "roq/binance_futures/json/contract_type.hpp"
#include "roq/binance_futures/json/order_status.hpp"
#include "roq/binance_futures/json/order_type.hpp"
#include "roq/binance_futures/json/side.hpp"
#include "roq/binance_futures/json/symbol_status.hpp"
#include "roq/binance_futures/json/time_in_force.hpp"

namespace roq {
namespace binance_futures {
namespace json {

template <typename... Args>
struct Map final {
  explicit Map(Args &&...args) : args_{std::forward<Args>(args)...} {}
  explicit Map(Args const &...args) : args_{args...} {}

  Map(Map const &) = delete;

  template <typename R>
  operator R();

 private:
  std::tuple<Args...> const args_;
};

template <typename R, typename... Args>
inline R map(Args &&...args) {
  return static_cast<R>(Map{std::forward<Args>(args)...});
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
