/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::BalanceUpdate;

TEST_CASE("simple", "[json_balance_update]") {
  auto message = R"({)"
                 R"("e":"balanceUpdate",)"
                 R"("E":1758021107347,)"
                 R"("a":"USDT",)"
                 R"("d":"100000.00000000",)"
                 R"("U":2239237878416,)"
                 R"("T":1758021107347)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::BALANCE_UPDATE);
    CHECK(obj.event_time == 1758021107347ms);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
