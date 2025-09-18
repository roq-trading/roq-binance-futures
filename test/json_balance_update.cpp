/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/balance_update.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_balance_update]") {
  auto message = R"({)"
                 R"("e":"balanceUpdate",)"
                 R"("E":1758021107347,)"
                 R"("a":"USDT",)"
                 R"("d":"100000.00000000",)"
                 R"("U":2239237878416,)"
                 R"("T":1758021107347)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::BalanceUpdate obj{message, buffer};
  CHECK(obj.event_type == json::EventType::BALANCE_UPDATE);
}
