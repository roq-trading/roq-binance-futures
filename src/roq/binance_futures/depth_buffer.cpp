/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/binance_futures/depth_buffer.h"

#include <utility>

#include "roq/server.h"

#include "roq/core/back_emplacer.h"

#include "roq/binance_futures/flags.h"

namespace roq {
namespace binance_futures {

namespace {
template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.price,
      .quantity = value.qty,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}
}  // namespace

DepthBuffer::DepthBuffer(
    Shared &shared,
    uint16_t stream_id,
    const std::string_view &symbol,
    double tick_size,
    double min_trade_vol)
    : shared_(shared), stream_id_(stream_id), symbol_(symbol),
      market_by_price_(
          core::market::MarketByPrice::Config{
              .type = {},
              .max_depth = server::Flags::cache_mbp_max_depth(),
              .allow_fractional_tick_size = {},
              .allow_remove_non_existing = true,  // note! documented on website
          },
          symbol) {
  // XXX should be constructor
  ReferenceData reference_data{
      .stream_id = stream_id,
      .exchange = {},
      .symbol = {},
      .description = {},
      .security_type = {},
      .currency = {},
      .settlement_currency = {},
      .commission_currency = {},
      .tick_size = tick_size,
      .multiplier = NaN,
      .min_trade_vol = min_trade_vol,
      .option_type = {},
      .strike_currency = {},
      .strike_price = NaN,
      .underlying = {},
      .time_zone = {},
      .issue_date = {},
      .settlement_date = {},
      .expiry_datetime = {},
      .expiry_datetime_utc = {},
  };
  market_by_price_(reference_data);
}

bool DepthBuffer::update_helper(const json::Depth &depth) {
  log::debug("got snapshot, symbol={}, last_update_id={}"_fmt, symbol_, depth.last_update_id);
  // snapshot
  core::back_emplacer bids(shared_.bids), asks(shared_.asks);
  log::debug("depth={}"_fmt, depth);
  for (auto &item : depth.bids)
    bids.emplace_back([&item](auto &result) { emplace(result, item); });
  for (auto &item : depth.asks)
    asks.emplace_back([&item](auto &result) { emplace(result, item); });
  MarketByPriceUpdate market_by_price_update{
      .stream_id = {},
      .exchange = Flags::exchange(),
      .symbol = symbol_,
      .bids = bids,
      .asks = asks,
      .snapshot = true,
      .exchange_time_utc = {},
  };
  market_by_price_(market_by_price_update);
  log::debug("market_by_price_update={}"_fmt, market_by_price_update);
  for (auto &iter : offsets_)
    log::debug("have symbol={} last_update_id={}"_fmt, symbol_, std::get<0>(iter));
  // apply updates
  std::tuple<uint64_t, size_t, size_t> value{depth.last_update_id, 0, 0};
  auto begin = std::upper_bound(offsets_.begin(), offsets_.end(), value, [](auto &lhs, auto &rhs) {
    return std::get<0>(lhs) < std::get<0>(rhs);
  });
  if (begin != offsets_.end()) {
    log::debug("symbol={}, last_update_id={}"_fmt, symbol_, std::get<0>(*begin));
    auto &previous = *begin;
    while (++begin != offsets_.end()) {
      auto &current = *begin;
      assert(std::get<1>(current) >= std::get<1>(previous));
      assert(std::get<2>(current) >= std::get<2>(previous));
      MarketByPriceUpdate market_by_price_update{
          .stream_id = {},
          .exchange = Flags::exchange(),
          .symbol = symbol_,
          .bids = {&bids_[std::get<1>(previous)], std::get<1>(current) - std::get<1>(previous)},
          .asks = {&asks_[std::get<2>(previous)], std::get<2>(current) - std::get<2>(previous)},
          .snapshot = false,
          .exchange_time_utc = {},
      };
      log::debug("apply: market_by_price_update={}"_fmt, market_by_price_update);
      market_by_price_(market_by_price_update);
      previous = current;
    }
    assert(bids_.size() >= std::get<1>(previous));
    assert(asks_.size() >= std::get<2>(previous));
    MarketByPriceUpdate market_by_price_update{
        .stream_id = {},
        .exchange = Flags::exchange(),
        .symbol = symbol_,
        .bids = {&bids_[std::get<1>(previous)], bids_.size() - std::get<1>(previous)},
        .asks = {&asks_[std::get<2>(previous)], asks_.size() - std::get<2>(previous)},
        .snapshot = false,
        .exchange_time_utc = {},
    };
    log::debug("apply: market_by_price_update={}"_fmt, market_by_price_update);
    market_by_price_(market_by_price_update);
  } else {
    log::debug("found nothing symbol={}"_fmt, symbol_);
  }
  bids_.clear();
  asks_.clear();
  offsets_.clear();
  ready_ = true;
  log::debug("READY symbol={}"_fmt, symbol_);
  return ready_;
}

std::pair<size_t, size_t> DepthBuffer::update_helper(const json::DepthUpdate &depth_update) {
  if (ROQ_LIKELY(ready_)) {
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (auto &item : depth_update.bids)
      bids.emplace_back([&item](auto &result) { emplace(result, item); });
    for (auto &item : depth_update.asks)
      asks.emplace_back([&item](auto &result) { emplace(result, item); });
    return std::make_pair(bids.size(), asks.size());
  } else {
    offsets_.emplace_back(depth_update.final_update_id, bids_.size(), asks_.size());
    for (auto &item : depth_update.bids)
      bids_.emplace_back(MBPUpdate{
          .price = item.price,
          .quantity = item.qty,
          .implied_quantity = NaN,
          .price_level = {},
          .number_of_orders = {},
      });
    for (auto &item : depth_update.asks)
      asks_.emplace_back(MBPUpdate{
          .price = item.price,
          .quantity = item.qty,
          .implied_quantity = NaN,
          .price_level = {},
          .number_of_orders = {},
      });
    return std::make_pair(bids_.size(), asks_.size());
  }
}

}  // namespace binance_futures
}  // namespace roq
