/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "wsapi_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::WSAPIOrderModify;

// fapi

TEST_CASE("fapi_failure", "[json_wsapi_order_modify]") {
  auto message = R"({)"
                 R"("id":"joQeAAwCpIs3B0UAAAAFAAAAAAAAAAAAAAAA",)"
                 R"("status":400,)"
                 R"("error":{)"
                 R"("code":-2013,)"
                 R"("msg":"Order does not exist.")"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":-1,)"
                 R"("count":-1)"
                 R"(},{)"
                 R"("rateLimitType":"ORDERS",)"
                 R"("interval":"SECOND",)"
                 R"("intervalNum":10,)"
                 R"("limit":300,)"
                 R"("count":2)"
                 R"(},{)"
                 R"("rateLimitType":"ORDERS",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":1200,)"
                 R"("count":5)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "joQeAAwCpIs3B0UAAAAFAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 400);
    CHECK(obj.error.code == -2013);
    CHECK(obj.error.msg == "Order does not exist."sv);
    REQUIRE(std::size(obj.rate_limits) == 3);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("fapi_success", "[json_wsapi_order_modify]") {
  auto message = R"({)"
                 R"("id":"i4QeAAwCpIs3B0UAAAACAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":{)"
                 R"("orderId":851545109401,)"
                 R"("symbol":"BTCUSDT",)"
                 R"("status":"NEW",)"
                 R"("clientOrderId":"HgACpIs3B0UAAQAAAAAA",)"
                 R"("price":"32323.00",)"
                 R"("avgPrice":"0.00",)"
                 R"("origQty":"1.000",)"
                 R"("executedQty":"0.000",)"
                 R"("cumQty":"0.000",)"
                 R"("cumQuote":"0.00000",)"
                 R"("timeInForce":"GTC",)"
                 R"("type":"LIMIT",)"
                 R"("reduceOnly":false,)"
                 R"("closePosition":false,)"
                 R"("side":"BUY",)"
                 R"("positionSide":"BOTH",)"
                 R"("stopPrice":"0.00",)"
                 R"("workingType":"CONTRACT_PRICE",)"
                 R"("priceProtect":false,)"
                 R"("origType":"LIMIT",)"
                 R"("priceMatch":"NONE",)"
                 R"("selfTradePreventionMode":"EXPIRE_MAKER",)"
                 R"("goodTillDate":0,)"
                 R"("updateTime":1765337030283)"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":-1,)"
                 R"("count":-1)"
                 R"(},{)"
                 R"("rateLimitType":"ORDERS",)"
                 R"("interval":"SECOND",)"
                 R"("intervalNum":10,)"
                 R"("limit":300,)"
                 R"("count":1)"
                 R"(},{)"
                 R"("rateLimitType":"ORDERS",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":1200,)"
                 R"("count":4)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "i4QeAAwCpIs3B0UAAAACAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 200);
    CHECK(obj.result.order_id == 851545109401);
    REQUIRE(std::size(obj.rate_limits) == 3);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// dapi

TEST_CASE("dapi_failure", "[json_wsapi_order_modify]") {
  auto message = R"({)"
                 R"("id":"h4QeAAwCuEHQB0UAAAADAAAAAAAAAAAAAAAA",)"
                 R"("status":400,)"
                 R"("error":{)"
                 R"("code":-4013,)"
                 R"("msg":"Price less than min price.")"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":-1,)"
                 R"("count":-1)"
                 R"(},{)"
                 R"("rateLimitType":"ORDERS",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":1200,)"
                 R"("count":2)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "h4QeAAwCuEHQB0UAAAADAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 400);
    CHECK(obj.error.code == -4013);
    CHECK(obj.error.msg == "Price less than min price."sv);
    REQUIRE(std::size(obj.rate_limits) == 2);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("dapi_success", "[json_wsapi_order_modify]") {
  auto message = R"({)"
                 R"("id":"iYQeAAwCuUHQB0UAAAACAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":{)"
                 R"("orderId":199338059036,)"
                 R"("symbol":"BTCUSD_PERP",)"
                 R"("pair":"BTCUSD",)"
                 R"("status":"NEW",)"
                 R"("clientOrderId":"KgQCuUHQB0UAAQAAAAAA",)"
                 R"("price":"32323",)"
                 R"("avgPrice":"0.00",)"
                 R"("origQty":"1",)"
                 R"("executedQty":"0",)"
                 R"("cumQty":"0",)"
                 R"("cumBase":"0",)"
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
                 R"("selfTradePreventionMode":"EXPIRE_MAKER",)"
                 R"("updateTime":1765338060809,)"
                 R"("priceMatch":"NONE")"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":-1,)"
                 R"("count":-1)"
                 R"(},{)"
                 R"("rateLimitType":"ORDERS",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":1200,)"
                 R"("count":1)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "iYQeAAwCuUHQB0UAAAACAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 200);
    CHECK(obj.result.order_id == 199338059036);
    REQUIRE(std::size(obj.rate_limits) == 2);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
