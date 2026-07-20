/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "market_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::AssetIndexUpdate;

TEST_CASE("coin_m", "[json_asset_index_update]") {
  auto message = R"({)"
                 R"("e":"assetIndexUpdate",)"
                 R"("E":1784556027000,)"
                 R"("s":"USDTUSD",)"
                 R"("i":"0.99891344",)"
                 R"("b":"0.00010000",)"
                 R"("a":"0.00010000",)"
                 R"("B":"0.99881355",)"
                 R"("A":"0.99901333",)"
                 R"("q":"0.00010000",)"
                 R"("g":"0.00010000",)"
                 R"("Q":"0.99881355",)"
                 R"("G":"0.99901333")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == protocol::json::EventType::ASSET_INDEX_UPDATE);
    CHECK(obj.event_time == 1784556027000ms);
  };
  MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
