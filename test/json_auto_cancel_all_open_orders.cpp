/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/auto_cancel_all_open_orders.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

TEST_CASE("json_auto_cancel_all_open_orders_simple", "json_auto_cancel_all_open_orders") {
  auto message = R"({)"
                 R"("symbol":"XRPUSDT",)"
                 R"("countdownTime":"30000")"
                 R"(})";
  auto obj = core::json::Parser::create<json::AutoCancelAllOpenOrders>(message);
  CHECK(obj.symbol == "XRPUSDT"sv);
  CHECK(obj.countdown_time == 30000);
}
