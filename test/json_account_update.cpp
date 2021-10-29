/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/account_update.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_account_update, order) {
  auto message = R"({)"
                 R"("e":"ACCOUNT_UPDATE",)"
                 R"("T":1634811213699,)"
                 R"("E":1634811213707,)"
                 R"("a":{)"
                 R"("B":[{)"
                 R"("a":"USDT",)"
                 R"("wb":"21.12168460",)"
                 R"("cw":"21.12168460",)"
                 R"("bc":"0")"
                 R"(})"
                 R"(],)"
                 R"("P":[{)"
                 R"("s":"XRPUSDT",)"
                 R"("pa":"-5",)"
                 R"("ep":"1.15540",)"
                 R"("cr":"0",)"
                 R"("up":"0.00100000",)"
                 R"("mt":"cross",)"
                 R"("iw":"0",)"
                 R"("ps":"BOTH",)"
                 R"("ma":"USDT")"
                 R"(})"
                 R"(],)"
                 R"("m":"ORDER")"
                 R"(})"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::AccountUpdate>(message, buffer);
  EXPECT_EQ(obj.event_type, json::EventType::ACCOUNT_UPDATE);
  EXPECT_EQ(obj.transaction_time, 1634811213699ms);
  EXPECT_EQ(obj.event_time, 1634811213707ms);
  auto &data = obj.data;
  auto &balances = data.balances;
  EXPECT_EQ(std::size(balances), 1);
  auto &b0 = balances[0];
  EXPECT_EQ(b0.asset, "USDT"sv);
  EXPECT_DOUBLE_EQ(b0.wallet_balance, 21.12168460);
  EXPECT_DOUBLE_EQ(b0.cross_wallet_balance, 21.12168460);
  EXPECT_DOUBLE_EQ(b0.balance_change, 0.0);
  auto &positions = data.positions;
  EXPECT_EQ(std::size(positions), 1);
  auto &p0 = positions[0];
  EXPECT_EQ(p0.symbol, "XRPUSDT"sv);
  EXPECT_DOUBLE_EQ(p0.position_amount, -5.0);
  EXPECT_DOUBLE_EQ(p0.entry_price, 1.15540);
  EXPECT_DOUBLE_EQ(p0.accumulated_realized, 0.0);
  EXPECT_DOUBLE_EQ(p0.unrealized_pnl, 0.001);
  EXPECT_EQ(p0.margin_type, "cross"sv);
  EXPECT_DOUBLE_EQ(p0.isolated_wallet, 0.0);
  EXPECT_EQ(p0.position_side, json::PositionSide::BOTH);
  EXPECT_EQ(p0.unknown_1, "USDT"sv);
  EXPECT_EQ(data.event_reason, json::EventReason::ORDER);
}

TEST(json_account_update, withdraw) {
  auto message = R"({)"
                 R"("e":"ACCOUNT_UPDATE",)"
                 R"("T":1634827654378,)"
                 R"("E":1634827654387,)"
                 R"("a":{)"
                 R"("B":[{)"
                 R"("a":"USDT",)"
                 R"("wb":"0",)"
                 R"("cw":"0",)"
                 R"("bc":"-21.14364261"}],)"
                 R"("P":[],)"
                 R"("m":"WITHDRAW")"
                 R"(})"
                 R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::AccountUpdate>(message, buffer);
  EXPECT_EQ(obj.event_type, json::EventType::ACCOUNT_UPDATE);
  EXPECT_EQ(obj.transaction_time, 1634827654378ms);
  EXPECT_EQ(obj.event_time, 1634827654387ms);
  auto &data = obj.data;
  auto &balances = data.balances;
  EXPECT_EQ(std::size(balances), 1);
  auto &b0 = balances[0];
  EXPECT_EQ(b0.asset, "USDT"sv);
  EXPECT_DOUBLE_EQ(b0.wallet_balance, 0.0);
  EXPECT_DOUBLE_EQ(b0.cross_wallet_balance, 0.0);
  EXPECT_DOUBLE_EQ(b0.balance_change, -21.14364261);
  auto &positions = data.positions;
  EXPECT_EQ(std::size(positions), 0);
  EXPECT_EQ(data.event_reason, json::EventReason::WITHDRAW);
}
