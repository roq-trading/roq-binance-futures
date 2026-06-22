/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/bridge/controller.hpp"

#include <cassert>

#include "roq/logging.hpp"

#include "roq/clock.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/io/sys/scheduler.hpp"

#include "roq/binance_futures/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace bridge {

// === CONSTANTS ===

namespace {
auto const YIELD_FREQUENCY = 100ms;
size_t const DISPATCH_THIS_MANY_BEFORE_CHECKING_CLOCK = 1000;
// XXX FIXME
auto const USER = "trader"sv;
}  // namespace

// === HELPERS ===

namespace {
auto create_dispatcher(auto &handler, auto &settings, auto &config, auto &context) {
  auto helper = [](auto &dispatcher, auto &settings, auto &config, auto &context) {
    return gateway::Controller::create(dispatcher, settings, config, context);
  };
  return std::make_unique<server::Strategy>(handler, settings, config, context, USER, helper);
}

auto create_bridge(auto &handler, auto &settings) {
  /*
  std::vector<std::pair<StatisticsType, double>> default_values;
  for (auto &item : config.default_values) {
    default_values.emplace_back(item);
  }
  std::vector<std::pair<StatisticsType, fix::MDEntryType>> statistics;
  for (auto &item : config.statistics) {
    statistics.emplace_back(item);
  }
  std::vector<std::pair<Error, fix::CxlRejReason>> cxl_rej_reason_mapping;
  for (auto &item : config.cxl_rej_reason) {
    cxl_rej_reason_mapping.emplace_back(item);
  }
  */
  auto options = fix::bridge::Manager::Options{
      // session
      .comp_id = settings.fix.fix_comp_id,
      .logon_timeout = utils::safe_cast(settings.fix.fix_logon_timeout),
      .logon_heartbeat_min = utils::safe_cast(settings.fix.fix_logon_heartbeat_min),
      .logon_heartbeat_max = utils::safe_cast(settings.fix.fix_logon_heartbeat_max),
      .heartbeat_freq = utils::safe_cast(settings.fix.fix_heartbeat_freq),
      // market data
      .top_of_book_from_market_by_price = settings.messaging.top_of_book_from_market_by_price,
      .default_values = {},  // default_values,
      .statistics = {},      // statistics,
      // order management
      .route_by_strategy = settings.oms.oms_route_by_strategy,
      .strict_checking = settings.test.strict_checking,
      .disable_req_id_validation = settings.fix.fix_disable_req_id_validation,
      .cxl_rej_reason_mapping = {},  // cxl_rej_reason_mapping,
      // test:
      // - market data
      .test_md_entry_position_no = settings.test.test_md_entry_position_no,
      .test_init_missing_md_entry_type_to_zero = settings.messaging.init_missing_md_entry_type_to_zero,
      // - order management
      .test_request_latency = settings.test.test_request_latency,
      .test_terminate_on_timeout = settings.test.terminate_on_timeout,
      .test_silence_non_final_order_ack = settings.test.silence_non_final_order_ack,
      .test_skip_order_download = settings.test.skip_order_download,
      // - other
      .test_terminate_on_disconnect{settings.test.terminate_on_disconnect},
      .test_block_client_on_not_ready{settings.test.block_client_on_not_ready},
      .default_decimal_digits = settings.test.test_default_decimal_digits,
  };
  return fix::bridge::Manager::create(handler, options);
}

}  // namespace

// === IMPLEMENTATION ===

Controller::Controller(Settings const &settings, Config const &config, io::Context &context)
    : settings_{settings}, terminate_{context.create_signal(*this, io::sys::Signal::Type::TERMINATE)},
      interrupt_{context.create_signal(*this, io::sys::Signal::Type::INTERRUPT)}, dispatcher_{create_dispatcher(*this, settings, config, context)},
      bridge_{create_bridge(*this, settings)}, shared_{*dispatcher_, settings, *bridge_}, session_manager_{shared_} {
}

void Controller::dispatch() {
  (*dispatcher_).start();
  std::chrono::nanoseconds next_yield_ = {};
  auto ok = true;
  while (ok) {
    auto now = clock::get_system();
    refresh(now);
    if (next_yield_ < now && YIELD_FREQUENCY.count() > 0) {
      next_yield_ = now + YIELD_FREQUENCY;
      io::sys::Scheduler::yield();
    }
    for (size_t i = 0; ok && i < DISPATCH_THIS_MANY_BEFORE_CHECKING_CLOCK; ++i) {
      ok = (*dispatcher_).dispatch();
    }
  }
}

void Controller::refresh(std::chrono::nanoseconds now) {
  if (now < next_update_) {
    return;
  }
}

// io::sys::Signal::Handler

void Controller::operator()(io::sys::Signal::Event const &event) {
  log::warn("*** SIGNAL: {} ***"sv, event.type);
  (*dispatcher_).stop();
}

// client::Handler

void Controller::operator()(Event<DownloadBegin> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<DownloadEnd> const &event) {
  auto &[message_info, download_end] = event;
  log::warn("download_end={}"sv, download_end);
  auto max_order_id = download_end.max_order_id;
  if (max_order_id_ < max_order_id) {
    max_order_id_ = max_order_id;
    log::info("max_order_id={}"sv, max_order_id_);
  }
  //
  shared_.bridge(event);
}

void Controller::operator()(Event<Ready> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<GatewaySettings> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<StreamStatus> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<GatewayStatus> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<ReferenceData> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<MarketStatus> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<TopOfBook> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<MarketByPriceUpdate> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<TradeSummary> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<StatisticsUpdate> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<OrderAck> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<OrderUpdate> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<TradeUpdate> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<FundsUpdate> const &event) {
  shared_.bridge(event);
}

void Controller::operator()(Event<PositionUpdate> const &event) {
  shared_.bridge(event);
}

// fix::bridge::Manager::Handler

std::pair<fix::codec::Error, uint32_t> Controller::operator()(fix::bridge::Manager::Credentials const &credentials) {
  auto iter = username_to_user_.find(credentials.username);
  if (iter == std::end(username_to_user_)) {
    log::error(R"(Unknown username "{}")"sv, credentials.username);
    return {fix::codec::Error::INVALID_USERNAME, {}};
  }
  auto &user = (*iter).second;
  if (credentials.component != user.component) {
    log::error(R"(Unexpected component "{}", expected "{}")"sv, credentials.component, user.component);
    return {fix::codec::Error::INVALID_COMPONENT, {}};
  }
  if (!shared_.crypto.validate(credentials.password, user.password, credentials.raw_data)) {
    log::error("Unexpected password");
    return {fix::codec::Error::INVALID_PASSWORD, {}};
  }
  return {{}, user.strategy_id};
}

void Controller::operator()(CreateOrder const &create_order, uint8_t source) {
  (*dispatcher_).send(create_order, source);
}

void Controller::operator()(ModifyOrder const &modify_order, uint8_t source) {
  (*dispatcher_).send(modify_order, source);
}

void Controller::operator()(CancelOrder const &cancel_order, uint8_t source) {
  (*dispatcher_).send(cancel_order, source);
}

void Controller::operator()(CancelAllOrders const &cancel_all_orders) {
  (*dispatcher_).send(cancel_all_orders);
}

void Controller::operator()(roq::MassQuote const &, [[maybe_unused]] uint8_t source) {
  // (*dispatcher_).send(mass_quote, source);
}

void Controller::operator()(CancelQuotes const &, [[maybe_unused]] uint8_t source) {
  // (*dispatcher_).send(cancel_quotes, source);
}

void Controller::operator()(Trace<fix::bridge::Manager::Disconnect> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::Reject> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::Logon> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::Logout> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::Heartbeat> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::TestRequest> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::BusinessMessageReject> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::UserResponse> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::TradingSessionStatus> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::SecurityList> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::SecurityDefinition> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::SecurityStatus> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::MarketDataRequestReject> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::MarketDataSnapshotFullRefresh> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::MarketDataIncrementalRefresh> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::ExecutionReport> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::OrderCancelReject> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::OrderMassCancelReport> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::TradeCaptureReportRequestAck> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::TradeCaptureReport> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::RequestForPositionsAck> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::PositionReport> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::MassQuoteAck> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::QuoteStatusReport> const &event, uint64_t session_id) {
  dispatch(event, session_id);
}

void Controller::operator()(Trace<fix::codec::ExecutionReport> const &event) {
  if (shared_.settings.oms.oms_route_by_strategy) {
    // XXX FIXME TODO no_party_ids => strategy_id => session(s)
  } else {
    // XXX FIXME TODO account => regex by user => session(s)
  }
  session_manager_.get_all_sessions([&](auto &session) { session(event); });
}

// tools

template <typename T>
void Controller::dispatch(Trace<T> const &event, uint64_t session_id) {
  session_manager_.get_session(session_id, [&](auto &session) { session(event); });
}

}  // namespace bridge
}  // namespace binance_futures
}  // namespace roq
