/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/countdown_cancel_all_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using value_type = json::CountdownCancelAllAck;

TEST_CASE("simple", "[json_countdown_cancel_all_ack]") {
  auto message = R"({)"
                 R"("symbol":"XRPUSDT",)"
                 R"("countdownTime":"30000")"
                 R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.symbol == "XRPUSDT"sv);
    CHECK(obj.countdown_time == 30000);
  };
  core::json::BufferStack buffers{65536, 2};
  value_type obj{message, buffers};
  helper(obj);
}
