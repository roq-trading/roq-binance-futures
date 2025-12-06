/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/open_orders_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::OpenOrdersAck;

TEST_CASE("json_open_orders_simple_empty", "[json_open_orders_ack]") {
  auto message = R"([])";
  auto helper = [&](value_type &obj) { REQUIRE(std::size(obj.data) == 0); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
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
  auto helper = [&](value_type &obj) {
    auto &data = obj.data;
    REQUIRE(std::size(data) == 1);
    auto &data_0 = data[0];
    CHECK(data_0.order_id == 17759938812);
    CHECK(data_0.symbol == "XRPUSDT"sv);
    CHECK(data_0.status == json::OrderStatus::NEW);
    CHECK(data_0.client_order_id == "GgAC6gMAAQAANhJPG9EW"sv);
    CHECK(data_0.price == 1.0765_a);
    CHECK(data_0.avg_price == 0.0_a);
    CHECK(data_0.orig_qty == 5.0_a);
    CHECK(data_0.executed_qty == 0.0_a);
    CHECK(data_0.cum_quote == 0.0_a);
    CHECK(data_0.time_in_force == json::TimeInForce::GTC);
    CHECK(data_0.type == json::OrderType::LIMIT);
    CHECK(data_0.reduce_only == false);
    CHECK(data_0.close_position == false);
    CHECK(data_0.side == json::Side::BUY);
    CHECK(data_0.position_side == json::PositionSide::BOTH);
    CHECK(data_0.stop_price == 0.0_a);
    CHECK(data_0.working_type == json::WorkingType::CONTRACT_PRICE);
    CHECK(data_0.price_protect == false);
    CHECK(data_0.orig_type == json::OrderType::LIMIT);
    CHECK(data_0.time == 1634546562277ms);
    CHECK(data_0.update_time == 1634546562277ms);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
