/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

namespace roq {
namespace binance_futures {
namespace gateway {

struct AssetsUpdate final {
  std::span<std::string_view> assets;
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
