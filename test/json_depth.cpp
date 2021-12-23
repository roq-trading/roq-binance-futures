/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/depth.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

// note! truncated
TEST(json_depth, simple_coin_m) {
  auto message = R"({)"
                 R"("lastUpdateId":300700442819,)"
                 R"("E":1640249388590,)"
                 R"("T":1640249388573,)"
                 R"("symbol":"BTCUSD_220325",)"
                 R"("pair":"BTCUSD",)"
                 R"("bids":[)"
                 R"(["49498.2","217"],)"
                 R"(["49493.9","84"])"
                 R"(],)"
                 R"("asks":[)"
                 R"(["49498.3","184"],)"
                 R"(["49498.8","7"])"
                 R"(])"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::Depth>(message, buffer);
}
