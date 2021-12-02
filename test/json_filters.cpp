/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/filters.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_filters, simple_1) {
  auto message = R"([{)"
                 R"("minPrice":"556.72",)"
                 R"("maxPrice":"4529764",)"
                 R"("filterType":"PRICE_FILTER",)"
                 R"("tickSize":"0.01")"
                 R"(},{)"
                 R"("stepSize":"0.001",)"
                 R"("filterType":"LOT_SIZE",)"
                 R"("maxQty":"1000",)"
                 R"("minQty":"0.001")"
                 R"(},{)"
                 R"("stepSize":"0.001",)"
                 R"("filterType":"MARKET_LOT_SIZE",)"
                 R"("maxQty":"300",)"
                 R"("minQty":"0.001")"
                 R"(},{)"
                 R"("limit":200,)"
                 R"("filterType":"MAX_NUM_ORDERS")"
                 R"(},{)"
                 R"("limit":10,)"
                 R"("filterType":"MAX_NUM_ALGO_ORDERS")"
                 R"(},{)"
                 R"("notional":"5",)"
                 R"("filterType":"MIN_NOTIONAL")"
                 R"(},{)"
                 R"("multiplierDown":"0.9500",)"
                 R"("multiplierUp":"1.0500",)"
                 R"("multiplierDecimal":"4",)"
                 R"("filterType":"PERCENT_PRICE")"
                 R"(})"
                 R"(])";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::Filters>(message, buffer);
  auto &data = obj.data;
  ASSERT_EQ(std::size(data), 7);
  auto &d0 = data[0];
  EXPECT_DOUBLE_EQ(d0.min_price, 556.72);
  EXPECT_DOUBLE_EQ(d0.max_price, 4529764.0);
  EXPECT_EQ(d0.filter_type, json::FilterType::PRICE_FILTER);
  EXPECT_DOUBLE_EQ(d0.tick_size, 0.01);
  auto &d1 = data[1];
  EXPECT_DOUBLE_EQ(d1.step_size, 0.001);
  EXPECT_EQ(d1.filter_type, json::FilterType::LOT_SIZE);
  EXPECT_DOUBLE_EQ(d1.max_qty, 1000.0);
  EXPECT_DOUBLE_EQ(d1.min_qty, 0.001);
  auto &d2 = data[2];
  EXPECT_DOUBLE_EQ(d2.step_size, 0.001);
  EXPECT_EQ(d2.filter_type, json::FilterType::MARKET_LOT_SIZE);
  EXPECT_DOUBLE_EQ(d2.max_qty, 300.0);
  EXPECT_DOUBLE_EQ(d2.min_qty, 0.001);
  auto &d3 = data[3];
  EXPECT_EQ(d3.limit, 200);
  EXPECT_EQ(d3.filter_type, json::FilterType::MAX_NUM_ORDERS);
  auto &d4 = data[4];
  EXPECT_EQ(d4.limit, 10);
  EXPECT_EQ(d4.filter_type, json::FilterType::MAX_NUM_ALGO_ORDERS);
  auto &d5 = data[5];
  EXPECT_EQ(d5.notional, 5);
  EXPECT_EQ(d5.filter_type, json::FilterType::MIN_NOTIONAL);
  auto &d6 = data[6];
  EXPECT_DOUBLE_EQ(d6.multiplier_down, 0.95);
  EXPECT_DOUBLE_EQ(d6.multiplier_up, 1.05);
  EXPECT_EQ(d6.multiplier_decimal, 4);
  EXPECT_EQ(d6.filter_type, json::FilterType::PERCENT_PRICE);
}
