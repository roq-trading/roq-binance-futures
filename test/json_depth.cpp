/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/depth.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// note! truncated
TEST_CASE("json_depth_simple_coin_m", "[json_depth]") {
  auto message = R"({)"
                 R"("lastUpdateId":300700442819,)"
                 R"("E":1640249388590,)"
                 R"("T":1640249388573,)"
                 R"("symbol":"BTCUSD_220325",)"
                 R"("pair":"BTCUSD",)"
                 R"("bids":[)"
                 R"(["49498.2","217"],)"
                 R"(["49493.9","84"])"
                 R"(],)"
                 R"("asks":[)"
                 R"(["49498.3","184"],)"
                 R"(["49498.8","7"])"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Depth obj{message, buffer};
}
