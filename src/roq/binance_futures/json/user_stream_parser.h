/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.h"

#include "roq/server.h"

#include "roq/binance_futures/json/event_type.h"

#include "roq/binance_futures/json/account_update.h"
#include "roq/binance_futures/json/margin_call.h"
#include "roq/binance_futures/json/order_trade_update.h"

namespace roq {
namespace binance_futures {
namespace json {

struct UserStreamParser final {
  struct Handler {
    virtual void operator()(const server::Trace<OrderTradeUpdate> &) = 0;
    virtual void operator()(const server::Trace<AccountUpdate> &) = 0;
    virtual void operator()(const server::Trace<MarginCall> &) = 0;
  };

  static void dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const server::TraceInfo &trace);

 private:
  static bool try_dispatch(
      UserStreamParser::Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      EventType event_type,
      const server::TraceInfo &trace);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
