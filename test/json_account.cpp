/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/binance_futures/json/account.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// note! positions have been truncated
TEST_CASE("json_account_simple_usd_m", "[json_account]") {
  auto message = R"({)"
                 R"("feeTier":0,)"
                 R"("canTrade":true,)"
                 R"("canDeposit":true,)"
                 R"("canWithdraw":true,)"
                 R"("updateTime":0,)"
                 R"("totalInitialMargin":"0.00000000",)"
                 R"("totalMaintMargin":"0.00000000",)"
                 R"("totalWalletBalance":"0.00000000",)"
                 R"("totalUnrealizedProfit":"0.00000000",)"
                 R"("totalMarginBalance":"0.00000000",)"
                 R"("totalPositionInitialMargin":"0.00000000",)"
                 R"("totalOpenOrderInitialMargin":"0.00000000",)"
                 R"("totalCrossWalletBalance":"0.00000000",)"
                 R"("totalCrossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("assets":[{)"
                 R"("asset":"BTC",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("asset":"BNB",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("asset":"ETH",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("asset":"USDT",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("asset":"BUSD",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(})"
                 R"(],)"
                 R"("positions":[{)"
                 R"("symbol":"RAYUSDT",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("entryPrice":"0.0",)"
                 R"("maxNotional":"25000",)"
                 R"("positionSide":"BOTH",)"
                 R"("positionAmt":"0.0",)"
                 R"("notional":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("symbol":"SUSHIUSDT",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("entryPrice":"0.0",)"
                 R"("maxNotional":"25000",)"
                 R"("positionSide":"BOTH",)"
                 R"("positionAmt":"0",)"
                 R"("notional":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("symbol":"CVCUSDT",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("entryPrice":"0.0",)"
                 R"("maxNotional":"25000",)"
                 R"("positionSide":"BOTH",)"
                 R"("positionAmt":"0",)"
                 R"("notional":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("symbol":"BTSUSDT",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("entryPrice":"0.0",)"
                 R"("maxNotional":"25000",)"
                 R"("positionSide":"BOTH",)"
                 R"("positionAmt":"0",)"
                 R"("notional":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0)"
                 R"(})"
                 R"(])"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  json::Account obj{message, buffer};
  CHECK(obj.fee_tier == 0);
  CHECK(obj.can_trade == true);
  CHECK(obj.can_deposit == true);
  CHECK(obj.can_withdraw == true);
  CHECK(obj.update_time == 0ms);
  CHECK(obj.total_initial_margin == 0.0_a);
  CHECK(obj.total_maint_margin == 0.0_a);
  CHECK(obj.total_wallet_balance == 0.0_a);
  CHECK(obj.total_unrealized_profit == 0.0_a);
  CHECK(obj.total_margin_balance == 0.0_a);
  CHECK(obj.total_position_initial_margin == 0.0_a);
  CHECK(obj.total_open_order_initial_margin == 0.0_a);
  CHECK(obj.total_cross_wallet_balance == 0.0_a);
  CHECK(obj.total_cross_un_pnl == 0.0_a);
  CHECK(obj.available_balance == 0.0_a);
  CHECK(obj.max_withdraw_amount == 0.0_a);
  auto &assets = obj.assets;
  REQUIRE(std::size(assets) == 5);
  auto &a0 = assets[0];
  CHECK(a0.asset == "BTC"sv);
  CHECK(a0.wallet_balance == 0.0_a);
  CHECK(a0.unrealized_profit == 0.0_a);
  CHECK(a0.margin_balance == 0.0_a);
  CHECK(a0.maint_margin == 0.0_a);
  CHECK(a0.initial_margin == 0.0_a);
  CHECK(a0.position_initial_margin == 0.0_a);
  CHECK(a0.open_order_initial_margin == 0.0_a);
  CHECK(a0.max_withdraw_amount == 0.0_a);
  CHECK(a0.cross_wallet_balance == 0.0_a);
  CHECK(a0.cross_un_pnl == 0.0_a);
  CHECK(a0.available_balance == 0.0_a);
  CHECK(a0.margin_available == true);
  CHECK(a0.update_time == 0ms);
  // XXX TODO a1
  // XXX TODO a2
  // XXX TODO a3
  // XXX TODO a4
  auto &positions = obj.positions;
  REQUIRE(std::size(positions) == 4);
  auto &p0 = positions[0];
  CHECK(p0.symbol == "RAYUSDT"sv);
  CHECK(p0.initial_margin == 0.0_a);
  CHECK(p0.maint_margin == 0.0_a);
  CHECK(p0.unrealized_profit == 0.0_a);
  CHECK(p0.position_initial_margin == 0.0_a);
  CHECK(p0.open_order_initial_margin == 0.0_a);
  CHECK(p0.leverage == 20.0_a);
  CHECK(p0.isolated == false);
  CHECK(p0.entry_price == 0.0_a);
  CHECK(p0.max_notional == 25000.0_a);
  CHECK(p0.position_side == "BOTH"sv);
  CHECK(p0.position_amt == 0.0_a);
  CHECK(p0.notional == 0.0_a);
  CHECK(p0.isolated_wallet == 0.0_a);
  CHECK(p0.update_time == 0ms);
  // XXX TODO p1
  // XXX TODO p2
  // XXX TODO p3
}

// note! positions have been truncated
TEST_CASE("json_account_simple_coin_m", "[json_account]") {
  auto message = R"({)"
                 R"("feeTier":0,)"
                 R"("canTrade":true,)"
                 R"("canDeposit":true,)"
                 R"("canWithdraw":true,)"
                 R"("updateTime":0,)"
                 R"("assets":[{)"
                 R"("asset":"BTC",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000")"
                 R"(},{)"
                 R"("asset":"ADA",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000")"
                 R"(})"
                 R"(],)"
                 R"("positions":[{)"
                 R"("symbol":"ADAUSD_211231",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("positionSide":"BOTH",)"
                 R"("entryPrice":"0.00000000",)"
                 R"("maxQty":"250000",)"
                 R"("notionalValue":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0,)"
                 R"("positionAmt":"0")"
                 R"(},{)"
                 R"("symbol":"EOSUSD_PERP",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("positionSide":"BOTH",)"
                 R"("entryPrice":"0.00000000",)"
                 R"("maxQty":"30000",)"
                 R"("notionalValue":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0,)"
                 R"("positionAmt":"0")"
                 R"(})"
                 R"(])"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] json::Account obj{message, buffer};
}

TEST_CASE("json_account_simple_usd_m_new", "[json_account]") {
  auto message = R"({)"
                 R"("feeTier":0,)"
                 R"("canTrade":true,)"
                 R"("canDeposit":true,)"
                 R"("canWithdraw":true,)"
                 R"("updateTime":0,)"
                 R"("totalInitialMargin":"0.00000000",)"
                 R"("totalMaintMargin":"0.00000000",)"
                 R"("totalWalletBalance":"21.18338431",)"
                 R"("totalUnrealizedProfit":"0.00000000",)"
                 R"("totalMarginBalance":"21.18338431",)"
                 R"("totalPositionInitialMargin":"0.00000000",)"
                 R"("totalOpenOrderInitialMargin":"0.00000000",)"
                 R"("totalCrossWalletBalance":"21.18338431",)"
                 R"("totalCrossUnPnl":"0.00000000",)"
                 R"("availableBalance":"21.18338431",)"
                 R"("maxWithdrawAmount":"21.18338431",)"
                 R"("assets":[{)"
                 R"("asset":"DOT",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("asset":"BTC",)"
                 R"("walletBalance":"0.00000000",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("marginBalance":"0.00000000",)"
                 R"("maintMargin":"0.00000000",)"
                 R"("initialMargin":"0.00000000",)"
                 R"("positionInitialMargin":"0.00000000",)"
                 R"("openOrderInitialMargin":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(}],)"
                 R"("positions":[{)"
                 R"("symbol":"RAYUSDT",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("entryPrice":"0.0",)"
                 R"("maxNotional":"25000",)"
                 R"("positionSide":"BOTH",)"
                 R"("positionAmt":"0.0",)"
                 R"("notional":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0,)"
                 R"("bidNotional":"0",)"
                 R"("askNotional":"0")"
                 R"(},{)"
                 R"("symbol":"SUSHIUSDT",)"
                 R"("initialMargin":"0",)"
                 R"("maintMargin":"0",)"
                 R"("unrealizedProfit":"0.00000000",)"
                 R"("positionInitialMargin":"0",)"
                 R"("openOrderInitialMargin":"0",)"
                 R"("leverage":"20",)"
                 R"("isolated":false,)"
                 R"("entryPrice":"0.0",)"
                 R"("maxNotional":"150000",)"
                 R"("positionSide":"BOTH",)"
                 R"("positionAmt":"0",)"
                 R"("notional":"0",)"
                 R"("isolatedWallet":"0",)"
                 R"("updateTime":0,)"
                 R"("bidNotional":"0",)"
                 R"("askNotional":"0")"
                 R"(})"
                 R"(])"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] json::Account obj{message, buffer};
}
