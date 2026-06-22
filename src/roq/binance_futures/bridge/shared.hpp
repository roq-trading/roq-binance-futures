/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/utils/metrics/profile.hpp"

#include "roq/fix/bridge/manager.hpp"

#include "roq/server.hpp"

#include "roq/binance_futures/bridge/fix_log.hpp"
#include "roq/binance_futures/bridge/settings.hpp"
#include "roq/binance_futures/bridge/user.hpp"

#include "roq/binance_futures/bridge/tools/crypto.hpp"

namespace roq {
namespace binance_futures {
namespace bridge {

struct Shared final {
  Shared(server::Strategy &, Settings const &, fix::bridge::Manager &);

  Shared(Shared &&) = delete;
  Shared(Shared const &) = delete;

  // session

  template <typename Callback>
  void session_logon(
      uint64_t session_id,
      std::string_view const &component,
      std::string_view const &username,
      std::string_view const &password,
      std::string_view const &raw_data,
      Callback callback) {
    auto [account, user, reason] = session_logon_helper(session_id, component, username, password, raw_data);
    auto success = std::empty(reason);
    auto user_id = user ? (*user).user_id : 0;
    callback(success, account, user_id, reason);
  }

  void session_logout(uint64_t session_id);

  // routing v2

  bool add_route(uint64_t session_id, uint32_t strategy_id);
  bool remove_route(uint64_t session_id, uint32_t strategy_id);
  void remove_all_routes(uint64_t session_id);

  // metrics

  void operator()(metrics::Writer &) const;

  // ...

  server::Strategy &dispatcher;

  Settings const &settings;

  fix::bridge::Manager &bridge;

  tools::Crypto crypto;

  FIXLog fix_log;

  struct {
    utils::metrics::Profile parse,                                                  //
        logon, logout, test_request, resend_request, reject, heartbeat,             //
        trading_session_status_request,                                             //
        security_list_request,                                                      //
        security_definition_request, security_status_request, market_data_request,  //
        user_request, order_status_request, order_mass_status_request,              //
        new_order_single, order_cancel_request, order_cancel_replace_request,       //
        order_mass_cancel_request,                                                  //
        trade_capture_report_request,                                               //
        request_for_positions,                                                      //
        mass_quote, quote_cancel;
  } profile;

  struct {
    utils::metrics::internal_latency_t events, end_to_end;
    utils::metrics::external_latency_t round_trip_gateway, round_trip_broker, round_trip_exchange;
  } latency;

 protected:
  std::tuple<std::string_view, User const *, std::string> session_logon_helper(
      uint64_t session_id,
      std::string_view const &component,
      std::string_view const &username,
      std::string_view const &password,
      std::string_view const &raw_data);

 private:
  // sessions
  // - config
  utils::unordered_map<std::string, User> const username_to_user_;
  // ...
  std::vector<Mask<Filter>> oms_cancel_all_orders_;
  // route v2
  utils::unordered_map<uint32_t, uint64_t> strategy_id_to_session_id_;
  utils::unordered_map<uint64_t, utils::unordered_set<uint32_t>> session_id_to_strategy_id_;
};

}  // namespace bridge
}  // namespace binance_futures
}  // namespace roq
