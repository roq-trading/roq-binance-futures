/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "wsapi_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::WSAPIOrderPlace;

// fapi

TEST_CASE("fapi_failure", "[json_wsapi_order_place]") {
  auto message = R"({)"
                 R"("id":"hYQeAAoCoos3B0UAAAABAAAAAAAAAAAAAAAA",)"
                 R"("status":400,)"
                 R"("error":{)"
                 R"("code":-2027,)"
                 R"("msg":"Exceeded the maximum allowable position at current leverage.")"
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
                 R"("count":1)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "hYQeAAoCoos3B0UAAAABAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 400);
    CHECK(obj.error.code == -2027);
    CHECK(obj.error.msg == "Exceeded the maximum allowable position at current leverage."sv);
    REQUIRE(std::size(obj.rate_limits) == 3);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("fapi_success", "[json_wsapi_order_place]") {
  auto message = R"({)"
                 R"("id":"h4QeAAoCo4s3B0UAAAABAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":{)"
                 R"("orderId":851545009715,)"
                 R"("symbol":"BTCUSDT",)"
                 R"("status":"NEW",)"
                 R"("clientOrderId":"GQACo4s3B0UAAQAAAAAA",)"
                 R"("price":"32000.00",)"
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
                 R"("updateTime":1765337013071)"
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
                 R"("count":2)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "h4QeAAoCo4s3B0UAAAABAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 200);
    CHECK(obj.result.order_id == 851545009715);
    REQUIRE(std::size(obj.rate_limits) == 3);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// dapi

TEST_CASE("dapi_failure", "[json_wsapi_order_place]") {
  auto message = R"({)"
                 R"("id":"hYQeAAoCuEHQB0UAAAABAAAAAAAAAAAAAAAA",)"
                 R"("status":400,)"
                 R"("error":{)"
                 R"("code":-2019,)"
                 R"("msg":"Margin is insufficient.")"
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
    CHECK(obj.id == "hYQeAAoCuEHQB0UAAAABAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 400);
    CHECK(obj.error.code == -2019);
    CHECK(obj.error.msg == "Margin is insufficient."sv);
    REQUIRE(std::size(obj.rate_limits) == 2);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("dapi_success", "[json_wsapi_order_place]") {
  auto message = R"({)"
                 R"("id":"iIQeAAoCuUHQB0UAAAABAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":{)"
                 R"("orderId":199338059036,)"
                 R"("symbol":"BTCUSD_PERP",)"
                 R"("pair":"BTCUSD",)"
                 R"("status":"NEW",)"
                 R"("clientOrderId":"KgQCuUHQB0UAAQAAAAAA",)"
                 R"("price":"32000",)"
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
                 R"("updateTime":1765338056122,)"
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
                 R"("count":3)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "iIQeAAoCuUHQB0UAAAABAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 200);
    CHECK(obj.result.order_id == 199338059036);
    REQUIRE(std::size(obj.rate_limits) == 2);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
