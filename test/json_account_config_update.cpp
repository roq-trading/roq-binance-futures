/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::AccountConfigUpdate;

TEST_CASE("simple_1", "[json_account_config_update]") {
  auto message = R"({)"
                 R"("e":"ACCOUNT_CONFIG_UPDATE",)"
                 R"("E":1611646737479,)"
                 R"("T":1611646737476,)"
                 R"("ac":{)"
                 R"("s":"BTCUSDT",)"
                 R"("l":25)"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ACCOUNT_CONFIG_UPDATE);
    CHECK(obj.event_time == 1611646737479ms);
    CHECK(obj.transaction_time == 1611646737476ms);
    auto &data = obj.data1;
    CHECK(data.symbol == "BTCUSDT"sv);
    CHECK(data.leverage == 25.0_a);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("simple_2", "[json_account_config_update]") {
  auto message = R"({)"
                 R"("e":"ACCOUNT_CONFIG_UPDATE",)"
                 R"("E":1611646737479,)"
                 R"("T":1611646737476,)"
                 R"("ai":{)"
                 R"("j":true)"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ACCOUNT_CONFIG_UPDATE);
    CHECK(obj.event_time == 1611646737479ms);
    CHECK(obj.transaction_time == 1611646737476ms);
    auto &data = obj.data2;
    CHECK(data.multi_asset_mode == true);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
