/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/margin_call.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_margin_call, online_example) {
  auto message = R"({)"
                 R"("e":"MARGIN_CALL",)"
                 R"("E":1587727187525,)"
                 R"("cw":"3.16812045",)"
                 R"("p":[{)"
                 R"("s":"ETHUSDT",)"
                 R"("ps":"LONG",)"
                 R"("pa":"1.327",)"
                 R"("mt":"CROSSED",)"
                 R"("iw":"0",)"
                 R"("mp":"187.17127",)"
                 R"("up":"-1.166074",)"
                 R"("mm":"1.614445")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::MarginCall>(message, buffer);
  EXPECT_EQ(obj.event_type, json::EventType::MARGIN_CALL);
  EXPECT_EQ(obj.event_time, 1587727187525ms);
  EXPECT_DOUBLE_EQ(obj.cross_wallet_balance, 3.16812045);
  auto &positions = obj.positions;
  ASSERT_EQ(std::size(positions), 1);
  auto &p0 = positions[0];
  EXPECT_EQ(p0.symbol, "ETHUSDT"sv);
}
