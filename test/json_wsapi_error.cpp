/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "wsapi_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::WSAPIError;

TEST_CASE("simple", "[json_wsapi_error]") {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("status":400,)"
                 R"("error":{)"
                 R"("code":-1000,)"
                 R"("msg":"Invalid JSON request.")"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(std::empty(obj.id));
    CHECK(obj.status == 400);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
