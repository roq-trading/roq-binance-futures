/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/gateway/request.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace gateway {

// === IMPLEMENTATION ===

Request::Request(Shared &shared)
    : balances{*this}, positions{*this}, positions_pm{*this}, open_orders{*this}, stale_orders{*this}, trades{*this}, shared_{shared} {
}

bool Request::ready() const {
  return balances.ready() && positions.ready() && positions_pm.ready() && open_orders.ready() && stale_orders.ready() && trades.ready();
}

bool Request::can_request(TraceInfo const &trace_info) {
  return shared_.rate_limiter.can_request(trace_info);
}

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
