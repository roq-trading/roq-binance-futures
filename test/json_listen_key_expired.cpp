/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::ListenKeyExpired;

TEST_CASE("simple", "[json_listen_key_expired]") {
  auto message = R"({)"
                 R"("e":"LISTEN_KEY_EXPIRED")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == protocol::json::EventType::LISTEN_KEY_EXPIRED);
    //
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
