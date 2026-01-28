/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "market_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Trade2;

TEST_CASE("coin_m", "[json_trade]") {
  auto message = R"({)"
                 R"("e":"trade",)"
                 R"("E":1769590256675,)"
                 R"("T":1769590256674,)"
                 R"("s":"BTCUSD_PERP",)"
                 R"("t":1074178546,)"
                 R"("p":"88904.9",)"
                 R"("q":"1",)"
                 R"("X":"MARKET",)"
                 R"("m":true)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::TRADE);
    CHECK(obj.event_time == 1769590256675ms);
  };
  MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
