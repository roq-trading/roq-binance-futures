/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/error.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::Error;

TEST_CASE("simple", "[json_error]") {
  auto message = R"#({)#"
                 R"#("code":-4164,)#"
                 R"#("msg":"Order's notional must be no smaller than 5.0 (unless you choose reduce only)")#"
                 R"#(})#";
  auto helper = [](value_type const &obj) {
    CHECK(obj.code == -4164);
    CHECK(obj.msg == "Order's notional must be no smaller than 5.0 (unless you choose reduce only)"sv);
  };
  core::json::BufferStack buffers{65536, 2};
  value_type obj{message, buffers};
  helper(obj);
}
