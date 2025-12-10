/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "wsapi_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::WSAPIOrderCancel;

// fapi

TEST_CASE("fapi_failure", "[json_wsapi_order_cancel]") {
  auto message = R"({)"
                 R"("id":"hoQeAAsCoos3B0UAAAACAAAAAAAAAAAAAAAA",)"
                 R"("status":400,)"
                 R"("error":{)"
                 R"("code":-2011,)"
                 R"("msg":"Unknown order sent.")"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":2400,)"
                 R"("count":19)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "hoQeAAsCoos3B0UAAAACAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 400);
    CHECK(obj.error.code == -2011);
    CHECK(obj.error.msg == "Unknown order sent."sv);
    REQUIRE(std::size(obj.rate_limits) == 1);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("fapi_success", "[json_wsapi_order_cancel]") {
  auto message = R"({)"
                 R"("id":"jIQeAAsCpIs3B0UAAAADAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":{)"
                 R"("orderId":851545109401,)"
                 R"("symbol":"BTCUSDT",)"
                 R"("status":"CANCELED",)"
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
                 R"("updateTime":1765337031434)"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":2400,)"
                 R"("count":22)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "jIQeAAsCpIs3B0UAAAADAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 200);
    CHECK(obj.result.order_id == 851545109401);
    REQUIRE(std::size(obj.rate_limits) == 1);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// dapi

TEST_CASE("dapi_failure", "[json_wsapi_order_cancel]") {
  auto message = R"({)"
                 R"("id":"hoQeAAsCuEHQB0UAAAACAAAAAAAAAAAAAAAA",)"
                 R"("status":400,)"
                 R"("error":{)"
                 R"("code":-2011,)"
                 R"("msg":"Unknown order sent.")"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":2400,)"
                 R"("count":13)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "hoQeAAsCuEHQB0UAAAACAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 400);
    CHECK(obj.error.code == -2011);
    CHECK(obj.error.msg == "Unknown order sent."sv);
    REQUIRE(std::size(obj.rate_limits) == 1);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("dapi_success", "[json_wsapi_order_cancel]") {
  auto message = R"({)"
                 R"("id":"ioQeAAsCuUHQB0UAAAADAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":{)"
                 R"("orderId":199338059036,)"
                 R"("symbol":"BTCUSD_PERP",)"
                 R"("pair":"BTCUSD",)"
                 R"("status":"CANCELED",)"
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
                 R"("updateTime":1765338062775,)"
                 R"("priceMatch":"NONE")"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":2400,)"
                 R"("count":1)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "ioQeAAsCuUHQB0UAAAADAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 200);
    CHECK(obj.result.order_id == 199338059036);
    REQUIRE(std::size(obj.rate_limits) == 1);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
