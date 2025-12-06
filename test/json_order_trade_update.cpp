/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "user_stream_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::OrderTradeUpdate;

TEST_CASE("new", "[json_order_trade_update]") {
  auto message = R"({)"
                 R"("e":"ORDER_TRADE_UPDATE",)"
                 R"("T":1634553049579,)"
                 R"("E":1634553049581,)"
                 R"("o":{)"
                 R"("s":"XRPUSDT",)"
                 R"("c":"mwAC6QMAAQAA1UL7ndIW",)"
                 R"("S":"BUY",)"
                 R"("o":"LIMIT",)"
                 R"("f":"GTC",)"
                 R"("q":"5",)"
                 R"("p":"1.0741",)"
                 R"("ap":"0",)"
                 R"("sp":"0",)"
                 R"("x":"NEW",)"
                 R"("X":"NEW",)"
                 R"("i":17761651527,)"
                 R"("l":"0",)"
                 R"("z":"0",)"
                 R"("L":"0",)"
                 R"("T":1634553049579,)"
                 R"("t":0,)"
                 R"("b":"5.37050",)"
                 R"("a":"0",)"
                 R"("m":false,)"
                 R"("R":false,)"
                 R"("wt":"CONTRACT_PRICE",)"
                 R"("ot":"LIMIT",)"
                 R"("ps":"BOTH",)"
                 R"("cp":false,)"
                 R"("rp":"0",)"
                 R"("pP":false,)"
                 R"("si":0,)"
                 R"("ss":0)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ORDER_TRADE_UPDATE);
    CHECK(obj.transaction_time == 1634553049579ms);
    CHECK(obj.event_time == 1634553049581ms);
    auto &execution_report = obj.execution_report;
    CHECK(execution_report.symbol == "XRPUSDT"sv);
    CHECK(execution_report.client_order_id == "mwAC6QMAAQAA1UL7ndIW"sv);
    CHECK(execution_report.side == json::Side::BUY);
    CHECK(execution_report.order_type == json::OrderType::LIMIT);
    CHECK(execution_report.time_in_force == json::TimeInForce::GTC);
    CHECK(execution_report.original_quantity == 5.0_a);
    CHECK(execution_report.original_price == 1.0741_a);
    CHECK(execution_report.average_price == 0.0_a);
    CHECK(execution_report.stop_price == 0.0_a);
    CHECK(execution_report.execution_type == json::ExecutionType::NEW);
    CHECK(execution_report.order_status == json::OrderStatus::NEW);
    CHECK(execution_report.order_id == 17761651527);
    CHECK(execution_report.last_filled_quantity == 0.0_a);
    CHECK(execution_report.order_filled_accumulated_quantity == 0.0_a);
    CHECK(execution_report.last_filled_price == 0.0_a);
    CHECK(execution_report.order_trade_time == 1634553049579ms);
    CHECK(execution_report.trade_id == 0);
    CHECK(execution_report.bids_notional == 5.3705_a);
    CHECK(execution_report.asks_notional == 0.0_a);
    CHECK(execution_report.is_trade_maker == false);
    CHECK(execution_report.is_reduce_only == false);
    CHECK(execution_report.stop_price_working_type == json::WorkingType::CONTRACT_PRICE);
    CHECK(execution_report.original_order_type == json::OrderType::LIMIT);
    CHECK(execution_report.position_side == json::PositionSide::BOTH);
    CHECK(execution_report.if_close_all == false);
    CHECK(execution_report.realized_profit == 0.0_a);
    // unknown from here
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("canceled", "[json_order_trade_update]") {
  auto message = R"({)"
                 R"("e":"ORDER_TRADE_UPDATE",)"
                 R"("T":1634561771964,)"
                 R"("E":1634561771970,)"
                 R"("o":{)"
                 R"("s":"XRPUSDT",)"
                 R"("c":"KQAC6QMAAQAASLsSpNQW",)"
                 R"("S":"BUY",)"
                 R"("o":"LIMIT",)"
                 R"("f":"GTC",)"
                 R"("q":"5",)"
                 R"("p":"1.0667",)"
                 R"("ap":"0",)"
                 R"("sp":"0",)"
                 R"("x":"CANCELED",)"
                 R"("X":"CANCELED",)"
                 R"("i":17763431911,)"
                 R"("l":"0",)"
                 R"("z":"0",)"
                 R"("L":"0",)"
                 R"("T":1634561771964,)"
                 R"("t":0,)"
                 R"("b":"0",)"
                 R"("a":"0",)"
                 R"("m":false,)"
                 R"("R":false,)"
                 R"("wt":"CONTRACT_PRICE",)"
                 R"("ot":"LIMIT",)"
                 R"("ps":"BOTH",)"
                 R"("cp":false,)"
                 R"("rp":"0",)"
                 R"("pP":false,)"
                 R"("si":0,)"
                 R"("ss":0)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ORDER_TRADE_UPDATE);
    CHECK(obj.transaction_time == 1634561771964ms);
    CHECK(obj.event_time == 1634561771970ms);
    auto &execution_report = obj.execution_report;
    CHECK(execution_report.symbol == "XRPUSDT"sv);
    CHECK(execution_report.client_order_id == "KQAC6QMAAQAASLsSpNQW"sv);
    CHECK(execution_report.side == json::Side::BUY);
    CHECK(execution_report.order_type == json::OrderType::LIMIT);
    CHECK(execution_report.time_in_force == json::TimeInForce::GTC);
    CHECK(execution_report.original_quantity == 5.0_a);
    CHECK(execution_report.original_price == 1.0667_a);
    CHECK(execution_report.average_price == 0.0_a);
    CHECK(execution_report.stop_price == 0.0_a);
    CHECK(execution_report.execution_type == json::ExecutionType::CANCELED);
    CHECK(execution_report.order_status == json::OrderStatus::CANCELED);
    CHECK(execution_report.order_id == 17763431911);
    CHECK(execution_report.last_filled_quantity == 0.0_a);
    CHECK(execution_report.order_filled_accumulated_quantity == 0.0_a);
    CHECK(execution_report.last_filled_price == 0.0_a);
    CHECK(execution_report.order_trade_time == 1634561771964ms);
    CHECK(execution_report.trade_id == 0);
    CHECK(execution_report.bids_notional == 0.0_a);
    CHECK(execution_report.asks_notional == 0.0_a);
    CHECK(execution_report.is_trade_maker == false);
    CHECK(execution_report.is_reduce_only == false);
    CHECK(execution_report.stop_price_working_type == json::WorkingType::CONTRACT_PRICE);
    CHECK(execution_report.original_order_type == json::OrderType::LIMIT);
    CHECK(execution_report.position_side == json::PositionSide::BOTH);
    CHECK(execution_report.if_close_all == false);
    CHECK(execution_report.realized_profit == 0.0_a);
    // unknown from here
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("filled_maker", "[json_order_trade_update]") {
  auto message = R"({)"
                 R"("e":"ORDER_TRADE_UPDATE",)"
                 R"("T":1634812374563,)"
                 R"("E":1634812374567,)"
                 R"("o":{)"
                 R"("s":"XRPUSDT",)"
                 R"("c":"-gAC6QMAAQAAYIZV_g4X",)"
                 R"("S":"SELL",)"
                 R"("o":"LIMIT",)"
                 R"("f":"GTC",)"
                 R"("q":"5",)"
                 R"("p":"1.1583",)"
                 R"("ap":"1.15830",)"
                 R"("sp":"0",)"
                 R"("x":"TRADE",)"
                 R"("X":"FILLED",)"
                 R"("i":17803846427,)"
                 R"("l":"5",)"
                 R"("z":"5",)"
                 R"("L":"1.1583",)"
                 R"("n":"0.00115829",)"
                 R"("N":"USDT",)"
                 R"("T":1634812374563,)"
                 R"("t":673747843,)"
                 R"("b":"0",)"
                 R"("a":"0",)"
                 R"("m":true,)"
                 R"("R":false,)"
                 R"("wt":"CONTRACT_PRICE",)"
                 R"("ot":"LIMIT",)"
                 R"("ps":"BOTH",)"
                 R"("cp":false,)"
                 R"("rp":"0",)"
                 R"("pP":false,)"
                 R"("si":0,)"
                 R"("ss":0)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ORDER_TRADE_UPDATE);
    CHECK(obj.transaction_time == 1634812374563ms);
    CHECK(obj.event_time == 1634812374567ms);
    auto &execution_report = obj.execution_report;
    CHECK(execution_report.symbol == "XRPUSDT"sv);
    CHECK(execution_report.client_order_id == "-gAC6QMAAQAAYIZV_g4X"sv);
    CHECK(execution_report.side == json::Side::SELL);
    CHECK(execution_report.order_type == json::OrderType::LIMIT);
    CHECK(execution_report.time_in_force == json::TimeInForce::GTC);
    CHECK(execution_report.original_quantity == 5.0_a);
    CHECK(execution_report.original_price == 1.1583_a);
    CHECK(execution_report.average_price == 1.1583_a);
    CHECK(execution_report.stop_price == 0.0_a);
    CHECK(execution_report.execution_type == json::ExecutionType::TRADE);
    CHECK(execution_report.order_status == json::OrderStatus::FILLED);
    CHECK(execution_report.order_id == 17803846427);
    CHECK(execution_report.last_filled_quantity == 5.0_a);
    CHECK(execution_report.order_filled_accumulated_quantity == 5.0_a);
    CHECK(execution_report.last_filled_price == 1.1583_a);
    CHECK(execution_report.commission == 0.00115829_a);
    CHECK(execution_report.commission_asset == "USDT"sv);
    CHECK(execution_report.order_trade_time == 1634812374563ms);
    CHECK(execution_report.trade_id == 673747843);
    CHECK(execution_report.bids_notional == 0.0_a);
    CHECK(execution_report.asks_notional == 0.0_a);
    CHECK(execution_report.is_trade_maker == true);  // note!
    CHECK(execution_report.is_reduce_only == false);
    CHECK(execution_report.stop_price_working_type == json::WorkingType::CONTRACT_PRICE);
    CHECK(execution_report.original_order_type == json::OrderType::LIMIT);
    CHECK(execution_report.position_side == json::PositionSide::BOTH);
    CHECK(execution_report.if_close_all == false);
    CHECK(execution_report.realized_profit == 0.0_a);
    // unknown from here
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("filled_taker", "[json_order_trade_update]") {
  auto message = R"({)"
                 R"("e":"ORDER_TRADE_UPDATE",)"
                 R"("T":1634814520988,)"
                 R"("E":1634814520998,)"
                 R"("o":{)"
                 R"( "s":"XRPUSDT",)"
                 R"("c":"mwAC6QMAAQAAH6jkfg8X",)"
                 R"("S":"BUY",)"
                 R"("o":"LIMIT",)"
                 R"("f":"GTC",)"
                 R"("q":"10",)"
                 R"("p":"1.1559",)"
                 R"("ap":"1.15590",)"
                 R"("sp":"0",)"
                 R"("x":"TRADE",)"
                 R"("X":"FILLED",)"
                 R"("i":17804214275,)"
                 R"("l":"10",)"
                 R"("z":"10",)"
                 R"("L":"1.1559",)"
                 R"("n":"0.00462360",)"
                 R"("N":"USDT",)"
                 R"("T":1634814520988,)"
                 R"("t":673771916,)"
                 R"("b":"0",)"
                 R"("a":"0",)"
                 R"("m":false,)"
                 R"("R":false,)"
                 R"("wt":"CONTRACT_PRICE",)"
                 R"("ot":"LIMIT",)"
                 R"("ps":"BOTH",)"
                 R"("cp":false,)"
                 R"("rp":"0.00950000",)"
                 R"("pP":false,)"
                 R"("si":0,)"
                 R"("ss":0)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event_type == json::EventType::ORDER_TRADE_UPDATE);
    CHECK(obj.transaction_time == 1634814520988ms);
    CHECK(obj.event_time == 1634814520998ms);
    auto &execution_report = obj.execution_report;
    CHECK(execution_report.symbol == "XRPUSDT"sv);
    CHECK(execution_report.client_order_id == "mwAC6QMAAQAAH6jkfg8X"sv);
    CHECK(execution_report.side == json::Side::BUY);
    CHECK(execution_report.order_type == json::OrderType::LIMIT);
    CHECK(execution_report.time_in_force == json::TimeInForce::GTC);
    CHECK(execution_report.original_quantity == 10.0_a);
    CHECK(execution_report.original_price == 1.1559_a);
    CHECK(execution_report.average_price == 1.1559_a);
    CHECK(execution_report.stop_price == 0.0_a);
    CHECK(execution_report.execution_type == json::ExecutionType::TRADE);
    CHECK(execution_report.order_status == json::OrderStatus::FILLED);
    CHECK(execution_report.order_id == 17804214275);
    CHECK(execution_report.last_filled_quantity == 10.0_a);
    CHECK(execution_report.order_filled_accumulated_quantity == 10.0_a);
    CHECK(execution_report.last_filled_price == 1.1559_a);
    CHECK(execution_report.commission == 0.00462360_a);
    CHECK(execution_report.commission_asset == "USDT"sv);
    CHECK(execution_report.order_trade_time == 1634814520988ms);
    CHECK(execution_report.trade_id == 673771916);
    CHECK(execution_report.bids_notional == 0.0_a);
    CHECK(execution_report.asks_notional == 0.0_a);
    CHECK(execution_report.is_trade_maker == false);  // note!
    CHECK(execution_report.is_reduce_only == false);
    CHECK(execution_report.stop_price_working_type == json::WorkingType::CONTRACT_PRICE);
    CHECK(execution_report.original_order_type == json::OrderType::LIMIT);
    CHECK(execution_report.position_side == json::PositionSide::BOTH);
    CHECK(execution_report.if_close_all == false);
    CHECK(execution_report.realized_profit == 0.0095_a);
    // unknown from here
  };
  UserStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
