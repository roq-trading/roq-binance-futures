/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/logging.hpp"

#include "roq/binance_futures/json/encoder.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// === HELPERS ===

namespace {
auto create_modify_order(double quantity, double price) {
  auto modify_order = ModifyOrder{
      .account = "A1"sv,
      .order_id = 1234,
      .request_template = {},
      .quantity = quantity,
      .price = price,
      .routing_id = {},
      .version = {},
      .conditional_on_version = {},
  };
  return modify_order;
}
auto create_order(double quantity, double price) {
  auto order = server::oms::Order{
      .user_id = {},
      .stream_id = {},
      .account = {},
      .order_id = {},
      .exchange = "binance"sv,
      .symbol = "BTC"sv,
      .side = Side::BUY,
      .position_effect = {},
      .margin_mode = {},
      .quantity_type = {},
      .max_show_quantity = {},
      .order_type = {},
      .time_in_force = {},
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = {},
      .external_account = {},
      .external_order_id = "oid:1234"sv,
      .client_order_id = {},
      .order_status = {},
      .quantity = quantity,
      .price = price,
      .stop_price = {},
      .leverage = NaN,
      .risk_exposure = {},
      .remaining_quantity = {},
      .traded_quantity = {},
      .average_traded_price = {},
      .last_traded_price = {},
      .last_traded_quantity = {},
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .security_type = {},
      .quantity_precision = {},
      .price_precision = {},
      .update_type = {},
      .user = {},
      .strategy_id = {},
  };
  return order;
}
}  // namespace

// === IMPLEMENTATION ===

TEST_CASE("dapi_none_1", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto modify_order = create_modify_order(1.0, 1.0);
  auto order = create_order(1.0, 1.0);
  CHECK_THROWS(json::Encoder::order_modify_url(buffer, modify_order, order, {}, {}, 5s, false));
}

TEST_CASE("dapi_none_2", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto modify_order = create_modify_order(NaN, NaN);
  auto order = create_order(1.0, 1.0);
  CHECK_THROWS(json::Encoder::order_modify_url(buffer, modify_order, order, {}, {}, 5s, false));
}

TEST_CASE("dapi_price_1", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto modify_order = create_modify_order(1.0, 2.0);
  auto order = create_order(1.0, 1.0);
  auto result = json::Encoder::order_modify_url(buffer, modify_order, order, {}, {}, 5s, false);
  CHECK(result == "symbol=BTC&orderId=oid:1234&origClientOrderId=&side=BUY&price=2&recvWindow=5000"sv);
}

TEST_CASE("dapi_price_2", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto modify_order = create_modify_order(NaN, 2.0);
  auto order = create_order(1.0, 1.0);
  auto result = json::Encoder::order_modify_url(buffer, modify_order, order, {}, {}, 5s, false);
  CHECK(result == "symbol=BTC&orderId=oid:1234&origClientOrderId=&side=BUY&price=2&recvWindow=5000"sv);
}

TEST_CASE("dapi_quantity_1", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto modify_order = create_modify_order(2.0, 1.0);
  auto order = create_order(1.0, 1.0);
  auto result = json::Encoder::order_modify_url(buffer, modify_order, order, {}, {}, 5s, false);
  CHECK(result == "symbol=BTC&orderId=oid:1234&origClientOrderId=&side=BUY&quantity=2&recvWindow=5000"sv);
}

TEST_CASE("dapi_quantity_2", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto modify_order = create_modify_order(2.0, NaN);
  auto order = create_order(1.0, 1.0);
  auto result = json::Encoder::order_modify_url(buffer, modify_order, order, {}, {}, 5s, false);
  CHECK(result == "symbol=BTC&orderId=oid:1234&origClientOrderId=&side=BUY&quantity=2&recvWindow=5000"sv);
}

TEST_CASE("dapi_both", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto modify_order = create_modify_order(2.0, 2.0);
  auto order = create_order(1.0, 1.0);
  CHECK_THROWS(json::Encoder::order_modify_url(buffer, modify_order, order, {}, {}, 5s, false));
}

TEST_CASE("rounding_issue_oc45", "[json_encoder]") {
  std::vector<char> buffer(4096);
  auto old_price = 90066.2;
  auto price = 90085.7 - 1.0e-12;
  auto order = create_order(1.0, old_price);
  order.price_precision = {0.1, Precision::_1};
  auto modify_order = create_modify_order(1.0, price);
  auto result = json::Encoder::order_modify_json(buffer, modify_order, order, {}, {}, 5s, {}, "SOME_ID");
  CHECK(
      result ==
      R"({"id":"SOME_ID","method":"order.modify","params":{"symbol":"BTC","side":"BUY","orderId":oid:1234,"quantity":"1","price":"90085.7","recvWindow":5000,"timestamp":0}})"sv);
}
