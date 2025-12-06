/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "market_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::DepthUpdate;

TEST_CASE("coin_m", "[json_depth_update]") {
  auto message = R"({)"
                 R"("e":"depthUpdate",)"
                 R"("E":1640247455980,)"
                 R"("T":1640247455971,)"
                 R"("s":"BTCUSD_220325",)"
                 R"("ps":"BTCUSD",)"
                 R"("U":300681317200,)"
                 R"("u":300681318938,)"
                 R"("pu":300681316807,)"
                 R"("b":[["49417.0","0"]],)"
                 R"("a":[)"
                 R"(["49425.9","332"],)"
                 R"(["49437.0","5"],)"
                 R"(["49438.9","30"],)"
                 R"(["49443.9","0"],)"
                 R"(["49445.2","0"],)"
                 R"(["49457.7","0"],)"
                 R"(["49460.1","7"],)"
                 R"(["49464.4","0"],)"
                 R"(["49508.1","61"],)"
                 R"(["49527.2","74"])"
                 R"(])"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::DEPTH_UPDATE);
    CHECK(obj.event_time == 1640247455980ms);
  };
  MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
