/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/outbound_account_position.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_outbound_account_position]") {
  // note! from docs
  auto message = R"({)"
                 R"("e":"outboundAccountPosition",)"
                 R"("E":1564034571105,)"
                 R"("u":1564034571073,)"
                 R"("B":[{)"
                 R"("a":"ETH",)"
                 R"("f":"10000.000000",)"
                 R"("l":"0.000000")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 2};
  json::OutboundAccountPosition obj{message, buffer};
  CHECK(obj.event_type == json::EventType::OUTBOUND_ACCOUNT_POSITION);
  REQUIRE(std::size(obj.balances) == 1);
}
