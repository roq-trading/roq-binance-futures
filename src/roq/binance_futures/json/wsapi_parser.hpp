/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/wsapi_request.hpp"

#include "roq/binance_futures/json/wsapi_account_balance.hpp"
#include "roq/binance_futures/json/wsapi_account_position.hpp"
#include "roq/binance_futures/json/wsapi_account_status.hpp"
#include "roq/binance_futures/json/wsapi_error.hpp"
#include "roq/binance_futures/json/wsapi_listen_key.hpp"
#include "roq/binance_futures/json/wsapi_open_orders.hpp"
#include "roq/binance_futures/json/wsapi_open_orders_cancel_all.hpp"
#include "roq/binance_futures/json/wsapi_order_cancel.hpp"
#include "roq/binance_futures/json/wsapi_order_modify.hpp"
#include "roq/binance_futures/json/wsapi_order_place.hpp"
#include "roq/binance_futures/json/wsapi_session_logon.hpp"
#include "roq/binance_futures/json/wsapi_trades.hpp"

namespace roq {
namespace binance_futures {
namespace json {

struct WSAPIParser final {
  struct Handler {
    virtual void operator()(Trace<WSAPIError> const &) = 0;
    virtual void operator()(Trace<WSAPISessionLogon> const &) = 0;
    virtual void operator()(Trace<WSAPIListenKey> const &) = 0;
    virtual void operator()(Trace<WSAPIAccountBalance> const &) = 0;
    virtual void operator()(Trace<WSAPIAccountStatus> const &) = 0;
    virtual void operator()(Trace<WSAPIAccountPosition> const &) = 0;
    virtual void operator()(Trace<WSAPIOpenOrders> const &) = 0;
    virtual void operator()(Trace<WSAPITrades> const &) = 0;
    virtual void operator()(Trace<WSAPIOpenOrdersCancelAll> const &, WSAPIRequest const &) = 0;
    virtual void operator()(Trace<WSAPIOrderPlace> const &, WSAPIRequest const &) = 0;
    virtual void operator()(Trace<WSAPIOrderModify> const &, WSAPIRequest const &) = 0;
    virtual void operator()(Trace<WSAPIOrderCancel> const &, WSAPIRequest const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types = false);
};

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
