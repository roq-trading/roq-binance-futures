/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/binance_futures/shared.h"

#include "roq/binance_futures/flags.h"

namespace roq {
namespace binance_futures {

Shared::Shared(server::Dispatcher &dispatcher)
    : bids(server::Flags::cache_mbp_max_depth()), asks(server::Flags::cache_mbp_max_depth()),
      final_bids(server::Flags::cache_mbp_max_depth()),
      final_asks(server::Flags::cache_mbp_max_depth()), dispatcher_(dispatcher),
      rate_limiter_(Flags::request_limit(), Flags::request_limit_interval()) {
}

}  // namespace binance_futures
}  // namespace roq
