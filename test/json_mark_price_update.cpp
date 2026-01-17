/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "market_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::MarkPriceUpdate;

TEST_CASE("coin_m", "[json_mark_price_update]") {
  auto message = R"({)"
                 R"("e":"markPriceUpdate",)"
                 R"("E":1768644498002,)"
                 R"("s":"BTCUSDT",)"
                 R"("p":"95209.55336957",)"
                 R"("P":"95252.48709172",)"
                 R"("i":"95252.46195652",)"
                 R"("r":"0.00005383",)"
                 R"("T":1768665600000)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::MARK_PRICE_UPDATE);
    CHECK(obj.event_time == 1768644498002ms);
  };
  MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
