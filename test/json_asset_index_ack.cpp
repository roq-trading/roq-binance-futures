/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/protocol/json/asset_index_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::AssetIndexAck;

// note: symbols heavily truncated
TEST_CASE("simple", "[json_asset_index_ack]") {
  auto message = R"([{)"
                 R"("symbol":"SOLUSD",)"
                 R"("time":1784571967000,)"
                 R"("index":"78.00217964",)"
                 R"("bidBuffer":"0.00000000",)"
                 R"("askBuffer":"0.00000000",)"
                 R"("bidRate":"78.00217964",)"
                 R"("askRate":"78.00217964",)"
                 R"("autoExchangeBidBuffer":"0.00000000",)"
                 R"("autoExchangeAskBuffer":"0.00000000",)"
                 R"("autoExchangeBidRate":"78.00217964",)"
                 R"("autoExchangeAskRate":"78.00217964")"
                 R"(},{)"
                 R"("symbol":"UNIUSD",)"
                 R"("time":1784571967000,)"
                 R"("index":"3.61187354",)"
                 R"("bidBuffer":"0.00000000",)"
                 R"("askBuffer":"0.00000000",)"
                 R"("bidRate":"3.61187354",)"
                 R"("askRate":"3.61187354",)"
                 R"("autoExchangeBidBuffer":"0.00000000",)"
                 R"("autoExchangeAskBuffer":"0.00000000",)"
                 R"("autoExchangeBidRate":"3.61187354",)"
                 R"("autoExchangeAskRate":"3.61187354")"
                 R"(},{)"
                 R"("symbol":"TRXUSD",)"
                 R"("time":1784571967000,)"
                 R"("index":"0.32658538",)"
                 R"("bidBuffer":"0.00000000",)"
                 R"("askBuffer":"0.00000000",)"
                 R"("bidRate":"0.32658538",)"
                 R"("askRate":"0.32658538",)"
                 R"("autoExchangeBidBuffer":"0.00000000",)"
                 R"("autoExchangeAskBuffer":"0.00000000",)"
                 R"("autoExchangeBidRate":"0.32658538",)"
                 R"("autoExchangeAskRate":"0.32658538")"
                 R"(})"
                 R"(])";
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 3);
    auto &obj_0 = obj.data[0];
    CHECK(obj_0.symbol == "SOLUSD"sv);
    CHECK(obj_0.time == 1784571967000ms);
    CHECK(obj_0.index == 78.00217964_a);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
