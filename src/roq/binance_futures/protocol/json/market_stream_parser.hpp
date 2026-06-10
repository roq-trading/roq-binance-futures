/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/protocol/json/error.hpp"
#include "roq/binance_futures/protocol/json/result.hpp"

#include "roq/binance_futures/protocol/json/agg_trade.hpp"
#include "roq/binance_futures/protocol/json/book_ticker.hpp"
#include "roq/binance_futures/protocol/json/depth_update.hpp"
#include "roq/binance_futures/protocol/json/kline.hpp"
#include "roq/binance_futures/protocol/json/mark_price_update.hpp"
#include "roq/binance_futures/protocol/json/mini_ticker.hpp"
#include "roq/binance_futures/protocol/json/trade_2.hpp"

namespace roq {
namespace binance_futures {
namespace protocol {
namespace json {

struct MarketStreamParser final {
  struct Handler {
    // response
    virtual void operator()(Trace<Error> const &, int32_t id) = 0;
    virtual void operator()(Trace<Result> const &, int32_t id) = 0;
    // update
    virtual void operator()(Trace<Trade2> const &) = 0;
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
}  // namespace protocol
}  // namespace binance_futures
}  // namespace roq
