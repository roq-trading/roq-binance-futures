/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/binance_futures/api.hpp"

#include "roq/exceptions.hpp"

#include "roq/binance_futures/flags.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

API API::create() {
  auto api = Flags::api();
  if (api.compare("dapi"sv) == 0) {
    return {
        .get_exchange_info = "/dapi/v1/exchangeInfo"sv,
        .get_depth = "/dapi/v1/depth"sv,
        .get_listen_key = "/dapi/v1/listenKey"sv,
        .get_balance = "/dapi/v1/balance"sv,
        .get_account = "/dapi/v1/account"sv,
        .get_open_orders = "/dapi/v1/openOrders"sv,
        .order = "/dapi/v1/order"sv,
        .all_open_orders = "/dapi/v1/allOpenOrders"sv,
        .countdown_cancel_all = "/dapi/v1/countdownCancelAll"sv,
    };
  }
  if (api.compare("fapi"sv) == 0) {
    return {
        .get_exchange_info = "/fapi/v1/exchangeInfo"sv,
        .get_depth = "/fapi/v1/depth"sv,
        .get_listen_key = "/fapi/v1/listenKey"sv,
        .get_balance = "/fapi/v2/balance"sv,
        .get_account = "/fapi/v2/account"sv,
        .get_open_orders = "/fapi/v1/openOrders"sv,
        .order = "/fapi/v1/order"sv,
        .all_open_orders = "/fapi/v1/allOpenOrders"sv,
        .countdown_cancel_all = "/fapi/v1/countdownCancelAll"sv,
    };
  }
  throw RuntimeError(R"(Unknown api="{}")"sv, api);
}

}  // namespace binance_futures
}  // namespace roq
