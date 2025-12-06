/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::MarginCall;

TEST_CASE("online_example", "[json_margin_call]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::MARGIN_CALL);
    CHECK(obj.event_time == 1587727187525ms);
    CHECK(obj.cross_wallet_balance == 3.16812045_a);
    auto &positions = obj.positions;
    REQUIRE(std::size(positions) == 1);
    auto &position_0 = positions[0];
    CHECK(position_0.symbol == "ETHUSDT"sv);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
