/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/balance.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_balance_simple_usd_m", "[json_balance]") {
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
  REQUIRE(std::size(data) == 5);
  auto &a0 = data[0];
  CHECK(a0.account_alias == "mYAuTiXqXqsR"sv);
  CHECK(a0.asset == "BTC"sv);
  CHECK(a0.balance == 0.0_a);
  CHECK(a0.cross_wallet_balance == 0.0_a);
  CHECK(a0.cross_un_pnl == 0.0_a);
  CHECK(a0.available_balance == 0.0_a);
  CHECK(a0.max_withdraw_amount == 0.0_a);
  CHECK(a0.margin_available == true);
  CHECK(a0.update_time == 0ms);
  auto &a1 = data[1];
  CHECK(a1.account_alias == "mYAuTiXqXqsR"sv);
  CHECK(a1.asset == "BNB"sv);
  CHECK(a1.balance == 0.0_a);
  CHECK(a1.cross_wallet_balance == 0.0_a);
  CHECK(a1.cross_un_pnl == 0.0_a);
  CHECK(a1.available_balance == 0.0_a);
  CHECK(a1.max_withdraw_amount == 0.0_a);
  CHECK(a1.margin_available == true);
  CHECK(a1.update_time == 0ms);
  auto &a2 = data[2];
  CHECK(a2.account_alias == "mYAuTiXqXqsR"sv);
  CHECK(a2.asset == "ETH"sv);
  CHECK(a2.balance == 0.0_a);
  CHECK(a2.cross_wallet_balance == 0.0_a);
  CHECK(a2.cross_un_pnl == 0.0_a);
  CHECK(a2.available_balance == 0.0_a);
  CHECK(a2.max_withdraw_amount == 0.0_a);
  CHECK(a2.margin_available == true);
  CHECK(a2.update_time == 0ms);
  auto &a3 = data[3];
  CHECK(a3.account_alias == "mYAuTiXqXqsR"sv);
  CHECK(a3.asset == "USDT"sv);
  CHECK(a3.balance == 0.0_a);
  CHECK(a3.cross_wallet_balance == 0.0_a);
  CHECK(a3.cross_un_pnl == 0.0_a);
  CHECK(a3.available_balance == 0.0_a);
  CHECK(a3.max_withdraw_amount == 0.0_a);
  CHECK(a3.margin_available == true);
  CHECK(a3.update_time == 0ms);
  auto &a4 = data[4];
  CHECK(a4.account_alias == "mYAuTiXqXqsR"sv);
  CHECK(a4.asset == "BUSD"sv);
  CHECK(a4.balance == 0.0_a);
  CHECK(a4.cross_wallet_balance == 0.0_a);
  CHECK(a4.cross_un_pnl == 0.0_a);
  CHECK(a4.available_balance == 0.0_a);
  CHECK(a4.max_withdraw_amount == 0.0_a);
  CHECK(a4.margin_available == true);
  CHECK(a4.update_time == 0ms);
}

TEST_CASE("json_balance_simple_coinm", "[json_balance]") {
  auto message = R"([{)"
                 R"("accountAlias":"SgmYSgfWmYmY",)"
                 R"("asset":"BTC",)"
                 R"("balance":"0.00000000",)"
                 R"("withdrawAvailable":"0.00000000",)"
                 R"("updateTime":1640262685148,)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000")"
                 R"(},{)"
                 R"("accountAlias":"SgmYSgfWmYmY",)"
                 R"("asset":"ADA",)"
                 R"("balance":"0.00000000",)"
                 R"("withdrawAvailable":"0.00000000",)"
                 R"("updateTime":1640262685148,)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000")"
                 R"(},{)"
                 R"("accountAlias":"SgmYSgfWmYmY",)"
                 R"("asset":"LINK",)"
                 R"("balance":"0.00000000",)"
                 R"("withdrawAvailable":"0.00000000",)"
                 R"("updateTime":1640262685148,)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000")"
                 R"(})"
                 R"(])";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::Balance>(message, buffer);
  auto &data = obj.data;
  REQUIRE(std::size(data) == 3);
}
