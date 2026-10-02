/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "market_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::ForceOrder;

TEST_CASE("coin_m", "[json_force_order]") {
  auto message = R"({)"
                 R"("e":"forceOrder",)"
                 R"("E":1790931824221,)"
                 R"("o":{)"
                 R"("s":"USELESSUSDT",)"
                 R"("S":"SELL",)"
                 R"("o":"LIMIT",)"
                 R"("f":"IOC",)"
                 R"("q":"59",)"
                 R"("p":"0.2488500",)"
                 R"("ap":"0.2525500",)"
                 R"("X":"FILLED",)"
                 R"("l":"59",)"
                 R"("z":"59",)"
                 R"("T":1790931823217,)"
                 R"("ps":"USELESSUSDT",)"
                 R"("st":1)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == protocol::json::EventType::FORCE_ORDER);
    //
  };
  MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
