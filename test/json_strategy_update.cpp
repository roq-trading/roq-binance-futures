/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::StrategyUpdate;

TEST_CASE("simple", "[json_strategy_update]") {
  auto message = R"({)"
                 R"("e":"STRATEGY_UPDATE",)"
                 R"("T":1669261797627,)"
                 R"("E":1669261797628,)"
                 R"("su":{)"
                 R"("si":176054594,)"
                 R"("st":"GRID",)"
                 R"("ss":"NEW",)"
                 R"("s":"BTCUSDT",)"
                 R"("ut":1669261797627,)"
                 R"("c":8007)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::STRATEGY_UPDATE);
    CHECK(obj.transaction_time == 1669261797627ms);
    CHECK(obj.event_time == 1669261797628ms);
    auto &data = obj.data;
    CHECK(data.id == 176054594);
    CHECK(data.type == "GRID"sv);
    CHECK(data.status == "NEW"sv);
    CHECK(data.symbol == "BTCUSDT"sv);
    CHECK(data.update_time == 1669261797627ms);
    CHECK(data.opcode == 8007);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
