/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/protocol/json/order_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::OrderAck;

/*
TEST_CASE("json_order_does_not_exist", "[json_order_ack]") {
  auto message = R"({)"
                 R"("code":-2013,)"
                 R"("msg":"Order does not exist.")"
                 R"(})"sv;
  auto helper = [&](value_type &obj) {
    // REQUIRE(std::size(obj.data) == 0);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
*/
TEST_CASE("json_order_canceled", "[json_order_ack]") {
  auto message = R"({)"
                 R"("orderId":4587027696,)"
                 R"("symbol":"BTCUSD_PERP",)"
                 R"("status":"CANCELED",)"
                 R"("clientOrderId":"vQgCL4I74YYAAQAAAAAA",)"
                 R"("price":"50000.0",)"
                 R"("avgPrice":"0.00",)"
                 R"("origQty":"1",)"
                 R"("executedQty":"0",)"
                 R"("timeInForce":"GTC",)"
                 R"("type":"LIMIT",)"
                 R"("reduceOnly":false,)"
                 R"("closePosition":false,)"
                 R"("side":"BUY",)"
                 R"("positionSide":"BOTH",)"
                 R"("stopPrice":"0.0",)"
                 R"("workingType":"CONTRACT_PRICE",)"
                 R"("priceProtect":false,)"
                 R"("origType":"LIMIT",)"
                 R"("priceMatch":"NONE",)"
                 R"("selfTradePreventionMode":"EXPIRE_MAKER",)"
                 R"("goodTillDate":0,)"
                 R"("pair":"BTCUSD",)"
                 R"("cumBase":"0.00000000",)"
                 R"("time":1793620047271,)"
                 R"("updateTime":1793620050023)"
                 R"(})"sv;
  auto helper = [&](value_type &obj) {
    // REQUIRE(std::size(obj.data) == 0);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
