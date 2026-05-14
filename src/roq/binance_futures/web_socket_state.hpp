/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace binance_futures {

enum class WebSocketState : uint8_t {
  UNDEFINED = 0,
  SESSION_LOGON,
  USER_DATA_STREAM_START,
  ACCOUNT_POSITION,
  DONE,
};

}  // namespace binance_futures
}  // namespace roq
