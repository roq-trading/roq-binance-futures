/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::LiabilityChange;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::LIABILITY_CHANGE);
    CHECK(obj.event_time == 1573200697110ms);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
