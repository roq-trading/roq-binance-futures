/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/cancel_order.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::chrono_literals;

TEST(json_cancel_order, simple) {
  auto message = R"({)"
                 R"("orderId":17759646892,)"
                 R"("symbol":"XRPUSDT",)"
                 R"("status":"CANCELED",)"
                 R"("clientOrderId":"rwAC6QMAAQAAM0V8zdAW",)"
                 R"("price":"1.0823",)"
                 R"("avgPrice":"0.00000",)"
                 R"("origQty":"5",)"
                 R"("executedQty":"0",)"
                 R"("cumQty":"0",)"
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
                 R"("updateTime":1634545259912)"
                 R"(})";
  auto obj = core::json::Parser::create<json::CancelOrder>(message);
  EXPECT_EQ(obj.order_id, 17759646892);
  EXPECT_EQ(obj.symbol, "XRPUSDT"_sv);
  EXPECT_EQ(obj.status, json::OrderStatus::CANCELED);
  EXPECT_EQ(obj.client_order_id, "rwAC6QMAAQAAM0V8zdAW"_sv);
  EXPECT_DOUBLE_EQ(obj.price, 1.0823);
  EXPECT_DOUBLE_EQ(obj.avg_price, 0.0);
  EXPECT_DOUBLE_EQ(obj.orig_qty, 5.0);
  EXPECT_DOUBLE_EQ(obj.executed_qty, 0.0);
  EXPECT_DOUBLE_EQ(obj.cum_qty, 0.0);
  EXPECT_DOUBLE_EQ(obj.cum_quote, 0.0);
  EXPECT_EQ(obj.time_in_force, json::TimeInForce::GTC);
  EXPECT_EQ(obj.type, json::OrderType::LIMIT);
  EXPECT_EQ(obj.reduce_only, false);
  EXPECT_EQ(obj.close_position, false);
  EXPECT_EQ(obj.side, json::Side::BUY);
  EXPECT_EQ(obj.position_side, json::PositionSide::BOTH);
  EXPECT_DOUBLE_EQ(obj.stop_price, 0.0);
  EXPECT_EQ(obj.working_type, json::WorkingType::CONTRACT_PRICE);
  EXPECT_EQ(obj.price_protect, false);
  EXPECT_EQ(obj.orig_type, json::OrderType::LIMIT);
  EXPECT_EQ(obj.update_time, 1634545259912ms);
}
