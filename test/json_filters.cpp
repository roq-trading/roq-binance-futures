/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/filters.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_filters_simple_1", "[json_filters]") {
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
  REQUIRE(std::size(data) == 7);
  auto &d0 = data[0];
  CHECK(d0.min_price == 556.72_a);
  CHECK(d0.max_price == 4529764.0_a);
  CHECK(d0.filter_type == json::FilterType::PRICE_FILTER);
  CHECK(d0.tick_size == 0.01_a);
  auto &d1 = data[1];
  CHECK(d1.step_size == 0.001_a);
  CHECK(d1.filter_type == json::FilterType::LOT_SIZE);
  CHECK(d1.max_qty == 1000.0_a);
  CHECK(d1.min_qty == 0.001_a);
  auto &d2 = data[2];
  CHECK(d2.step_size == 0.001_a);
  CHECK(d2.filter_type == json::FilterType::MARKET_LOT_SIZE);
  CHECK(d2.max_qty == 300.0_a);
  CHECK(d2.min_qty == 0.001_a);
  auto &d3 = data[3];
  CHECK(d3.limit == 200);
  CHECK(d3.filter_type == json::FilterType::MAX_NUM_ORDERS);
  auto &d4 = data[4];
  CHECK(d4.limit == 10);
  CHECK(d4.filter_type == json::FilterType::MAX_NUM_ALGO_ORDERS);
  auto &d5 = data[5];
  CHECK(d5.notional == 5);
  CHECK(d5.filter_type == json::FilterType::MIN_NOTIONAL);
  auto &d6 = data[6];
  CHECK(d6.multiplier_down == 0.95_a);
  CHECK(d6.multiplier_up == 1.05_a);
  CHECK(d6.multiplier_decimal == 4);
  CHECK(d6.filter_type == json::FilterType::PERCENT_PRICE);
}
