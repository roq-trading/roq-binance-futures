/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.h"

#include "roq/server.h"

#include "roq/binance_futures/json/error.h"
#include "roq/binance_futures/json/result.h"

#include "roq/binance_futures/json/agg_trade.h"
#include "roq/binance_futures/json/book_ticker.h"
#include "roq/binance_futures/json/depth_update.h"
#include "roq/binance_futures/json/mark_price_update.h"
#include "roq/binance_futures/json/mini_ticker.h"

namespace roq {
namespace binance_futures {
namespace json {

struct MarketStreamParser final {
  struct Handler {
    // response
    virtual void operator()(const server::Trace<Error> &, int32_t id) = 0;
    virtual void operator()(const server::Trace<Result> &, int32_t id) = 0;
    // update
    virtual void operator()(const server::Trace<AggTrade> &) = 0;
    virtual void operator()(const server::Trace<MiniTicker> &) = 0;
    virtual void operator()(const server::Trace<BookTicker> &) = 0;
    virtual void operator()(const server::Trace<DepthUpdate> &) = 0;
    virtual void operator()(const server::Trace<MarkPriceUpdate> &) = 0;
  };

  static void dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const server::TraceInfo &);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
