/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/binance_futures/settings.hpp"

namespace roq {
namespace binance_futures {

struct API final {
  // rest
  std::string_view get_exchange_info;
  std::string_view get_depth;
  // ws
  std::string_view get_listen_key;
  std::string_view get_balance;
  std::string_view get_account;
  std::string_view get_open_orders;
  std::string_view get_trades;
  std::string_view order;
  std::string_view all_open_orders;
  std::string_view countdown_cancel_all;
  // oms
  bool modify_order_full = {};
  // factory
  static API create(Settings const &);
};

}  // namespace binance_futures
}  // namespace roq
