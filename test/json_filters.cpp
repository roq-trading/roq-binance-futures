/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/filters.hpp"

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
  core::json::BufferStack buffer{8192, 1};
  json::Filters obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 7);
  auto &data_0 = data[0];
  CHECK(data_0.min_price == 556.72_a);
  CHECK(data_0.max_price == 4529764.0_a);
  CHECK(data_0.filter_type == json::FilterType::PRICE_FILTER);
  CHECK(data_0.tick_size == 0.01_a);
  auto &data_1 = data[1];
  CHECK(data_1.step_size == 0.001_a);
  CHECK(data_1.filter_type == json::FilterType::LOT_SIZE);
  CHECK(data_1.max_qty == 1000.0_a);
  CHECK(data_1.min_qty == 0.001_a);
  auto &data_2 = data[2];
  CHECK(data_2.step_size == 0.001_a);
  CHECK(data_2.filter_type == json::FilterType::MARKET_LOT_SIZE);
  CHECK(data_2.max_qty == 300.0_a);
  CHECK(data_2.min_qty == 0.001_a);
  auto &data_3 = data[3];
  CHECK(data_3.limit == 200);
  CHECK(data_3.filter_type == json::FilterType::MAX_NUM_ORDERS);
  auto &data_4 = data[4];
  CHECK(data_4.limit == 10);
  CHECK(data_4.filter_type == json::FilterType::MAX_NUM_ALGO_ORDERS);
  auto &data_5 = data[5];
  CHECK(data_5.notional == 5.0_a);
  CHECK(data_5.filter_type == json::FilterType::MIN_NOTIONAL);
  auto &data_6 = data[6];
  CHECK(data_6.multiplier_down == 0.95_a);
  CHECK(data_6.multiplier_up == 1.05_a);
  CHECK(data_6.multiplier_decimal == 4);
  CHECK(data_6.filter_type == json::FilterType::PERCENT_PRICE);
}
