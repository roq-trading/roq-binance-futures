/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/balance.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_balance, simple) {
  auto message = R"([{)"
                 R"("accountAlias":"mYAuTiXqXqsR",)"
                 R"("asset":"BTC",)"
                 R"("balance":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("accountAlias":"mYAuTiXqXqsR",)"
                 R"("asset":"BNB",)"
                 R"("balance":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("accountAlias":"mYAuTiXqXqsR",)"
                 R"("asset":"ETH",)"
                 R"("balance":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("accountAlias":"mYAuTiXqXqsR",)"
                 R"("asset":"USDT",)"
                 R"("balance":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("accountAlias":"mYAuTiXqXqsR",)"
                 R"("asset":"BUSD",)"
                 R"("balance":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(})"
                 R"(])";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::Balance>(message, buffer);
  auto &data = obj.data;
  ASSERT_EQ(std::size(data), 5);
  auto &a0 = data[0];
  EXPECT_EQ(a0.account_alias, "mYAuTiXqXqsR"sv);
  EXPECT_EQ(a0.asset, "BTC"sv);
  EXPECT_DOUBLE_EQ(a0.balance, 0.0);
  EXPECT_DOUBLE_EQ(a0.cross_wallet_balance, 0.0);
  EXPECT_DOUBLE_EQ(a0.cross_un_pnl, 0.0);
  EXPECT_DOUBLE_EQ(a0.available_balance, 0.0);
  EXPECT_DOUBLE_EQ(a0.max_withdraw_amount, 0.0);
  EXPECT_EQ(a0.margin_available, true);
  EXPECT_EQ(a0.update_time, 0ms);
  auto &a1 = data[1];
  EXPECT_EQ(a1.account_alias, "mYAuTiXqXqsR"sv);
  EXPECT_EQ(a1.asset, "BNB"sv);
  EXPECT_DOUBLE_EQ(a1.balance, 0.0);
  EXPECT_DOUBLE_EQ(a1.cross_wallet_balance, 0.0);
  EXPECT_DOUBLE_EQ(a1.cross_un_pnl, 0.0);
  EXPECT_DOUBLE_EQ(a1.available_balance, 0.0);
  EXPECT_DOUBLE_EQ(a1.max_withdraw_amount, 0.0);
  EXPECT_EQ(a1.margin_available, true);
  EXPECT_EQ(a1.update_time, 0ms);
  auto &a2 = data[2];
  EXPECT_EQ(a2.account_alias, "mYAuTiXqXqsR"sv);
  EXPECT_EQ(a2.asset, "ETH"sv);
  EXPECT_DOUBLE_EQ(a2.balance, 0.0);
  EXPECT_DOUBLE_EQ(a2.cross_wallet_balance, 0.0);
  EXPECT_DOUBLE_EQ(a2.cross_un_pnl, 0.0);
  EXPECT_DOUBLE_EQ(a2.available_balance, 0.0);
  EXPECT_DOUBLE_EQ(a2.max_withdraw_amount, 0.0);
  EXPECT_EQ(a2.margin_available, true);
  EXPECT_EQ(a2.update_time, 0ms);
  auto &a3 = data[3];
  EXPECT_EQ(a3.account_alias, "mYAuTiXqXqsR"sv);
  EXPECT_EQ(a3.asset, "USDT"sv);
  EXPECT_DOUBLE_EQ(a3.balance, 0.0);
  EXPECT_DOUBLE_EQ(a3.cross_wallet_balance, 0.0);
  EXPECT_DOUBLE_EQ(a3.cross_un_pnl, 0.0);
  EXPECT_DOUBLE_EQ(a3.available_balance, 0.0);
  EXPECT_DOUBLE_EQ(a3.max_withdraw_amount, 0.0);
  EXPECT_EQ(a3.margin_available, true);
  EXPECT_EQ(a3.update_time, 0ms);
  auto &a4 = data[4];
  EXPECT_EQ(a4.account_alias, "mYAuTiXqXqsR"sv);
  EXPECT_EQ(a4.asset, "BUSD"sv);
  EXPECT_DOUBLE_EQ(a4.balance, 0.0);
  EXPECT_DOUBLE_EQ(a4.cross_wallet_balance, 0.0);
  EXPECT_DOUBLE_EQ(a4.cross_un_pnl, 0.0);
  EXPECT_DOUBLE_EQ(a4.available_balance, 0.0);
  EXPECT_DOUBLE_EQ(a4.max_withdraw_amount, 0.0);
  EXPECT_EQ(a4.margin_available, true);
  EXPECT_EQ(a4.update_time, 0ms);
}
