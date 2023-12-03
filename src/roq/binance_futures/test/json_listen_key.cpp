/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/listen_key.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_listen_key_simple", "[json_listen_key]") {
  auto message = R"({)"
                 R"("listenKey":"JRNjbBr3bWns94f71afrzHXwsfZgVczUWTD02hBmO03OFHQh08YsuwmkpewsrdNY")"
                 R"(})";
  auto obj = core::json::Parser::create<json::ListenKey>(message);
  CHECK(obj.listen_key == "JRNjbBr3bWns94f71afrzHXwsfZgVczUWTD02hBmO03OFHQh08YsuwmkpewsrdNY"sv);
}
