/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/shared.hpp"

#include "roq/binance_futures/flags.hpp"

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : settings{settings}, api{API::create()}, dispatcher_{dispatcher},
      rate_limiter{Flags::request_limit(), Flags::request_limit_interval()},
      symbols{Flags::ws_max_subscriptions_per_stream()}, depth_request_queue{Flags::ws_mbp_request_delay()} {
}

}  // namespace binance_futures
}  // namespace roq
