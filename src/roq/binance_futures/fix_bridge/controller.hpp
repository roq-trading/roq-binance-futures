/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/io/sys/signal.hpp"

#include "roq/fix/bridge/manager.hpp"

#include "roq/server.hpp"

#include "roq/binance_futures/fix_bridge/config.hpp"
#include "roq/binance_futures/fix_bridge/settings.hpp"
#include "roq/binance_futures/fix_bridge/shared.hpp"

#include "roq/binance_futures/fix_bridge/session_manager.hpp"
#include "roq/binance_futures/fix_bridge/user.hpp"

namespace roq {
namespace binance_futures {
namespace fix_bridge {

struct Controller final : public io::sys::Signal::Handler, public client::Handler, public fix::bridge::Manager::Handler {
  Controller(Settings const &, Config const &, io::Context &context);

  Controller(Controller &&) = delete;
  Controller(Controller const &) = delete;

  void dispatch();

 protected:
  // helpers

  void refresh(std::chrono::nanoseconds now);

  // io::sys::Signal::Handler

  void operator()(io::sys::Signal::Event const &) override;

  // client::Handler

  void operator()(Event<DownloadBegin> const &) override;
  void operator()(Event<DownloadEnd> const &) override;

  void operator()(Event<Ready> const &) override;

  void operator()(Event<GatewaySettings> const &) override;
  void operator()(Event<StreamStatus> const &) override;
  void operator()(Event<GatewayStatus> const &) override;

  void operator()(Event<ReferenceData> const &) override;
  void operator()(Event<MarketStatus> const &) override;
  void operator()(Event<TopOfBook> const &) override;
  void operator()(Event<MarketByPriceUpdate> const &) override;
  void operator()(Event<TradeSummary> const &) override;
  void operator()(Event<StatisticsUpdate> const &) override;

  void operator()(Event<OrderAck> const &) override;
  void operator()(Event<OrderUpdate> const &) override;

  void operator()(Event<TradeUpdate> const &) override;

  void operator()(Event<FundsUpdate> const &) override;
  void operator()(Event<PositionUpdate> const &) override;

  // fix::bridge::Manager::Handler

  std::pair<fix::codec::Error, uint32_t> operator()(fix::bridge::Manager::Credentials const &) override;

  void operator()(CreateOrder const &, uint8_t source) override;
  void operator()(ModifyOrder const &, uint8_t source) override;
  void operator()(CancelOrder const &, uint8_t source) override;
  void operator()(CancelAllOrders const &) override;

  void operator()(roq::MassQuote const &, uint8_t source) override;
  void operator()(CancelQuotes const &, uint8_t source) override;

  void operator()(Trace<fix::bridge::Manager::Disconnect> const &, uint64_t session_id) override;

  void operator()(Trace<fix::codec::Reject> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::Logon> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::Logout> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::Heartbeat> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::TestRequest> const &, uint64_t session_id) override;

  void operator()(Trace<fix::codec::BusinessMessageReject> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::UserResponse> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::TradingSessionStatus> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::SecurityList> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::SecurityDefinition> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::SecurityStatus> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::MarketDataRequestReject> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::MarketDataSnapshotFullRefresh> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::MarketDataIncrementalRefresh> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::ExecutionReport> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::OrderCancelReject> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::OrderMassCancelReport> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::TradeCaptureReportRequestAck> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::TradeCaptureReport> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::RequestForPositionsAck> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::PositionReport> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::MassQuoteAck> const &, uint64_t session_id) override;
  void operator()(Trace<fix::codec::QuoteStatusReport> const &, uint64_t session_id) override;

  void operator()(Trace<fix::codec::ExecutionReport> const &) override;

  // helpers

  template <typename T>
  void dispatch(Trace<T> const &, uint64_t session_id);

 private:
  Settings const &settings_;
  std::unique_ptr<io::sys::Signal> const terminate_;
  std::unique_ptr<io::sys::Signal> const interrupt_;
  std::unique_ptr<server::Strategy> const dispatcher_;
  std::unique_ptr<fix::bridge::Manager> const bridge_;
  Shared shared_;
  SessionManager session_manager_;
  std::chrono::nanoseconds next_update_ = {};
};

}  // namespace fix_bridge
}  // namespace binance_futures
}  // namespace roq
