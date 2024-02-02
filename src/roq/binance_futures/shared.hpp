/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/symbols.hpp"
#include "roq/core/timer_queue.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/market/mbp/sequencer.hpp"

#include "roq/binance_futures/api.hpp"
#include "roq/binance_futures/settings.hpp"

namespace roq {
namespace binance_futures {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared &&) = default;
  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

 public:
  Settings const &settings;
  API const api;

 private:
  struct {
    std::vector<MBPUpdate> bids, asks;
    auto &clear() {
      bids.clear();
      asks.clear();
      return *this;
    }
    bool empty() const { return std::empty(bids) && std::empty(asks); }
  } mbp;

 public:
  auto &get_mbp() { return mbp.clear(); }

  struct Instrument final {
    int64_t tob_last_update_id = {};
    int64_t mbp_last_update_id = {};
    market::mbp::Sequencer sequencer;

    bool tob_update(int64_t update_id) {
      if (update_id < tob_last_update_id)
        return false;
      tob_last_update_id = update_id;
      return true;
    }

    bool mbp_update(int64_t update_id) {
      if (update_id < mbp_last_update_id)
        return false;
      mbp_last_update_id = update_id;
      return true;
    }
  };
  absl::flat_hash_map<Symbol, Instrument> instruments;

 private:
  server::Dispatcher &dispatcher_;

 public:
  core::limit::RateLimiter rate_limiter;
  core::Symbols symbols;
  core::TimerQueue<std::string> depth_request_queue;
};

}  // namespace binance_futures
}  // namespace roq
