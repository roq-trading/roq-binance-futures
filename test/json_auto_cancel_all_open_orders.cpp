/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/auto_cancel_all_open_orders.h"

using namespace roq;
using namespace roq::binance_futures;

TEST(json_auto_cancel_all_open_orders, simple) {
  auto message = R"({)"
                 R"("symbol":"XRPUSDT",)"
                 R"("countdownTime":"30000")"
                 R"(})";
  auto obj = core::json::Parser::create<json::AutoCancelAllOpenOrders>(message);
  EXPECT_EQ(obj.symbol, "XRPUSDT"_sv);
  EXPECT_EQ(obj.countdown_time, 30000);
}
