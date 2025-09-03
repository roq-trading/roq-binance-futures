/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/grid_update.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_grid_update]") {
  auto message = R"({)"
                 R"("e":"GRID_UPDATE",)"
                 R"("T":1669262908216,)"
                 R"("E":1669262908218,)"
                 R"("gu":{)"
                 R"("si":176057039,)"
                 R"("st":"GRID",)"
                 R"("ss":"WORKING",)"
                 R"("s":"BTCUSDT",)"
                 R"("r":"-0.00300716",)"
                 R"("up":"16720",)"
                 R"("uq":"-0.001",)"
                 R"("uf":"-0.00300716",)"
                 R"("mp":"0.0",)"
                 R"("ut":1669262908197)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::GridUpdate obj{message, buffer};
  CHECK(obj.event_type == json::EventType::GRID_UPDATE);
  CHECK(obj.transaction_time == 1669262908216ms);
  CHECK(obj.event_time == 1669262908218ms);
  auto &data = obj.data;
  CHECK(data.id == 176057039);
  CHECK(data.type == "GRID"sv);
  CHECK(data.status == "WORKING"sv);
  CHECK(data.symbol == "BTCUSDT"sv);
  CHECK(data.realized_pnl == Catch::Approx{-0.00300716});
  CHECK(data.unmatched_average_price == Catch::Approx{16720});
  CHECK(data.unmatched_qty == Catch::Approx{-0.001});
  CHECK(data.unmatched_fee == Catch::Approx{-0.00300716});
  CHECK(data.matched_pnl == Catch::Approx{0.0});
  CHECK(data.update_time == 1669262908197ms);
}
