/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/binance_futures/shared.h"

#include "roq/logging.h"

#include "roq/utils/update.h"

#include "roq/binance_futures/flags.h"

namespace roq {
namespace binance_futures {

Shared::Shared(server::Dispatcher &dispatcher)
    : gateway_settings(dispatcher.get_gateway_settings()),
      bids(server::Flags::cache_mbp_max_depth()), asks(server::Flags::cache_mbp_max_depth()),
      final_bids(server::Flags::cache_mbp_max_depth()),
      final_asks(server::Flags::cache_mbp_max_depth()), dispatcher_(dispatcher) {
}

bool Shared::can_request_helper(std::chrono::nanoseconds now) {
  auto size = std::size(request_history_);
  if (size >= Flags::request_limit()) {
    auto period = now - request_history_.front();
    if (period < Flags::request_limit_interval()) {
      if (utils::update(request_is_blocked_, true)) {
        auto remaining = Flags::request_limit_interval() - period;
        log::debug("BLOCK"_sv);
        log::warn<1>(
            "Requests are being blocked for {} second(s)"_sv,
            std::chrono::duration_cast<std::chrono::seconds>(remaining));
      }
      return false;
    }
  }
  request_history_.pop_front();
  request_history_.push_back(now);
  if (utils::update(request_is_blocked_, false)) {
    log::debug("UNBLOCK"_sv);
    log::info<1>("Requests are now unblocked"_sv);
  }
  return true;
}

}  // namespace binance_futures
}  // namespace roq
