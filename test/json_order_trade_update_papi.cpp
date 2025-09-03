/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/order_trade_update.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_trade_update_papi_simple_new", "[json_order_trade_update_papi]") {
  auto message = R"({)"
                 R"("e":"ORDER_TRADE_UPDATE",)"
                 R"("T":1708673882856,)"
                 R"("E":1708673882861,)"
                 R"("fs":"UM",)"
                 R"("o":{)"
                 R"("s":"BTCUSDT",)"
                 R"("c":"MQIC_J57-5QBAQAAAAAA",)"
                 R"("S":"BUY",)"
                 R"("o":"LIMIT",)"
                 R"("f":"GTC",)"
                 R"("q":"0.005",)"
                 R"("p":"32000",)"
                 R"("ap":"0",)"
                 R"("sp":"0",)"
                 R"("x":"NEW",)"
                 R"("X":"NEW",)"
                 R"("i":270905906796,)"
                 R"("l":"0",)"
                 R"("z":"0",)"
                 R"("L":"0",)"
                 R"("n":"0",)"
                 R"("N":"USDT",)"
                 R"("T":1708673882856,)"
                 R"("t":0,)"
                 R"("b":"160",)"
                 R"("a":"0",)"
                 R"("m":false,)"
                 R"("R":false,)"
                 R"("ps":"BOTH",)"
                 R"("rp":"0",)"
                 R"("V":"EXPIRE_NONE",)"
                 R"("pm":"PM_NONE",)"
                 R"("gtd":0)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::OrderTradeUpdate obj{message, buffer};
  CHECK(obj.event_type == json::EventType::ORDER_TRADE_UPDATE);
  CHECK(obj.transaction_time == 1708673882856ms);
  CHECK(obj.event_time == 1708673882861ms);
  CHECK(obj.fs == "UM"sv);
  auto &execution_report = obj.execution_report;
  CHECK(execution_report.symbol == "BTCUSDT"sv);
  CHECK(execution_report.client_order_id == "MQIC_J57-5QBAQAAAAAA"sv);
  CHECK(execution_report.side == json::Side::BUY);
  CHECK(execution_report.order_type == json::OrderType::LIMIT);
  CHECK(execution_report.time_in_force == json::TimeInForce::GTC);
  CHECK(execution_report.original_quantity == 0.005_a);
  CHECK(execution_report.original_price == 32000.0_a);
  CHECK(execution_report.average_price == 0.0_a);
  CHECK(execution_report.stop_price == 0.0_a);
  CHECK(execution_report.execution_type == json::ExecutionType::NEW);
  CHECK(execution_report.order_status == json::OrderStatus::NEW);
  CHECK(execution_report.order_id == 270905906796);
  CHECK(execution_report.last_filled_quantity == 0.0_a);
  CHECK(execution_report.order_filled_accumulated_quantity == 0.0_a);
  CHECK(execution_report.last_filled_price == 0.0_a);
  CHECK(execution_report.order_trade_time == 1708673882856ms);
  CHECK(execution_report.trade_id == 0);
  CHECK(execution_report.bids_notional == 160.0_a);
  CHECK(execution_report.asks_notional == 0.0_a);
  CHECK(execution_report.is_trade_maker == false);
  CHECK(execution_report.is_reduce_only == false);
  // CHECK(execution_report.stop_price_working_type == json::WorkingType::CONTRACT_PRICE);
  // CHECK(execution_report.original_order_type == json::OrderType::LIMIT);
  CHECK(execution_report.position_side == json::PositionSide::BOTH);
  CHECK(execution_report.if_close_all == false);
  CHECK(execution_report.realized_profit == 0.0_a);
  // unknown from here
}
