/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.hpp"

#include "roq/server.hpp"

#include "roq/binance_futures/json/error.hpp"
#include "roq/binance_futures/json/result.hpp"

#include "roq/binance_futures/json/agg_trade.hpp"
#include "roq/binance_futures/json/book_ticker.hpp"
#include "roq/binance_futures/json/depth_update.hpp"
#include "roq/binance_futures/json/mark_price_update.hpp"
#include "roq/binance_futures/json/mini_ticker.hpp"

namespace roq {
namespace binance_futures {
namespace json {

struct MarketStreamParser final {
  struct Handler {
    // response
    virtual void operator()(const Trace<Error> &, int32_t id) = 0;
    virtual void operator()(const Trace<Result> &, int32_t id) = 0;
    // update
    virtual void operator()(const Trace<AggTrade> &) = 0;
    virtual void operator()(const Trace<MiniTicker> &) = 0;
    virtual void operator()(const Trace<BookTicker> &) = 0;
    virtual void operator()(const Trace<DepthUpdate> &) = 0;
    virtual void operator()(const Trace<MarkPriceUpdate> &) = 0;
  };

  static void dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const TraceInfo &);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
