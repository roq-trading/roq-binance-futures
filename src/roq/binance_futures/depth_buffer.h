/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>
#include <tuple>

#include "roq/core/memory.h"

#include "roq/core/market/market_by_price.h"

#include "roq/binance_futures/flags.h"
#include "roq/binance_futures/shared.h"

#include "roq/binance_futures/json/depth.h"
#include "roq/binance_futures/json/depth_update.h"

namespace roq {
namespace binance_futures {

struct DepthBuffer final {
  DepthBuffer(
      Shared &,
      uint16_t stream_id,
      const std::string_view &symbol,
      double tick_size,
      double min_trade_vol);

  DepthBuffer(DepthBuffer &&) = default;
  DepthBuffer(const DepthBuffer &) = delete;

  // request snap?
  // request timeout?
  // correlate time of request with what we get?
  // reference data ???

  template <typename F>
  void operator()(const std::string_view &symbol, const json::Depth &depth, F callback) {
    if (!ready_) {
      update_helper(depth);
      if (ready_) {
        auto result = market_by_price_.extract(shared_.bids, shared_.asks);
        MarketByPriceUpdate market_by_price_update{
            .stream_id = stream_id_,
            .exchange = Flags::exchange(),
            .symbol = symbol,
            .bids = {shared_.bids.data(), result.first},
            .asks = {shared_.asks.data(), result.second},
            .snapshot = true,
            .exchange_time_utc = {},
        };
        callback(market_by_price_update);
      }
    }
  }

  template <typename F>
  void operator()(const json::DepthUpdate &depth_update, F callback) {
    auto result = update_helper(depth_update);
    if (ready_) {
      MarketByPriceUpdate market_by_price_update{
          .stream_id = stream_id_,
          .exchange = Flags::exchange(),
          .symbol = depth_update.symbol,
          .bids = {shared_.bids.data(), result.first},
          .asks = {shared_.asks.data(), result.second},
          .snapshot = false,
          .exchange_time_utc = {},
      };
      callback(market_by_price_update);
    }
  }

 protected:
  bool update_helper(const json::Depth &);
  std::pair<size_t, size_t> update_helper(const json::DepthUpdate &);

 private:
  Shared &shared_;
  const uint16_t stream_id_;
  const std::string symbol_;
  core::market::MarketByPrice market_by_price_;
  core::page_aligned_vector<MBPUpdate> bids_, asks_;
  core::page_aligned_vector<std::tuple<uint64_t, size_t, size_t>> offsets_;
  bool ready_ = false;
};

}  // namespace binance_futures
}  // namespace roq
