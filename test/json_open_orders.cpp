/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/open_orders.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_open_orders, simple_empty) {
  auto message = R"([])";
  core::Buffer buffer_(8192);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer);
  ASSERT_EQ(std::size(obj.data), 0);
}

TEST(json_open_orders, simple_1) {
  auto message = R"([{)"
                 R"("orderId":17759938812,)"
                 R"("symbol":"XRPUSDT",)"
                 R"("status":"NEW",)"
                 R"("clientOrderId":"GgAC6gMAAQAANhJPG9EW",)"
                 R"("price":"1.0765",)"
                 R"("avgPrice":"0",)"
                 R"("origQty":"5",)"
                 R"("executedQty":"0",)"
                 R"("cumQuote":"0",)"
                 R"("timeInForce":"GTC",)"
                 R"("type":"LIMIT",)"
                 R"("reduceOnly":false,)"
                 R"("closePosition":false,)"
                 R"("side":"BUY",)"
                 R"("positionSide":"BOTH",)"
                 R"("stopPrice":"0",)"
                 R"("workingType":"CONTRACT_PRICE",)"
                 R"("priceProtect":false,)"
                 R"("origType":"LIMIT",)"
                 R"("time":1634546562277,)"
                 R"("updateTime":1634546562277)"
                 R"(})"
                 R"(])";
  core::Buffer buffer_(8192);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer);
  auto &data = obj.data;
  ASSERT_EQ(std::size(data), 1);
  auto &d0 = data[0];
  EXPECT_EQ(d0.order_id, 17759938812);
  EXPECT_EQ(d0.symbol, "XRPUSDT"sv);
  EXPECT_EQ(d0.status, json::OrderStatus::NEW);
  EXPECT_EQ(d0.client_order_id, "GgAC6gMAAQAANhJPG9EW"sv);
  EXPECT_DOUBLE_EQ(d0.price, 1.0765);
  EXPECT_DOUBLE_EQ(d0.avg_price, 0.0);
  EXPECT_DOUBLE_EQ(d0.orig_qty, 5.0);
  EXPECT_DOUBLE_EQ(d0.executed_qty, 0.0);
  EXPECT_DOUBLE_EQ(d0.cum_quote, 0.0);
  EXPECT_EQ(d0.time_in_force, json::TimeInForce::GTC);
  EXPECT_EQ(d0.type, json::OrderType::LIMIT);
  EXPECT_EQ(d0.reduce_only, false);
  EXPECT_EQ(d0.close_position, false);
  EXPECT_EQ(d0.side, json::Side::BUY);
  EXPECT_EQ(d0.position_side, json::PositionSide::BOTH);
  EXPECT_DOUBLE_EQ(d0.stop_price, 0.0);
  EXPECT_EQ(d0.working_type, json::WorkingType::CONTRACT_PRICE);
  EXPECT_EQ(d0.price_protect, false);
  EXPECT_EQ(d0.orig_type, json::OrderType::LIMIT);
  EXPECT_EQ(d0.time, 1634546562277ms);
  EXPECT_EQ(d0.update_time, 1634546562277ms);
}
