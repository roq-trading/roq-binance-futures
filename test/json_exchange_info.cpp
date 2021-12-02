/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/exchange_info.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

// note: symbols heavily truncated
TEST(json_exchange_info, simple) {
  auto message =
      R"({)"
      R"("timezone":"UTC",)"
      R"("serverTime":1634122324532,)"
      R"("futuresType":"U_MARGINED",)"
      R"("rateLimits":[{)"
      R"("rateLimitType":"REQUEST_WEIGHT", "interval":"MINUTE", "intervalNum":1, "limit":2400)"
      R"(},{)"
      R"("rateLimitType":"ORDERS","interval":"MINUTE","intervalNum":1,"limit":1200)"
      R"(},{)"
      R"("rateLimitType":"ORDERS","interval":"SECOND","intervalNum":10,"limit":300)"
      R"(})"
      R"(],)"
      R"("exchangeFilters":[],)"
      R"("assets":[{)"
      R"("asset":"USDT","marginAvailable":true,"autoAssetExchange":"-10000")"
      R"(},{)"
      R"("asset":"BTC","marginAvailable":true,"autoAssetExchange":"-0.00100000")"
      R"(},{)"
      R"("asset":"BNB","marginAvailable":true,"autoAssetExchange":"-10")"
      R"(},{)"
      R"("asset":"ETH","marginAvailable":true,"autoAssetExchange":"-5")"
      R"(},{)"
      R"("asset":"BUSD","marginAvailable":true,"autoAssetExchange":"-10000")"
      R"(})"
      R"(],)"
      R"("symbols":[{)"
      R"("symbol":"BTCUSDT",)"
      R"("pair":"BTCUSDT",)"
      R"("contractType":"PERPETUAL",)"
      R"("deliveryDate":4133404800000,)"
      R"("onboardDate":1569398400000,)"
      R"("status":"TRADING",)"
      R"("maintMarginPercent":"2.5000",)"
      R"("requiredMarginPercent":"5.0000",)"
      R"("baseAsset":"BTC",)"
      R"("quoteAsset":"USDT",)"
      R"("marginAsset":"USDT",)"
      R"("pricePrecision":2,)"
      R"("quantityPrecision":3,)"
      R"("baseAssetPrecision":8,)"
      R"("quotePrecision":8,)"
      R"("underlyingType":"COIN",)"
      R"("underlyingSubType":[],)"
      R"("settlePlan":0,)"
      R"("triggerProtect":"0.0500",)"
      R"("liquidationFee":"0.015000",)"
      R"("marketTakeBound":"0.05",)"
      R"("filters":[{)"
      R"("minPrice":"556.72","maxPrice":"4529764","filterType":"PRICE_FILTER","tickSize":"0.01")"
      R"(},{)"
      R"("stepSize":"0.001","filterType":"LOT_SIZE","maxQty":"1000","minQty":"0.001")"
      R"(},{"stepSize":"0.001","filterType":"MARKET_LOT_SIZE","maxQty":"300","minQty":"0.001")"
      R"(},{)"
      R"("limit":200,"filterType":"MAX_NUM_ORDERS")"
      R"(},{)"
      R"("limit":10,"filterType":"MAX_NUM_ALGO_ORDERS")"
      R"(},{)"
      R"("notional":"5","filterType":"MIN_NOTIONAL")"
      R"(},{)"
      R"("multiplierDown":"0.9500","multiplierUp":"1.0500","multiplierDecimal":"4","filterType":"PERCENT_PRICE")"
      R"(})"
      R"(],)"
      R"("orderTypes":["LIMIT","MARKET","STOP","STOP_MARKET","TAKE_PROFIT","TAKE_PROFIT_MARKET","TRAILING_STOP_MARKET"],)"
      R"("timeInForce":["GTC","IOC","FOK","GTX"])"
      R"(},{)"
      R"("symbol":"ETHUSDT",)"
      R"("pair":"ETHUSDT",)"
      R"("contractType":"PERPETUAL",)"
      R"("deliveryDate":4133404800000,)"
      R"("onboardDate":1569398400000,)"
      R"("status":"TRADING",)"
      R"("maintMarginPercent":"2.5000",)"
      R"("requiredMarginPercent":"5.0000",)"
      R"("baseAsset":"ETH",)"
      R"("quoteAsset":"USDT",)"
      R"("marginAsset":"USDT",)"
      R"("pricePrecision":2,)"
      R"("quantityPrecision":3,)"
      R"("baseAssetPrecision":8,)"
      R"("quotePrecision":8,)"
      R"("underlyingType":"COIN",)"
      R"("underlyingSubType":[],)"
      R"("settlePlan":0,)"
      R"("triggerProtect":"0.0500",)"
      R"("liquidationFee":"0.010000",)"
      R"("marketTakeBound":"0.05",)"
      R"("filters":[{)"
      R"("minPrice":"39.86","maxPrice":"306177","filterType":"PRICE_FILTER","tickSize":"0.01")"
      R"(},{)"
      R"("stepSize":"0.001","filterType":"LOT_SIZE","maxQty":"10000","minQty":"0.001")"
      R"(},{)"
      R"("stepSize":"0.001","filterType":"MARKET_LOT_SIZE","maxQty":"2000","minQty":"0.001")"
      R"(},{)"
      R"("limit":200,"filterType":"MAX_NUM_ORDERS")"
      R"(},{)"
      R"("limit":10,"filterType":"MAX_NUM_ALGO_ORDERS")"
      R"(},{)"
      R"("notional":"5","filterType":"MIN_NOTIONAL")"
      R"(},{)"
      R"("multiplierDown":"0.9500","multiplierUp":"1.0500","multiplierDecimal":"4","filterType":"PERCENT_PRICE")"
      R"(})"
      R"(],)"
      R"("orderTypes":["LIMIT","MARKET","STOP","STOP_MARKET","TAKE_PROFIT","TAKE_PROFIT_MARKET","TRAILING_STOP_MARKET"],)"
      R"("timeInForce":["GTC","IOC","FOK","GTX"])"
      R"(})"
      R"(])"
      R"(})";
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::ExchangeInfo>(message, buffer);
  EXPECT_EQ(obj.timezone, "UTC"sv);
  EXPECT_EQ(obj.server_time, 1634122324532ms);
  EXPECT_EQ(obj.futures_type, "U_MARGINED"sv);
  // not parsed: rate_limits
  // not parsed: exchange_filters
  auto &assets = obj.assets;
  ASSERT_EQ(std::size(assets), 5);
  auto &a0 = assets[0];
  EXPECT_EQ(a0.asset, "USDT"sv);
  EXPECT_EQ(a0.margin_available, true);
  EXPECT_DOUBLE_EQ(a0.auto_asset_exchange, -10000.0);
  auto &a1 = assets[1];
  EXPECT_EQ(a1.asset, "BTC"sv);
  EXPECT_EQ(a1.margin_available, true);
  EXPECT_DOUBLE_EQ(a1.auto_asset_exchange, -0.001);
  auto &a2 = assets[2];
  EXPECT_EQ(a2.asset, "BNB"sv);
  EXPECT_EQ(a2.margin_available, true);
  EXPECT_DOUBLE_EQ(a2.auto_asset_exchange, -10.0);
  auto &a3 = assets[3];
  EXPECT_EQ(a3.asset, "ETH"sv);
  EXPECT_EQ(a3.margin_available, true);
  EXPECT_DOUBLE_EQ(a3.auto_asset_exchange, -5.0);
  auto &a4 = assets[4];
  EXPECT_EQ(a4.asset, "BUSD"sv);
  EXPECT_EQ(a4.margin_available, true);
  EXPECT_DOUBLE_EQ(a4.auto_asset_exchange, -10000.0);
  auto &symbols = obj.symbols;
  ASSERT_EQ(std::size(symbols), 2);
  auto &s0 = symbols[0];
  EXPECT_EQ(s0.symbol, "BTCUSDT"sv);
  EXPECT_EQ(s0.pair, "BTCUSDT"sv);
  EXPECT_EQ(s0.contract_type, json::ContractType::PERPETUAL);
  EXPECT_EQ(s0.delivery_date, 4133404800000ms);
  EXPECT_EQ(s0.onboard_date, 1569398400000ms);
  EXPECT_EQ(s0.status, json::SymbolStatus::TRADING);
  EXPECT_DOUBLE_EQ(s0.maint_margin_percent, 2.5);
  EXPECT_DOUBLE_EQ(s0.required_margin_percent, 5.0);
  EXPECT_EQ(s0.base_asset, "BTC"sv);
  EXPECT_EQ(s0.quote_asset, "USDT"sv);
  EXPECT_EQ(s0.margin_asset, "USDT"sv);
  EXPECT_EQ(s0.price_precision, 2);
  EXPECT_EQ(s0.quantity_precision, 3);
  EXPECT_EQ(s0.base_asset_precision, 8);
  EXPECT_EQ(s0.quote_precision, 8);
  EXPECT_EQ(s0.underlying_type, "COIN"sv);
  // not parsed: underlying_sub_type
  EXPECT_EQ(s0.settle_plan, 0);
  EXPECT_DOUBLE_EQ(s0.trigger_protect, 0.05);
  EXPECT_DOUBLE_EQ(s0.liquidation_fee, 0.015);
  EXPECT_DOUBLE_EQ(s0.market_take_bound, 0.05);
  // not parsed: filters
  // not parsed: order_type
  // not parsed: time_in_force
  auto &s1 = symbols[1];
  EXPECT_EQ(s1.symbol, "ETHUSDT"sv);
  EXPECT_EQ(s1.pair, "ETHUSDT"sv);
  EXPECT_EQ(s1.contract_type, json::ContractType::PERPETUAL);
  EXPECT_EQ(s1.delivery_date, 4133404800000ms);
  EXPECT_EQ(s1.onboard_date, 1569398400000ms);
  EXPECT_EQ(s1.status, json::SymbolStatus::TRADING);
  EXPECT_DOUBLE_EQ(s1.maint_margin_percent, 2.5);
  EXPECT_DOUBLE_EQ(s1.required_margin_percent, 5.0);
  EXPECT_EQ(s1.base_asset, "ETH"sv);
  EXPECT_EQ(s1.quote_asset, "USDT"sv);
  EXPECT_EQ(s1.margin_asset, "USDT"sv);
  EXPECT_EQ(s1.price_precision, 2);
  EXPECT_EQ(s1.quantity_precision, 3);
  EXPECT_EQ(s1.base_asset_precision, 8);
  EXPECT_EQ(s1.quote_precision, 8);
  EXPECT_EQ(s1.underlying_type, "COIN"sv);
  // not parsed: underlying_sub_type
  EXPECT_EQ(s1.settle_plan, 0);
  EXPECT_DOUBLE_EQ(s1.trigger_protect, 0.05);
  EXPECT_DOUBLE_EQ(s1.liquidation_fee, 0.01);
  EXPECT_DOUBLE_EQ(s1.market_take_bound, 0.05);
  // not parsed: filters
  // not parsed: order_type
  // not parsed: time_in_force
}
