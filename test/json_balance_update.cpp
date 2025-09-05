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
                 R"("E":1573200697110,)"
                 R"("a":"BTC",)"
                 R"("d":"100.00000000",)"
                 R"("U":1027053479517,)"
                 R"("T":1573200697068)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::BalanceUpdate obj{message, buffer};
  CHECK(obj.event_type == json::EventType::BALANCE_UPDATE);
}
