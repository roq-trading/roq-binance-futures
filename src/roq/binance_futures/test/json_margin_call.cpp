/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/binance_futures/json/margin_call.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_margin_call_online_example", "[json_margin_call]") {
  auto message = R"({)"
                 R"("e":"MARGIN_CALL",)"
                 R"("E":1587727187525,)"
                 R"("cw":"3.16812045",)"
                 R"("p":[{)"
                 R"("s":"ETHUSDT",)"
                 R"("ps":"LONG",)"
                 R"("pa":"1.327",)"
                 R"("mt":"CROSSED",)"
                 R"("iw":"0",)"
                 R"("mp":"187.17127",)"
                 R"("up":"-1.166074",)"
                 R"("mm":"1.614445")"
                 R"(})"
                 R"(])"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::MarginCall::create(message, buffer);
  CHECK(obj.event_type == json::EventType::MARGIN_CALL);
  CHECK(obj.event_time == 1587727187525ms);
  CHECK(obj.cross_wallet_balance == 3.16812045_a);
  auto &positions = obj.positions;
  REQUIRE(std::size(positions) == 1);
  auto &p0 = positions[0];
  CHECK(p0.symbol == "ETHUSDT"sv);
}
