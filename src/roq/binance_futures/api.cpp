/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  auto api = settings.app.api;
  // USD-M futures
  if (api.compare("fapi"sv) == 0) {
    return {
        .get_exchange_info = "/fapi/v1/exchangeInfo"sv,
        .get_depth = "/fapi/v1/depth"sv,
        .get_listen_key = "/fapi/v1/listenKey"sv,
        .get_balance = "/fapi/v2/balance"sv,
        .get_account = "/fapi/v2/account"sv,
        .get_open_orders = "/fapi/v1/openOrders"sv,
        .get_trades = "/fapi/v1/userTrades"sv,
        .order = "/fapi/v1/order"sv,
        .all_open_orders = "/fapi/v1/allOpenOrders"sv,
        .countdown_cancel_all = "/fapi/v1/countdownCancelAll"sv,
        .modify_order_full = true,
        .papi{
            .get_listen_key = "/papi/v1/listenKey"sv,
            .get_position = "/papi/v1/um/positionRisk"sv,
            .get_open_orders = "/papi/v1/um/openOrders"sv,
            .get_trades = "/papi/v1/um/userTrades"sv,
            .order = "/papi/v1/um/order"sv,
            .all_open_orders = "/papi/v1/um/allOpenOrders"sv,
        },
    };
  }
  // COIN-M futures
  if (api.compare("dapi"sv) == 0) {
    return {
        .get_exchange_info = "/dapi/v1/exchangeInfo"sv,
        .get_depth = "/dapi/v1/depth"sv,
        .get_listen_key = "/dapi/v1/listenKey"sv,
        .get_balance = "/dapi/v1/balance"sv,
        .get_account = "/dapi/v1/account"sv,
        .get_open_orders = "/dapi/v1/openOrders"sv,
        .get_trades = "/dapi/v1/userTrades"sv,
        .order = "/dapi/v1/order"sv,
        .all_open_orders = "/dapi/v1/allOpenOrders"sv,
        .countdown_cancel_all = "/dapi/v1/countdownCancelAll"sv,
        .modify_order_full = false,
        .papi{
            .get_listen_key = "/papi/v1/listenKey"sv,
            .get_position = "/papi/v1/cm/positionRisk"sv,
            .get_open_orders = "/papi/v1/cm/openOrders"sv,
            .get_trades = "/papi/v1/cm/userTrades"sv,
            .order = "/papi/v1/cm/order"sv,
            .all_open_orders = "/papi/v1/cm/allOpenOrders"sv,
        },
    };
  }
  throw RuntimeError{R"(Unknown api="{}")"sv, api};
}

}  // namespace binance_futures
}  // namespace roq
