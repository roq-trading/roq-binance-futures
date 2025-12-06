/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/order_place_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::OrderPlaceAck;

TEST_CASE("json_new_order_simple", "[json_order_place_ack]") {
  auto message = R"({)"
                 R"("orderId":17759343290,)"
                 R"("symbol":"XRPUSDT",)"
                 R"("status":"NEW",)"
                 R"("clientOrderId":"qQAC6QMAAQAAVtg9ctAW",)"
                 R"("price":"1.0847",)"
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
                 R"("updateTime":1634543725791)"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.order_id == 17759343290);
    CHECK(obj.symbol == "XRPUSDT"sv);
    CHECK(obj.status == json::OrderStatus::NEW);
    CHECK(obj.client_order_id == "qQAC6QMAAQAAVtg9ctAW"sv);
    CHECK(obj.price == 1.0847_a);
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
    CHECK(obj.update_time == 1634543725791ms);
  };
  core::json::BufferStack buffers{65536, 2};
  value_type obj{message, buffers};
  helper(obj);
}
