/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/order_trade_update.h"

using namespace roq;
using namespace roq::binance_futures;

TEST(json_order_trade_update, simple_new) {
  auto message = R"({)"
                 R"("e":"ORDER_TRADE_UPDATE",)"
                 R"("T":1634553049579,)"
                 R"("E":1634553049581,)"
                 R"("o":{)"
                 R"("s":"XRPUSDT",)"
                 R"("c":"mwAC6QMAAQAA1UL7ndIW",)"
                 R"("S":"BUY",)"
                 R"("o":"LIMIT",)"
                 R"("f":"GTC",)"
                 R"("q":"5",)"
                 R"("p":"1.0741",)"
                 R"("ap":"0",)"
                 R"("sp":"0",)"
                 R"("x":"NEW",)"
                 R"("X":"NEW",)"
                 R"("i":17761651527,)"
                 R"("l":"0",)"
                 R"("z":"0",)"
                 R"("L":"0",)"
                 R"("T":1634553049579,)"
                 R"("t":0,)"
                 R"("b":"5.37050",)"
                 R"("a":"0",)"
                 R"("m":false,)"
                 R"("R":false,)"
                 R"("wt":"CONTRACT_PRICE",)"
                 R"("ot":"LIMIT",)"
                 R"("ps":"BOTH",)"
                 R"("cp":false,)"
                 R"("rp":"0",)"
                 R"("pP":false,)"
                 R"("si":0,)"
                 R"("ss":0)"
                 R"(})"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::OrderTradeUpdate>(message, buffer);
}

TEST(json_order_trade_update, simple_canceled) {
  auto message = R"({)"
                 R"("e":"ORDER_TRADE_UPDATE",)"
                 R"("T":1634561771964,)"
                 R"("E":1634561771970,)"
                 R"("o":{)"
                 R"("s":"XRPUSDT",)"
                 R"("c":"KQAC6QMAAQAASLsSpNQW",)"
                 R"("S":"BUY",)"
                 R"("o":"LIMIT",)"
                 R"("f":"GTC",)"
                 R"("q":"5",)"
                 R"("p":"1.0667",)"
                 R"("ap":"0",)"
                 R"("sp":"0",)"
                 R"("x":"CANCELED",)"
                 R"("X":"CANCELED",)"
                 R"("i":17763431911,)"
                 R"("l":"0",)"
                 R"("z":"0",)"
                 R"("L":"0",)"
                 R"("T":1634561771964,)"
                 R"("t":0,)"
                 R"("b":"0",)"
                 R"("a":"0",)"
                 R"("m":false,)"
                 R"("R":false,)"
                 R"("wt":"CONTRACT_PRICE",)"
                 R"("ot":"LIMIT",)"
                 R"("ps":"BOTH",)"
                 R"("cp":false,)"
                 R"("rp":"0",)"
                 R"("pP":false,)"
                 R"("si":0,)"
                 R"("ss":0)"
                 R"(})"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::OrderTradeUpdate>(message, buffer);
}
