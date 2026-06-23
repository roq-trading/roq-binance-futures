/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/bridge/shared.hpp"

#include "roq/utils/metrics/factory.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace bridge {

// === HELPERS ===

namespace {
struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto const &function) : utils::metrics::Factory{settings.app.name, "session"sv, function} {}
};

auto create_internal_latency(auto &settings) {
  auto labels = fmt::format(R"(source="{}")"sv, settings.app.name);
  return utils::metrics::internal_latency_t{labels};
}

auto create_external_latency(auto &settings, auto const &function) {
  auto labels = fmt::format(R"(source="{}", function="{}")"sv, settings.app.name, function);
  return utils::metrics::external_latency_t{labels};
}

template <typename R>
auto create_username_to_user() {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  // XXX FIXME TODO parse from config
  auto username = "trader"s;
  auto user = User{
      .user_id = 1,
      .component = "fix-client"s,
      .username = username,
      .password = "secret"s,
      .strategy_id = {},
      .account = "A1"s,
      .accounts_regex = {},
      .symbols_regex = {},
  };
  result.emplace(username, std::move(user));
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::Strategy &dispatcher, Settings const &settings, fix::bridge::Manager &bridge)
    : dispatcher{dispatcher}, settings{settings}, bridge{bridge}, crypto{settings.fix.fix_auth_method, settings.fix.fix_auth_timestamp_tolerance},
      fix_log{settings},
      profile{
          .parse = create_metrics(settings, "parse"sv),
          .logon = create_metrics(settings, "logon"sv),
          .logout = create_metrics(settings, "logout"sv),
          .test_request = create_metrics(settings, "test_request"sv),
          .resend_request = create_metrics(settings, "resend_request"sv),
          .reject = create_metrics(settings, "reject"sv),
          .heartbeat = create_metrics(settings, "heartbeat"sv),
          .trading_session_status_request = create_metrics(settings, "trading_session_status_request"sv),
          .security_list_request = create_metrics(settings, "security_list_request"sv),
          .security_definition_request = create_metrics(settings, "security_definition_request"sv),
          .security_status_request = create_metrics(settings, "security_status_request"sv),
          .market_data_request = create_metrics(settings, "market_data_request"sv),
          .user_request = create_metrics(settings, "user_request"sv),
          .order_status_request = create_metrics(settings, "order_status_request"sv),
          .order_mass_status_request = create_metrics(settings, "order_mass_status_request"sv),
          .new_order_single = create_metrics(settings, "new_order_single"sv),
          .order_cancel_request = create_metrics(settings, "order_cancel_request"sv),
          .order_cancel_replace_request = create_metrics(settings, "order_cancel_replace_request"sv),
          .order_mass_cancel_request = create_metrics(settings, "order_mass_cancel_request"sv),
          .trade_capture_report_request = create_metrics(settings, "trade_capture_report_request"sv),
          .request_for_positions = create_metrics(settings, "request_for_positions"sv),
          .mass_quote = create_metrics(settings, "mass_quote"sv),
          .quote_cancel = create_metrics(settings, "quote_cancel"sv),
      },
      latency{
          .events = create_internal_latency(settings),
          .end_to_end = create_internal_latency(settings),
          .round_trip_gateway = create_external_latency(settings, "gateway"sv),
          .round_trip_broker = create_external_latency(settings, "broker"sv),
          .round_trip_exchange = create_external_latency(settings, "exchange"sv),
      },
      username_to_user_{create_username_to_user<decltype(username_to_user_)>()} {
}

// session

std::tuple<std::string_view, User const *, std::string> Shared::session_logon_helper(
    uint64_t session_id,
    std::string_view const &component,
    std::string_view const &username,
    std::string_view const &password,
    std::string_view const &raw_data) {
  log::info(R"(Session: ADD session_id={} (component="{}", username="{}"))"sv, session_id, component, username);
  auto iter = username_to_user_.find(username);
  if (iter == std::end(username_to_user_)) {
    return {{}, {}, fmt::format(R"(Unknown username="{}")"sv, username)};
  }
  auto &user = (*iter).second;
  if (std::empty(user.account)) {
    return {};
  }
  /*
  if (settings.test.block_client_on_not_ready) {
    auto ready = true;
    for (auto &item : oms_status_) {
      auto iter = item.find(user.account);
      if (iter != item.end() && (*iter).second == ConnectionStatus::READY) {
      } else {
        ready = false;
      }
    }
    if (!ready) {
      return {{}, {}, fmt::format(R"(Not ready: account="{}")"sv, user.account)};
    }
  }
  */
  if (component != user.component) {
    return {{}, {}, "Invalid component"s};
  }
  if (!crypto.validate(password, user.password, raw_data)) {
    return {{}, {}, "Invalid password"s};
  }
  return {user.account, &user, {}};
}

void Shared::session_logout(uint64_t session_id) {
  log::info("Session: REMOVE session_id={}"sv, session_id);
}

// routing v2

bool Shared::add_route(uint64_t session_id, uint32_t strategy_id) {
  auto iter = strategy_id_to_session_id_.find(strategy_id);
  if (iter == std::end(strategy_id_to_session_id_)) {
    strategy_id_to_session_id_.try_emplace(strategy_id, session_id);
    session_id_to_strategy_id_[session_id].emplace(strategy_id);
    log::info(R"(DEBUG: ROUTE ADD strategy_id={} <==> session_id={})"sv, strategy_id, session_id);
    return true;
  }
  return false;
}

bool Shared::remove_route(uint64_t session_id, uint32_t strategy_id) {
  auto iter = strategy_id_to_session_id_.find(strategy_id);
  if (iter == std::end(strategy_id_to_session_id_) || (*iter).second != session_id) {
    return false;
  }
  log::info(R"(DEBUG ROUTE REMOVE strategy_id={} <==> session_id={})"sv, strategy_id, session_id);
  session_id_to_strategy_id_[session_id].erase(strategy_id);
  strategy_id_to_session_id_.erase(iter);
  return true;
}

void Shared::remove_all_routes(uint64_t session_id) {
  auto iter = session_id_to_strategy_id_.find(session_id);
  if (iter == std::end(session_id_to_strategy_id_)) {
    return;
  }
  for (auto strategy_id : (*iter).second) {
    log::info(R"(DEBUG: ROUTE REMOVE strategy_id={} <==> session_id={})"sv, strategy_id, session_id);
    strategy_id_to_session_id_.erase(strategy_id);
  }
  session_id_to_strategy_id_.erase(iter);
}

// metrics

void Shared::operator()(metrics::Writer &writer) const {
  writer
      // profile
      .write(profile.parse, metrics::Type::PROFILE)
      .write(profile.logon, metrics::Type::PROFILE)
      .write(profile.logout, metrics::Type::PROFILE)
      .write(profile.test_request, metrics::Type::PROFILE)
      .write(profile.resend_request, metrics::Type::PROFILE)
      .write(profile.reject, metrics::Type::PROFILE)
      .write(profile.heartbeat, metrics::Type::PROFILE)
      .write(profile.trading_session_status_request, metrics::Type::PROFILE)
      .write(profile.security_list_request, metrics::Type::PROFILE)
      .write(profile.security_definition_request, metrics::Type::PROFILE)
      .write(profile.security_status_request, metrics::Type::PROFILE)
      .write(profile.market_data_request, metrics::Type::PROFILE)
      .write(profile.user_request, metrics::Type::PROFILE)
      .write(profile.order_status_request, metrics::Type::PROFILE)
      .write(profile.order_mass_status_request, metrics::Type::PROFILE)
      .write(profile.new_order_single, metrics::Type::PROFILE)
      .write(profile.order_cancel_request, metrics::Type::PROFILE)
      .write(profile.order_cancel_replace_request, metrics::Type::PROFILE)
      .write(profile.order_mass_cancel_request, metrics::Type::PROFILE)
      .write(profile.trade_capture_report_request, metrics::Type::PROFILE)
      .write(profile.request_for_positions, metrics::Type::PROFILE)
      .write(profile.mass_quote, metrics::Type::PROFILE)
      .write(profile.quote_cancel, metrics::Type::PROFILE)
      //
      .write(latency.events, metrics::Type::EVENTS)
      .write(latency.end_to_end, metrics::Type::END_TO_END_LATENCY)
      .write(latency.round_trip_gateway, metrics::Type::REQUEST_LATENCY)
      .write(latency.round_trip_broker, metrics::Type::REQUEST_LATENCY)
      .write(latency.round_trip_exchange, metrics::Type::REQUEST_LATENCY);
}

}  // namespace bridge
}  // namespace binance_futures
}  // namespace roq
