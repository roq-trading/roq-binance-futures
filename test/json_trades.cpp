/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/trades.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("empty", "[json_trades]") {
  auto message = R"([])";
  std::vector<std::byte> buffer(8192);
  json::Trades obj{message, buffer};
  REQUIRE(std::size(obj.data) == 0);
}

TEST_CASE("fapi", "[json_trades]") {
  auto message = R"([{)"
                 R"("buyer": false,)"
                 R"("commission": "-0.07819010",)"
                 R"("commissionAsset": "USDT",)"
                 R"("id": 698759,)"
                 R"("maker": false,)"
                 R"("orderId": 25851813,)"
                 R"("price": "7819.01",)"
                 R"("qty": "0.002",)"
                 R"("quoteQty": "15.63802",)"
                 R"("realizedPnl": "-0.91539999",)"
                 R"("side": "SELL",)"
                 R"("positionSide": "SHORT",)"
                 R"("symbol": "BTCUSDT",)"
                 R"("time": 1569514978020)"
                 R"(})"
                 R"(])";
  std::vector<std::byte> buffer(8192);
  json::Trades obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
  auto &d0 = data[0];
  CHECK(d0.buyer == false);
  CHECK(d0.commission == Catch::Approx{-0.07819010});
  CHECK(d0.commission_asset == "USDT"sv);
  CHECK(d0.id == 698759);
  CHECK(d0.maker == false);
  CHECK(d0.order_id == 25851813);
  CHECK(d0.price == Catch::Approx{7819.01});
  CHECK(d0.qty == Catch::Approx{0.002});
  CHECK(d0.quote_qty == Catch::Approx{15.63802});
  CHECK(d0.realized_pnl == Catch::Approx{-0.91539999});
  CHECK(d0.side == json::Side::SELL);
  CHECK(d0.position_side == json::PositionSide::SHORT);
  CHECK(d0.symbol == "BTCUSDT"sv);
  CHECK(d0.time == 1569514978020ms);
}

TEST_CASE("dapi", "[json_trades]") {
  auto message = R"([{)"
                 R"("symbol": "BTCUSD_200626",)"
                 R"("id": 6,)"
                 R"("orderId": 28,)"
                 R"("pair": "BTCUSD",)"
                 R"("side": "SELL",)"
                 R"("price": "8800",)"
                 R"("qty": "1",)"
                 R"("realizedPnl": "0",)"
                 R"("marginAsset": "BTC",)"
                 R"("baseQty": "0.01136364",)"
                 R"("commission": "0.00000454",)"
                 R"("commissionAsset": "BTC",)"
                 R"("time": 1590743483586,)"
                 R"("positionSide": "BOTH",)"
                 R"("buyer": false,)"
                 R"("maker": false)"
                 R"(})"
                 R"(])";
  std::vector<std::byte> buffer(8192);
  json::Trades obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
  auto &d0 = data[0];
  CHECK(d0.id == 6);
  CHECK(d0.order_id == 28);
  CHECK(d0.pair == "BTCUSD"sv);
  CHECK(d0.side == json::Side::SELL);
  CHECK(d0.price == Catch::Approx{8800.});
  CHECK(d0.qty == Catch::Approx{1.0});
  CHECK(d0.realized_pnl == Catch::Approx{0.0});
  CHECK(d0.margin_asset == "BTC"sv);
  CHECK(d0.base_qty == Catch::Approx{0.01136364});
  CHECK(d0.commission == Catch::Approx{0.00000454});
  CHECK(d0.commission_asset == "BTC"sv);
  CHECK(d0.time == 1590743483586ms);
  CHECK(d0.position_side == json::PositionSide::BOTH);
  CHECK(d0.buyer == false);
  CHECK(d0.maker == false);
}
