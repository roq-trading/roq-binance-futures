/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/binance_futures/shared.h"

#include <magic_enum.hpp>

#include "roq/binance_futures/flags.h"

namespace roq {
namespace binance_futures {

Shared::Shared(server::Dispatcher &dispatcher)
    : bids(server::Flags::cache_mbp_max_depth()), asks(server::Flags::cache_mbp_max_depth()),
      dispatcher_(dispatcher) {
}

}  // namespace binance_futures
}  // namespace roq
