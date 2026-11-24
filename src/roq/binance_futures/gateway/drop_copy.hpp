/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/server/stream.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct DropCopy : public server::Stream {
  using server::Stream::Stream;

  virtual void update_listen_key(TraceInfo const &, std::string_view const &listen_key) = 0;
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
