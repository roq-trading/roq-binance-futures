/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/listen_key_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::ListenKeyAck;

TEST_CASE("simple", "[json_listen_key_ack]") {
  auto message = R"({)"
                 R"("listenKey":"JRNjbBr3bWns94f71afrzHXwsfZgVczUWTD02hBmO03OFHQh08YsuwmkpewsrdNY")"
                 R"(})";
  auto helper = [&](value_type &obj) { CHECK(obj.listen_key == "JRNjbBr3bWns94f71afrzHXwsfZgVczUWTD02hBmO03OFHQh08YsuwmkpewsrdNY"sv); };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
