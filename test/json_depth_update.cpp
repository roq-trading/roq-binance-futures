/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/depth_update.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_depth_update_simple_coin_m", "[json_depth_update]") {
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
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::DepthUpdate obj{message, buffer};
}
