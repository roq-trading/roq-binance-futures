/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/market_stream_parser.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_mini_ticker, simple_coin_m) {
  auto message = R"({)"
                 R"("e":"24hrMiniTicker",)"
                 R"("E":1640248670092,)"
                 R"("s":"BTCUSD_220325",)"
                 R"("ps":"BTCUSD",)"
                 R"("c":"49461.1",)"
                 R"("o":"50769.1",)"
                 R"("h":"50903.5",)"
                 R"("l":"49230.6",)"
                 R"("v":"1913789",)"
                 R"("q":"3833.39491534")"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::MiniTicker>(message, buffer);
  EXPECT_EQ(obj.event_type, json::EventType::_24HR_MINI_TICKER);
  EXPECT_EQ(obj.event_time, 1640248670092ms);
  EXPECT_EQ(obj.symbol, "BTCUSD_220325"sv);
  EXPECT_EQ(obj.pair, "BTCUSD"sv);
  EXPECT_DOUBLE_EQ(obj.close_price, 49461.1);
  EXPECT_DOUBLE_EQ(obj.open_price, 50769.1);
  EXPECT_DOUBLE_EQ(obj.high_price, 50903.5);
  EXPECT_DOUBLE_EQ(obj.low_price, 49230.6);
  EXPECT_DOUBLE_EQ(obj.total_traded_base_asset_volume, 1913789.0);
  EXPECT_DOUBLE_EQ(obj.total_traded_quote_asset_volume, 3833.39491534);
}
