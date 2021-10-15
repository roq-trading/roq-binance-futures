/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/error.h"

using namespace roq;
using namespace roq::binance_futures;

TEST(json_error, simple) {
  auto message =
      R"#({)#"
      R"#("code":-4164,)#"
      R"#("msg":"Order's notional must be no smaller than 5.0 (unless you choose reduce only)")#"
      R"#(})#";
  auto obj = core::json::Parser::create<json::Error>(message);
  EXPECT_EQ(obj.code, -4164);
  EXPECT_EQ(
      obj.msg, "Order's notional must be no smaller than 5.0 (unless you choose reduce only)"_sv);
}
