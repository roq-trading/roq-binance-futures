/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/account_balance_ack.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::AccountBalanceAck;

TEST_CASE("usd_m", "[json_account_balance_ack]") {
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
  auto helper = [&](value_type &obj) {
    auto &data = obj.data;
    REQUIRE(std::size(data) == 5);
    auto &data_0 = data[0];
    CHECK(data_0.account_alias == "mYAuTiXqXqsR"sv);
    CHECK(data_0.asset == "BTC"sv);
    CHECK(data_0.balance == 0.0_a);
    CHECK(data_0.cross_wallet_balance == 0.0_a);
    CHECK(data_0.cross_un_pnl == 0.0_a);
    CHECK(data_0.available_balance == 0.0_a);
    CHECK(data_0.max_withdraw_amount == 0.0_a);
    CHECK(data_0.margin_available == true);
    CHECK(data_0.update_time == 0ms);
    auto &data_1 = data[1];
    CHECK(data_1.account_alias == "mYAuTiXqXqsR"sv);
    CHECK(data_1.asset == "BNB"sv);
    CHECK(data_1.balance == 0.0_a);
    CHECK(data_1.cross_wallet_balance == 0.0_a);
    CHECK(data_1.cross_un_pnl == 0.0_a);
    CHECK(data_1.available_balance == 0.0_a);
    CHECK(data_1.max_withdraw_amount == 0.0_a);
    CHECK(data_1.margin_available == true);
    CHECK(data_1.update_time == 0ms);
    auto &data_2 = data[2];
    CHECK(data_2.account_alias == "mYAuTiXqXqsR"sv);
    CHECK(data_2.asset == "ETH"sv);
    CHECK(data_2.balance == 0.0_a);
    CHECK(data_2.cross_wallet_balance == 0.0_a);
    CHECK(data_2.cross_un_pnl == 0.0_a);
    CHECK(data_2.available_balance == 0.0_a);
    CHECK(data_2.max_withdraw_amount == 0.0_a);
    CHECK(data_2.margin_available == true);
    CHECK(data_2.update_time == 0ms);
    auto &data_3 = data[3];
    CHECK(data_3.account_alias == "mYAuTiXqXqsR"sv);
    CHECK(data_3.asset == "USDT"sv);
    CHECK(data_3.balance == 0.0_a);
    CHECK(data_3.cross_wallet_balance == 0.0_a);
    CHECK(data_3.cross_un_pnl == 0.0_a);
    CHECK(data_3.available_balance == 0.0_a);
    CHECK(data_3.max_withdraw_amount == 0.0_a);
    CHECK(data_3.margin_available == true);
    CHECK(data_3.update_time == 0ms);
    auto &data_4 = data[4];
    CHECK(data_4.account_alias == "mYAuTiXqXqsR"sv);
    CHECK(data_4.asset == "BUSD"sv);
    CHECK(data_4.balance == 0.0_a);
    CHECK(data_4.cross_wallet_balance == 0.0_a);
    CHECK(data_4.cross_un_pnl == 0.0_a);
    CHECK(data_4.available_balance == 0.0_a);
    CHECK(data_4.max_withdraw_amount == 0.0_a);
    CHECK(data_4.margin_available == true);
    CHECK(data_4.update_time == 0ms);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("coinm", "[json_balance]") {
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
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 3);
    CHECK(obj.data[0].account_alias == "SgmYSgfWmYmY"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
