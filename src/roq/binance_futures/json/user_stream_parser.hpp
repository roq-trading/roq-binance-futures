/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/account_config_update.hpp"
#include "roq/binance_futures/json/account_update.hpp"
#include "roq/binance_futures/json/balance_update.hpp"
#include "roq/binance_futures/json/execution_report_2.hpp"
#include "roq/binance_futures/json/grid_update.hpp"
#include "roq/binance_futures/json/liability_change.hpp"
#include "roq/binance_futures/json/margin_call.hpp"
#include "roq/binance_futures/json/order_trade_update.hpp"
#include "roq/binance_futures/json/outbound_account_position.hpp"
#include "roq/binance_futures/json/strategy_update.hpp"
#include "roq/binance_futures/json/trade_lite.hpp"

namespace roq {
namespace binance_futures {
namespace json {

struct UserStreamParser final {
  struct Handler {
    virtual void operator()(Trace<OrderTradeUpdate> const &) = 0;
    virtual void operator()(Trace<AccountUpdate> const &) = 0;
    virtual void operator()(Trace<MarginCall> const &) = 0;
    virtual void operator()(Trace<StrategyUpdate> const &) = 0;
    virtual void operator()(Trace<GridUpdate> const &) = 0;
    virtual void operator()(Trace<AccountConfigUpdate> const &) = 0;
    virtual void operator()(Trace<TradeLite> const &) = 0;
    virtual void operator()(Trace<ExecutionReport2> const &) = 0;
    virtual void operator()(Trace<BalanceUpdate> const &) = 0;
    virtual void operator()(Trace<LiabilityChange> const &) = 0;
    virtual void operator()(Trace<OutboundAccountPosition> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
