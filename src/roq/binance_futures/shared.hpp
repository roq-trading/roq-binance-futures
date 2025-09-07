/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/utils/container.hpp"

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

  template <typename... Args>
  auto get_ref_data(Args &&...args) {
    return dispatcher_.get_ref_data(std::forward<Args>(args)...);
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
    explicit Instrument(Settings const &);

    int64_t tob_last_update_id = {};
    int64_t mbp_last_update_id = {};
    market::mbp::Sequencer sequencer;

    bool tob_update(int64_t update_id) {
      if (update_id < tob_last_update_id) {
        return false;
      }
      tob_last_update_id = update_id;
      return true;
    }

    bool mbp_update(int64_t update_id) {
      if (update_id < mbp_last_update_id) {
        return false;
      }
      mbp_last_update_id = update_id;
      return true;
    }
  };

  Instrument &get_instrument(std::string_view const &symbol);

 private:
  utils::unordered_map<std::string, Instrument> instruments_;

  server::Dispatcher &dispatcher_;

 public:
  core::limit::RateLimiter rate_limiter;
  core::Symbols symbols;
  core::TimerQueue<std::string> depth_request_queue;
  core::TimerQueue<std::string> time_series_request_queue;
  std::vector<RateLimit> rate_limits;

  struct {
    uint32_t request_weight_1m = {};
    uint32_t create_order_10s = {};
    uint32_t create_order_1m = {};
  } limits;

  std::vector<Bar> bars;

  bool const allow_unknown_event_types;
};

}  // namespace binance_futures
}  // namespace roq
