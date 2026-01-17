/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "market_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::AggTrade;

TEST_CASE("coin_m", "[json_agg_trade]") {
  auto message = R"({)"
                 R"("e":"aggTrade",)"
                 R"("E":1768644497528,)"
                 R"("a":3057600382,)"
                 R"("s":"BTCUSDT",)"
                 R"("p":"95208.60",)"
                 R"("q":"0.020",)"
                 R"("nq":"0.020",)"
                 R"("f":7111099183,)"
                 R"("l":7111099183,)"
                 R"("T":1768644497374,)"
                 R"("m":true)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::AGG_TRADE);
    CHECK(obj.event_time == 1768644497528ms);
  };
  MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
