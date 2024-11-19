/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/binance_futures/json/account_update.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_account_update_papi_order", "[json_account_update_papi]") {
  auto message = R"({)"
                 R"("e":"ACCOUNT_UPDATE",)"
                 R"("T":1708690960633,)"
                 R"("E":1708690960639,)"
                 R"("fs":"UM",)"
                 R"("a":{)"
                 R"("B":[{)"
                 R"("a":"USDT",)"
                 R"("wb":"-0.05106850",)"
                 R"("cw":"-0.05106850",)"
                 R"("bc":"0")"
                 R"(})"
                 R"(],)"
                 R"("P":[{)"
                 R"("s":"BTCUSDT",)"
                 R"("pa":"0.005",)"
                 R"("ep":"51068.50000000",)"
                 R"("cr":"0",)"
                 R"("up":"-0.00050000",)"
                 R"("ps":"BOTH",)"
                 R"("bep":51078.7137)"
                 R"(})"
                 R"(],)"
                 R"("m":"ORDER")"
                 R"(})"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  json::AccountUpdate obj{message, buffer};
  CHECK(obj.event_type == json::EventType::ACCOUNT_UPDATE);
  CHECK(obj.transaction_time == 1708690960633ms);
  CHECK(obj.event_time == 1708690960639ms);
  CHECK(obj.fs == "UM"sv);
  auto &data = obj.data;
  auto &balances = data.balances;
  CHECK(std::size(balances) == 1);
  auto &b0 = balances[0];
  CHECK(b0.asset == "USDT"sv);
  CHECK(b0.wallet_balance == -0.05106850_a);
  CHECK(b0.cross_wallet_balance == -0.05106850_a);
  CHECK(b0.balance_change == 0.0_a);
  auto &positions = data.positions;
  CHECK(std::size(positions) == 1);
  auto &p0 = positions[0];
  CHECK(p0.symbol == "BTCUSDT"sv);
  CHECK(p0.position_amount == 0.005_a);
  CHECK(p0.entry_price == 51068.50000000_a);
  CHECK(p0.accumulated_realized == 0.0_a);
  CHECK(p0.unrealized_pnl == -0.0005_a);
  CHECK(p0.position_side == json::PositionSide::BOTH);
  CHECK(data.event_reason == json::EventReason::ORDER);
}

TEST_CASE("json_account_update_papi_order_cm", "[json_account_update_papi]") {
  auto message = R"({)"
                 R"("e":"ACCOUNT_UPDATE",)"
                 R"("T":1708696507923,)"
                 R"("E":1708696507932,)"
                 R"("i":"SgmYXqmYsRFzoCoC",)"
                 R"("fs":"CM",)"
                 R"("a":{)"
                 R"("B":[{)"
                 R"("a":"BTC",)"
                 R"("wb":"-0.00000039",)"
                 R"("cw":"-0.00000039",)"
                 R"("bc":"0")"
                 R"(})"
                 R"(],)"
                 R"("P":[{)"
                 R"("s":"BTCUSD_PERP",)"
                 R"("pa":"1",)"
                 R"("ep":"51080.10000066",)"
                 R"("cr":"0",)"
                 R"("up":"-0.00000009",)"
                 R"("ps":"BOTH",)"
                 R"("bep":51090.31602036)"
                 R"(})"
                 R"(],)"
                 R"("m":"ORDER")"
                 R"(})"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  json::AccountUpdate obj{message, buffer};
  CHECK(obj.event_type == json::EventType::ACCOUNT_UPDATE);
  CHECK(obj.transaction_time == 1708696507923ms);
  CHECK(obj.event_time == 1708696507932ms);
  CHECK(obj.fs == "CM"sv);
  auto &data = obj.data;
  auto &balances = data.balances;
  CHECK(std::size(balances) == 1);
  auto &b0 = balances[0];
  CHECK(b0.asset == "BTC"sv);
  CHECK(b0.wallet_balance == -0.00000039_a);
  CHECK(b0.cross_wallet_balance == -0.00000039_a);
  CHECK(b0.balance_change == 0.0_a);
  auto &positions = data.positions;
  CHECK(std::size(positions) == 1);
  auto &p0 = positions[0];
  CHECK(p0.symbol == "BTCUSD_PERP"sv);
  CHECK(p0.position_amount == 1.0_a);
  CHECK(p0.entry_price == 51080.10000066_a);
  CHECK(p0.accumulated_realized == 0.0_a);
  CHECK(p0.unrealized_pnl == -0.00000009_a);
  CHECK(p0.position_side == json::PositionSide::BOTH);
  CHECK(data.event_reason == json::EventReason::ORDER);
}
