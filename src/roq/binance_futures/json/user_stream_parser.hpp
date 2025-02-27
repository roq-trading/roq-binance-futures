/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/binance_futures/json/event_type.hpp"

#include "roq/binance_futures/json/account_config_update.hpp"
#include "roq/binance_futures/json/account_update.hpp"
#include "roq/binance_futures/json/grid_update.hpp"
#include "roq/binance_futures/json/margin_call.hpp"
#include "roq/binance_futures/json/order_trade_update.hpp"
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
  };

  static bool dispatch(
      Handler &, std::string_view const &message, std::span<std::byte> const &, TraceInfo const &, bool continue_with_unknown_event_type = false);

 private:
  static bool try_dispatch(
      UserStreamParser::Handler &,
      std::string_view const &message,
      std::span<std::byte> const &,
      EventType,
      TraceInfo const &,
      bool continue_with_unknown_event_type);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
