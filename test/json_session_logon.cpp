/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/wsapi_session_logon.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// === IMPLEMENTATION ===

TEST_CASE("success", "[json_session_logon]") {
  auto const message = R"({)"
                       R"("id":"gYQeAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",)"
                       R"("status":200,)"
                       R"("result":{)"
                       R"("apiKey":"n8k4HXQD1hyBzSAnCIPIsEbqJahgdQOMTe0lnc2JHjeGM8oRZuHxdBQbGn6XURHT",)"
                       R"("authorizedSince":1761362177206,)"
                       R"("connectedSince":1761362176969,)"
                       R"("returnRateLimits":true,)"
                       R"("serverTime":1761362177206},)"
                       R"("rateLimits":[{)"
                       R"("rateLimitType":"REQUEST_WEIGHT",)"
                       R"("interval":"MINUTE",)"
                       R"("intervalNum":1,)"
                       R"("limit":6000,)"
                       R"("count":9)"
                       R"(})"
                       R"(])"
                       R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::WSAPISessionLogon obj{message, buffers};
}

TEST_CASE("failure", "[json_session_logon]") {
  auto const message = R"({)"
                       R"("id":"gYQeAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",)"
                       R"("status":400,)"
                       R"("error":{)"
                       R"("code":-1022,)"
                       R"("msg":"Signature for this request is not valid."},)"
                       R"("rateLimits":[{)"
                       R"("rateLimitType":"REQUEST_WEIGHT",)"
                       R"("interval":"MINUTE",)"
                       R"("intervalNum":1,)"
                       R"("limit":6000,)"
                       R"("count":9)"
                       R"(})"
                       R"(])"
                       R"(})";
  core::json::BufferStack buffers{8192, 1};
  json::WSAPISessionLogon obj{message, buffers};
}
