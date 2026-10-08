/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/server/stream.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct OrderEntry : public server::OrderActionStream {
  // note! a bit dirty -- only valid for WSAPI
  virtual void force_listen_key_refresh() {}
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
