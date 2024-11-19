/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/binance_futures/json/open_orders.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_open_orders_simple_empty", "[json_open_orders]") {
  auto message = R"([])";
  std::vector<std::byte> buffer(8192);
  json::OpenOrders obj{message, buffer};
  REQUIRE(std::size(obj.data) == 0);
}

TEST_CASE("json_open_orders_simple_1", "[json_open_orders]") {
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
  std::vector<std::byte> buffer(8192);
  json::OpenOrders obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
  auto &d0 = data[0];
  CHECK(d0.order_id == 17759938812);
  CHECK(d0.symbol == "XRPUSDT"sv);
  CHECK(d0.status == json::OrderStatus::NEW);
  CHECK(d0.client_order_id == "GgAC6gMAAQAANhJPG9EW"sv);
  CHECK(d0.price == 1.0765_a);
  CHECK(d0.avg_price == 0.0_a);
  CHECK(d0.orig_qty == 5.0_a);
  CHECK(d0.executed_qty == 0.0_a);
  CHECK(d0.cum_quote == 0.0_a);
  CHECK(d0.time_in_force == json::TimeInForce::GTC);
  CHECK(d0.type == json::OrderType::LIMIT);
  CHECK(d0.reduce_only == false);
  CHECK(d0.close_position == false);
  CHECK(d0.side == json::Side::BUY);
  CHECK(d0.position_side == json::PositionSide::BOTH);
  CHECK(d0.stop_price == 0.0_a);
  CHECK(d0.working_type == json::WorkingType::CONTRACT_PRICE);
  CHECK(d0.price_protect == false);
  CHECK(d0.orig_type == json::OrderType::LIMIT);
  CHECK(d0.time == 1634546562277ms);
  CHECK(d0.update_time == 1634546562277ms);
}
