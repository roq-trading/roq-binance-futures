/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/error.hpp"
#include "roq/binance_futures/json/result.hpp"

#include "roq/binance_futures/json/agg_trade.hpp"
#include "roq/binance_futures/json/book_ticker.hpp"
#include "roq/binance_futures/json/depth_update.hpp"
#include "roq/binance_futures/json/kline.hpp"
#include "roq/binance_futures/json/mark_price_update.hpp"
#include "roq/binance_futures/json/mini_ticker.hpp"

namespace roq {
namespace binance_futures {
namespace json {

struct MarketStreamParser final {
  struct Handler {
    // response
    virtual void operator()(Trace<Error> const &, int32_t id) = 0;
    virtual void operator()(Trace<Result> const &, int32_t id) = 0;
    // update
    virtual void operator()(Trace<AggTrade> const &) = 0;
    virtual void operator()(Trace<MiniTicker> const &) = 0;
    virtual void operator()(Trace<BookTicker> const &) = 0;
    virtual void operator()(Trace<DepthUpdate> const &) = 0;
    virtual void operator()(Trace<MarkPriceUpdate> const &) = 0;
    virtual void operator()(Trace<Kline> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
