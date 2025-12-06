/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/order_cancel_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::OrderCancelAck;

TEST_CASE("simple", "[json_order_cancel_ack]") {
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
  auto helper = [&](value_type &obj) {
    CHECK(obj.order_id == 17759646892);
    CHECK(obj.symbol == "XRPUSDT"sv);
    CHECK(obj.status == json::OrderStatus::CANCELED);
    CHECK(obj.client_order_id == "rwAC6QMAAQAAM0V8zdAW"sv);
    CHECK(obj.price == 1.0823_a);
    CHECK(obj.avg_price == 0.0_a);
    CHECK(obj.orig_qty == 5.0_a);
    CHECK(obj.executed_qty == 0.0_a);
    CHECK(obj.cum_qty == 0.0_a);
    CHECK(obj.cum_quote == 0.0_a);
    CHECK(obj.time_in_force == json::TimeInForce::GTC);
    CHECK(obj.type == json::OrderType::LIMIT);
    CHECK(obj.reduce_only == false);
    CHECK(obj.close_position == false);
    CHECK(obj.side == json::Side::BUY);
    CHECK(obj.position_side == json::PositionSide::BOTH);
    CHECK(obj.stop_price == 0.0_a);
    CHECK(obj.working_type == json::WorkingType::CONTRACT_PRICE);
    CHECK(obj.price_protect == false);
    CHECK(obj.orig_type == json::OrderType::LIMIT);
    CHECK(obj.update_time == 1634545259912ms);
  };
  core::json::BufferStack buffers{65536, 2};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("cm", "[json_order_cancel_ack]") {
  auto message = R"({)"
                 R"("avgPrice":"0.0",)"
                 R"("clientOrderId":"HAICAAxDgZUBAQAAAAAA",)"
                 R"("cumQty":"0",)"
                 R"("cumBase":"0",)"
                 R"("executedQty":"0",)"
                 R"("orderId":117308066172,)"
                 R"("price":"32000",)"
                 R"("reduceOnly":false,)"
                 R"("side":"BUY",)"
                 R"("positionSide":"BOTH",)"
                 R"("status":"CANCELED",)"
                 R"("symbol":"BTCUSD_PERP",)"
                 R"("pair":"BTCUSD",)"
                 R"("timeInForce":"GTC",)"
                 R"("type":"LIMIT",)"
                 R"("updateTime":1708696325480,)"
                 R"("origQty":"1")"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.avg_price == 0.0_a);
    CHECK(obj.client_order_id == "HAICAAxDgZUBAQAAAAAA"sv);
  };
  core::json::BufferStack buffers{65536, 2};
  value_type obj{message, buffers};
  helper(obj);
}
