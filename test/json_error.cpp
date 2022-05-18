/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/error.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_error_simple", "[json_error]") {
  auto message = R"#({)#"
                 R"#("code":-4164,)#"
                 R"#("msg":"Order's notional must be no smaller than 5.0 (unless you choose reduce only)")#"
                 R"#(})#";
  auto obj = core::json::Parser::create<json::Error>(message);
  CHECK(obj.code == -4164);
  CHECK(obj.msg == "Order's notional must be no smaller than 5.0 (unless you choose reduce only)"sv);
}
