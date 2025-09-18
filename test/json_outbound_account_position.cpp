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
  auto message = R"({)"
                 R"("e":"outboundAccountPosition",)"
                 R"("E":1758021107347,)"
                 R"("u":1758021107347,)"
                 R"("U":2239237878416,)"
                 R"("B":[{)"
                 R"("a":"USDT",)"
                 R"("f":"110096.34321000",)"
                 R"("l":"0.00000000")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 2};
  json::OutboundAccountPosition obj{message, buffer};
  CHECK(obj.event_type == json::EventType::OUTBOUND_ACCOUNT_POSITION);
  REQUIRE(std::size(obj.balances) == 1);
}
