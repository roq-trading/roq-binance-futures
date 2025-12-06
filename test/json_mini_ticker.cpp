/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "market_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::MiniTicker;

TEST_CASE("coin_m", "[json_mini_ticker]") {
  auto message = R"({)"
                 R"("e":"24hrMiniTicker",)"
                 R"("E":1640248670092,)"
                 R"("s":"BTCUSD_220325",)"
                 R"("ps":"BTCUSD",)"
                 R"("c":"49461.1",)"
                 R"("o":"50769.1",)"
                 R"("h":"50903.5",)"
                 R"("l":"49230.6",)"
                 R"("v":"1913789",)"
                 R"("q":"3833.39491534")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::_24HR_MINI_TICKER);
    CHECK(obj.event_time == 1640248670092ms);
    CHECK(obj.symbol == "BTCUSD_220325"sv);
    CHECK(obj.pair == "BTCUSD"sv);
    CHECK(obj.close_price == 49461.1_a);
    CHECK(obj.open_price == 50769.1_a);
    CHECK(obj.high_price == 50903.5_a);
    CHECK(obj.low_price == 49230.6_a);
    CHECK(obj.total_traded_base_asset_volume == 1913789.0_a);
    CHECK(obj.total_traded_quote_asset_volume == 3833.39491534_a);
  };
  MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
