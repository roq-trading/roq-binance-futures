/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/shared.hpp"

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : settings{settings}, api{API::create()}, dispatcher_{dispatcher},
      rate_limiter{settings.common.request_limit, settings.common.request_limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream}, depth_request_queue{settings.ws.mbp_request_delay} {
}

}  // namespace binance_futures
}  // namespace roq
