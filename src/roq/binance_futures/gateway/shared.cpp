/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/gateway/shared.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

// === HELPERS ===

namespace {
auto create_sequencer(auto &settings) {
  auto options = market::mbp::Sequencer::Options{
      .timeout = settings.mbp.sequencer_timeout,
      .max_updates = {},
  };
  return market::mbp::Sequencer{options};
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher{dispatcher}, settings{settings}, api{API::create(settings)}, rate_limiter{settings.request.limit, settings.request.limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream}, depth_request_queue{settings.ws.mbp_request_delay},
      allow_unknown_event_types{settings.experimental.allow_unknown_event_types || settings.misc.continue_with_unknown_event_type} {
}

Shared::Instrument &Shared::get_instrument(std::string_view const &symbol) {
  auto iter = instruments_.find(symbol);
  if (iter == std::end(instruments_)) [[unlikely]] {
    auto res = instruments_.try_emplace(symbol, settings);
    assert(res.second);
    iter = res.first;
  }
  return (*iter).second;
}

// instrument

Shared::Instrument::Instrument(Settings const &settings) : sequencer{create_sequencer(settings)} {
}

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
