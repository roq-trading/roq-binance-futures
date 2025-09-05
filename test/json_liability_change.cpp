/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/liability_change.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_liability_change]") {
  auto message = R"({)"
                 R"("e":"liabilityChange",)"
                 R"("E":1573200697110,)"
                 R"("a":"BTC",)"
                 R"("t":"BORROW",)"
                 R"("T":1352286576452864727,)"
                 R"("p":"1.03453430",)"
                 R"("i":"0",)"
                 R"("l":"1.03476851")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::LiabilityChange obj{message, buffer};
  CHECK(obj.event_type == json::EventType::LIABILITY_CHANGE);
}
