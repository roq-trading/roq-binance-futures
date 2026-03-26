/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/api.hpp"

#include "roq/logging.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === HELPERS ===

namespace {
auto parse_self_trade_prevention(auto &value) -> json::SelfTradePrevention {
  if (std::empty(value)) {
    return {};
  }
  std::string tmp{value};
  std::ranges::transform(tmp, std::begin(tmp), [](auto item) { return std::toupper(item); });  // note! convert to uppercase
  json::SelfTradePrevention result{std::string_view{tmp}};
  log::warn("Using self_trade_prevention_mode={}"sv, result);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  auto api = settings.app.api;
  auto self_trade_prevention = parse_self_trade_prevention(settings.misc.self_trade_prevention_mode);
  // USD-M futures
  if (api == "fapi"sv) {
    return {
        .market_data{
            .exchange_info = "/fapi/v1/exchangeInfo"sv,
            .depth = "/fapi/v1/depth"sv,
            .kline = "/fapi/v1/klines"sv,
        },
        .simple{
            .listen_key = "/fapi/v1/listenKey"sv,
            .account_balance = "/fapi/v2/balance"sv,
            .account_status = "/fapi/v2/account"sv,
            .open_orders = "/fapi/v1/openOrders"sv,
            .user_trades = "/fapi/v1/userTrades"sv,
            .order = "/fapi/v1/order"sv,
            .all_open_orders = "/fapi/v1/allOpenOrders"sv,
            .countdown_cancel_all = "/fapi/v1/countdownCancelAll"sv,
        },
        .papi{
            .listen_key = "/papi/v1/listenKey"sv,
            .account_balance = "/papi/v1/balance"sv,
            .account_status = "/papi/v1/account"sv,
            .position_risk = "/papi/v1/um/positionRisk"sv,
            .open_orders = "/papi/v1/um/openOrders"sv,
            .user_trades = "/papi/v1/um/userTrades"sv,
            .order = "/papi/v1/um/order"sv,
            .all_open_orders = "/papi/v1/um/allOpenOrders"sv,
        },
        .modify_order_full = true,
        .self_trade_prevention = self_trade_prevention,
    };
  }
  // COIN-M futures
  if (api == "dapi"sv) {
    return {
        .market_data{
            .exchange_info = "/dapi/v1/exchangeInfo"sv,
            .depth = "/dapi/v1/depth"sv,
            .kline = "/dapi/v1/klines"sv,
        },
        .simple{
            .listen_key = "/dapi/v1/listenKey"sv,
            .account_balance = "/dapi/v1/balance"sv,
            .account_status = "/dapi/v1/account"sv,
            .open_orders = "/dapi/v1/openOrders"sv,
            .user_trades = "/dapi/v1/userTrades"sv,
            .order = "/dapi/v1/order"sv,
            .all_open_orders = "/dapi/v1/allOpenOrders"sv,
            .countdown_cancel_all = "/dapi/v1/countdownCancelAll"sv,
        },
        .papi{
            .listen_key = "/papi/v1/listenKey"sv,
            .account_balance = "/papi/v1/balance"sv,
            .account_status = "/papi/v1/account"sv,
            .position_risk = "/papi/v1/cm/positionRisk"sv,
            .open_orders = "/papi/v1/cm/openOrders"sv,
            .user_trades = "/papi/v1/cm/userTrades"sv,
            .order = "/papi/v1/cm/order"sv,
            .all_open_orders = "/papi/v1/cm/allOpenOrders"sv,
        },
        .modify_order_full = false,
        .self_trade_prevention = self_trade_prevention,
    };
  }
  throw RuntimeError{R"(Unknown api="{}")"sv, api};
}

}  // namespace binance_futures
}  // namespace roq
