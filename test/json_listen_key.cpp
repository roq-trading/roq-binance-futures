/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/listen_key.h"

using namespace roq;
using namespace roq::binance_futures;

TEST(json_listen_key, simple) {
  auto message = R"({)"
                 R"("listenKey":"JRNjbBr3bWns94f71afrzHXwsfZgVczUWTD02hBmO03OFHQh08YsuwmkpewsrdNY")"
                 R"(})";
  auto obj = core::json::Parser::create<json::ListenKey>(message);
  EXPECT_EQ(obj.listen_key, "JRNjbBr3bWns94f71afrzHXwsfZgVczUWTD02hBmO03OFHQh08YsuwmkpewsrdNY"_sv);
}
