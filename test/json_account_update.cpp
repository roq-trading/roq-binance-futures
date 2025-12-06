/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::AccountUpdate;

TEST_CASE("order", "[json_account_update]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ACCOUNT_UPDATE);
    CHECK(obj.transaction_time == 1634811213699ms);
    CHECK(obj.event_time == 1634811213707ms);
    auto &data = obj.data;
    auto &balances = data.balances;
    CHECK(std::size(balances) == 1);
    auto &balance_0 = balances[0];
    CHECK(balance_0.asset == "USDT"sv);
    CHECK(balance_0.wallet_balance == 21.12168460_a);
    CHECK(balance_0.cross_wallet_balance == 21.12168460_a);
    CHECK(balance_0.balance_change == 0.0_a);
    auto &positions = data.positions;
    CHECK(std::size(positions) == 1);
    auto &position_0 = positions[0];
    CHECK(position_0.symbol == "XRPUSDT"sv);
    CHECK(position_0.position_amount == -5.0_a);
    CHECK(position_0.entry_price == 1.15540_a);
    CHECK(position_0.accumulated_realized == 0.0_a);
    CHECK(position_0.unrealized_pnl == 0.001_a);
    CHECK(position_0.margin_type == json::MarginType::CROSS);
    CHECK(position_0.isolated_wallet == 0.0_a);
    CHECK(position_0.position_side == json::PositionSide::BOTH);
    CHECK(position_0.unknown_1 == "USDT"sv);
    CHECK(data.event_reason == json::EventReason::ORDER);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("withdraw", "[json_account_update]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ACCOUNT_UPDATE);
    CHECK(obj.transaction_time == 1634827654378ms);
    CHECK(obj.event_time == 1634827654387ms);
    auto &data = obj.data;
    auto &balances = data.balances;
    CHECK(std::size(balances) == 1);
    auto &balance_0 = balances[0];
    CHECK(balance_0.asset == "USDT"sv);
    CHECK(balance_0.wallet_balance == 0.0_a);
    CHECK(balance_0.cross_wallet_balance == 0.0_a);
    CHECK(balance_0.balance_change == -21.14364261_a);
    auto &positions = data.positions;
    CHECK(std::size(positions) == 0);
    CHECK(data.event_reason == json::EventReason::WITHDRAW);
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
