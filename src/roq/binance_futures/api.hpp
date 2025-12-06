/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/binance_futures/settings.hpp"

namespace roq {
namespace binance_futures {

struct API final {
  struct {
    std::string_view exchange_info;
    std::string_view depth;
    std::string_view kline;
  } market_data;
  struct {
    std::string_view listen_key;
    std::string_view account_balance;
    std::string_view account_status;
    std::string_view open_orders;
    std::string_view user_trades;
    std::string_view order;
    std::string_view all_open_orders;
    std::string_view countdown_cancel_all;
  } simple;
  struct {
    std::string_view listen_key;
    std::string_view account_balance;
    std::string_view account_status;
    std::string_view position_risk;
    std::string_view open_orders;
    std::string_view user_trades;
    std::string_view order;
    std::string_view all_open_orders;
  } papi;
  // oms
  bool modify_order_full = {};

  // factory
  static API create(Settings const &);
};

}  // namespace binance_futures
}  // namespace roq
