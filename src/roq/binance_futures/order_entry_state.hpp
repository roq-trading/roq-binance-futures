/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace binance_futures {

enum class OrderEntryState : uint8_t {
  UNDEFINED = 0,
  LISTEN_KEY,
  DONE,
};

}  // namespace binance_futures
}  // namespace roq
