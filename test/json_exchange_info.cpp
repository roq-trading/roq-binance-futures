/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/exchange_info.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// note: symbols heavily truncated
TEST_CASE("json_exchange_info_simple_usd_m", "[json_exchange_info]") {
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
  std::vector<std::byte> buffer(65536);
  auto obj = json::ExchangeInfo::create(message, buffer);
  CHECK(obj.timezone == "UTC"sv);
  CHECK(obj.server_time == 1634122324532ms);
  CHECK(obj.futures_type == "U_MARGINED"sv);
  // not parsed: rate_limits
  // not parsed: exchange_filters
  auto &assets = obj.assets;
  REQUIRE(std::size(assets) == 5);
  auto &a0 = assets[0];
  CHECK(a0.asset == "USDT"sv);
  CHECK(a0.margin_available == true);
  CHECK(a0.auto_asset_exchange == -10000.0_a);
  auto &a1 = assets[1];
  CHECK(a1.asset == "BTC"sv);
  CHECK(a1.margin_available == true);
  CHECK(a1.auto_asset_exchange == -0.001_a);
  auto &a2 = assets[2];
  CHECK(a2.asset == "BNB"sv);
  CHECK(a2.margin_available == true);
  CHECK(a2.auto_asset_exchange == -10.0_a);
  auto &a3 = assets[3];
  CHECK(a3.asset == "ETH"sv);
  CHECK(a3.margin_available == true);
  CHECK(a3.auto_asset_exchange == -5.0_a);
  auto &a4 = assets[4];
  CHECK(a4.asset == "BUSD"sv);
  CHECK(a4.margin_available == true);
  CHECK(a4.auto_asset_exchange == -10000.0_a);
  auto &symbols = obj.symbols;
  REQUIRE(std::size(symbols) == 2);
  auto &s0 = symbols[0];
  CHECK(s0.symbol == "BTCUSDT"sv);
  CHECK(s0.pair == "BTCUSDT"sv);
  CHECK(s0.contract_type == json::ContractType::PERPETUAL);
  CHECK(s0.delivery_date == 4133404800000ms);
  CHECK(s0.onboard_date == 1569398400000ms);
  CHECK(s0.status == json::SymbolStatus::TRADING);
  CHECK(s0.maint_margin_percent == 2.5_a);
  CHECK(s0.required_margin_percent == 5.0_a);
  CHECK(s0.base_asset == "BTC"sv);
  CHECK(s0.quote_asset == "USDT"sv);
  CHECK(s0.margin_asset == "USDT"sv);
  CHECK(s0.price_precision == 2);
  CHECK(s0.quantity_precision == 3);
  CHECK(s0.base_asset_precision == 8);
  CHECK(s0.quote_precision == 8);
  CHECK(s0.underlying_type == "COIN"sv);
  // not parsed: underlying_sub_type
  CHECK(s0.settle_plan == 0);
  CHECK(s0.trigger_protect == 0.05_a);
  CHECK(s0.liquidation_fee == 0.015_a);
  CHECK(s0.market_take_bound == 0.05_a);
  // not parsed: filters
  // not parsed: order_type
  // not parsed: time_in_force
  auto &s1 = symbols[1];
  CHECK(s1.symbol == "ETHUSDT"sv);
  CHECK(s1.pair == "ETHUSDT"sv);
  CHECK(s1.contract_type == json::ContractType::PERPETUAL);
  CHECK(s1.delivery_date == 4133404800000ms);
  CHECK(s1.onboard_date == 1569398400000ms);
  CHECK(s1.status == json::SymbolStatus::TRADING);
  CHECK(s1.maint_margin_percent == 2.5_a);
  CHECK(s1.required_margin_percent == 5.0_a);
  CHECK(s1.base_asset == "ETH"sv);
  CHECK(s1.quote_asset == "USDT"sv);
  CHECK(s1.margin_asset == "USDT"sv);
  CHECK(s1.price_precision == 2);
  CHECK(s1.quantity_precision == 3);
  CHECK(s1.base_asset_precision == 8);
  CHECK(s1.quote_precision == 8);
  CHECK(s1.underlying_type == "COIN"sv);
  // not parsed: underlying_sub_type
  CHECK(s1.settle_plan == 0);
  CHECK(s1.trigger_protect == 0.05_a);
  CHECK(s1.liquidation_fee == 0.01_a);
  CHECK(s1.market_take_bound == 0.05_a);
  // not parsed: filters
  // not parsed: order_type
  // not parsed: time_in_force
}

// note: symbols heavily truncated
TEST_CASE("json_exchange_info_simple_coin_m", "[json_exchange_info]") {
  auto message =
      R"({)"
      R"("timezone":"UTC",)"
      R"("serverTime":1640230201523,)"
      R"("rateLimits":[{)"
      R"("rateLimitType":"REQUEST_WEIGHT","interval":"MINUTE","intervalNum":1,"limit":2400)"
      R"(},{)"
      R"("rateLimitType":"ORDERS","interval":"MINUTE","intervalNum":1,"limit":1200)"
      R"(})"
      R"(],)"
      R"("exchangeFilters":[],)"
      R"("symbols":[{)"
      R"("symbol":"BTCUSD_PERP",)"
      R"("pair":"BTCUSD",)"
      R"("contractType":"PERPETUAL",)"
      R"("deliveryDate":4133404800000,)"
      R"("onboardDate":1597042800000,)"
      R"("contractStatus":"TRADING",)"
      R"("contractSize":100,)"
      R"("marginAsset":"BTC",)"
      R"("maintMarginPercent":"2.5000",)"
      R"("requiredMarginPercent":"5.0000",)"
      R"("baseAsset":"BTC",)"
      R"("quoteAsset":"USD",)"
      R"("pricePrecision":1,)"
      R"("quantityPrecision":0,)"
      R"("baseAssetPrecision":8,)"
      R"("quotePrecision":8,)"
      R"("equalQtyPrecision":4,)"
      R"("maxMoveOrderLimit":10000,)"
      R"("triggerProtect":"0.0500",)"
      R"("underlyingType":"COIN",)"
      R"("underlyingSubType":[],)"
      R"("filters":[{)"
      R"("minPrice":"1000","maxPrice":"4520958","filterType":"PRICE_FILTER","tickSize":"0.1")"
      R"(},{)"
      R"("stepSize":"1","filterType":"LOT_SIZE","maxQty":"1000000","minQty":"1")"
      R"(},{)"
      R"("stepSize":"1","filterType":"MARKET_LOT_SIZE","maxQty":"60000","minQty":"1")"
      R"(},{)"
      R"("limit":200, "filterType":"MAX_NUM_ORDERS")"
      R"(},{)"
      R"("limit":100,"filterType":"MAX_NUM_ALGO_ORDERS")"
      R"(},{)"
      R"("multiplierDown":"0.9500","multiplierUp":"1.0500","multiplierDecimal":"4","filterType":"PERCENT_PRICE")"
      R"(})"
      R"(],)"
      R"("orderTypes":["LIMIT","MARKET","STOP","STOP_MARKET","TAKE_PROFIT","TAKE_PROFIT_MARKET","TRAILING_STOP_MARKET"],)"
      R"("timeInForce":["GTC","IOC","FOK","GTX"],)"
      R"("liquidationFee":"0.010000",)"
      R"("marketTakeBound":"0.05"},)"
      R"({)"
      R"("symbol":"BTCUSD_211231",)"
      R"("pair":"BTCUSD",)"
      R"("contractType":"CURRENT_QUARTER",)"
      R"("deliveryDate":1640937600000,)"
      R"("onboardDate":1624608000000,)"
      R"("contractStatus":"TRADING",)"
      R"("contractSize":100,)"
      R"("marginAsset":"BTC",)"
      R"("maintMarginPercent":"2.5000",)"
      R"("requiredMarginPercent":"5.0000",)"
      R"("baseAsset":"BTC",)"
      R"("quoteAsset":"USD",)"
      R"("pricePrecision":1,)"
      R"("quantityPrecision":0,)"
      R"("baseAssetPrecision":8,)"
      R"("quotePrecision":8,)"
      R"("equalQtyPrecision":4,)"
      R"("maxMoveOrderLimit":10000,)"
      R"("triggerProtect":"0.0500",)"
      R"("underlyingType":"COIN",)"
      R"("underlyingSubType":[],)"
      R"("filters":[{)"
      R"("minPrice":"1000","maxPrice":"4671848","filterType":"PRICE_FILTER","tickSize":"0.1")"
      R"(},{)"
      R"("stepSize":"1","filterType":"LOT_SIZE","maxQty":"1000000","minQty":"1")"
      R"(},{)"
      R"("stepSize":"1","filterType":"MARKET_LOT_SIZE","maxQty":"10000","minQty":"1")"
      R"(},{)"
      R"("limit":200,"filterType":"MAX_NUM_ORDERS")"
      R"(},{)"
      R"("limit":100,"filterType":"MAX_NUM_ALGO_ORDERS")"
      R"(},{)"
      R"("multiplierDown":"0.9500","multiplierUp":"1.0500","multiplierDecimal":"4","filterType":"PERCENT_PRICE")"
      R"(}],)"
      R"("orderTypes":["LIMIT","MARKET","STOP","STOP_MARKET","TAKE_PROFIT","TAKE_PROFIT_MARKET","TRAILING_STOP_MARKET"],)"
      R"("timeInForce":["GTC","IOC","FOK","GTX"],)"
      R"("liquidationFee":"0.010000",)"
      R"("marketTakeBound":"0.05")"
      R"(})"
      R"(])"
      R"(})";
  std::vector<std::byte> buffer(65536);
  [[maybe_unused]] auto obj = json::ExchangeInfo::create(message, buffer);
}
