/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "wsapi_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::WSAPIListenKey;

// fapi

TEST_CASE("dapi", "[json_wsapi_listen_key]") {
  auto message = R"({)"
                 R"("id":"goQeAAMAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":{)"
                 R"("listenKey":"UgOoXIu4JctC0MF8ujB7aerfRl564LHHP7rhwLrdeH1VrUeY03mKkslHWVbIN7h4")"
                 R"(},)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":2400,)"
                 R"("count":10)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "goQeAAMAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"sv);
    CHECK(obj.status == 200);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
