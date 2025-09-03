/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"
#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/trades.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("empty", "[json_trades]") {
  auto message = R"([])";
  core::json::BufferStack buffer{8192, 1};
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
  core::json::BufferStack buffer{8192, 1};
  json::Trades obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
  auto &data_0 = data[0];
  CHECK(data_0.buyer == false);
  CHECK(data_0.commission == Catch::Approx{-0.07819010});
  CHECK(data_0.commission_asset == "USDT"sv);
  CHECK(data_0.id == 698759);
  CHECK(data_0.maker == false);
  CHECK(data_0.order_id == 25851813);
  CHECK(data_0.price == Catch::Approx{7819.01});
  CHECK(data_0.qty == Catch::Approx{0.002});
  CHECK(data_0.quote_qty == Catch::Approx{15.63802});
  CHECK(data_0.realized_pnl == Catch::Approx{-0.91539999});
  CHECK(data_0.side == json::Side::SELL);
  CHECK(data_0.position_side == json::PositionSide::SHORT);
  CHECK(data_0.symbol == "BTCUSDT"sv);
  CHECK(data_0.time == 1569514978020ms);
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
  core::json::BufferStack buffer{8192, 1};
  json::Trades obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
  auto &data_0 = data[0];
  CHECK(data_0.id == 6);
  CHECK(data_0.order_id == 28);
  CHECK(data_0.pair == "BTCUSD"sv);
  CHECK(data_0.side == json::Side::SELL);
  CHECK(data_0.price == Catch::Approx{8800.});
  CHECK(data_0.qty == Catch::Approx{1.0});
  CHECK(data_0.realized_pnl == Catch::Approx{0.0});
  CHECK(data_0.margin_asset == "BTC"sv);
  CHECK(data_0.base_qty == Catch::Approx{0.01136364});
  CHECK(data_0.commission == Catch::Approx{0.00000454});
  CHECK(data_0.commission_asset == "BTC"sv);
  CHECK(data_0.time == 1590743483586ms);
  CHECK(data_0.position_side == json::PositionSide::BOTH);
  CHECK(data_0.buyer == false);
  CHECK(data_0.maker == false);
}
