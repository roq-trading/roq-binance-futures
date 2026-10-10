/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/binance_futures/gateway/account.hpp"
#include "roq/binance_futures/gateway/request.hpp"
#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/protocol/json/user_stream_parser.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct DropCopyClassic final : public Base<DropCopyClassic>,
                               public server::Stream,
                               public web::socket::Client::Handler,
                               public protocol::json::UserStreamParser::Handler {
  struct Remove final {
    std::string_view account;
  };

  struct Handler {
    virtual void operator()(Remove const &) = 0;
  };

  DropCopyClassic(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &, std::string_view const &listen_key);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override;

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

 protected:
  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;
  //
  std::string_view get_query() const override { return query_; }

  // core::Download

  enum class State {
    UNDEFINED = 0,
    BALANCE,
    ACCOUNT,
    ORDERS,
    TRADES,
    DONE,
  };

  int32_t download(Trace<State> const &);

  // protocol::json::UserStreamParser::Handler

  void operator()(Trace<protocol::json::ListenKeyExpired> const &) override;
  void operator()(Trace<protocol::json::OrderTradeUpdate> const &) override;
  void operator()(Trace<protocol::json::AccountUpdate> const &) override;
  void operator()(Trace<protocol::json::MarginCall> const &) override;
  void operator()(Trace<protocol::json::StrategyUpdate> const &) override;
  void operator()(Trace<protocol::json::GridUpdate> const &) override;
  void operator()(Trace<protocol::json::AccountConfigUpdate> const &) override;
  void operator()(Trace<protocol::json::TradeLite> const &) override;
  void operator()(Trace<protocol::json::ExecutionReport2> const &) override;
  void operator()(Trace<protocol::json::BalanceUpdate> const &) override;
  void operator()(Trace<protocol::json::LiabilityChange> const &) override;
  void operator()(Trace<protocol::json::OutboundAccountPosition> const &) override;

  // helpers

  void refresh_balance(std::chrono::nanoseconds now);

  void request_balance();
  void request_account();
  void request_orders();
  void request_trades();

  void check_response_balance(TraceInfo const &);
  void check_response_account(TraceInfo const &);
  void check_response_orders(TraceInfo const &);
  void check_response_trades(TraceInfo const &);

  void parse(std::string_view const &message);

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // web socket
  std::string query_;
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse, order_trade_update, account_update, margin_call, strategy_update, grid_update, account_config_update, trade_lite,
        execution_report, balance_update, liability_change, outbound_account_position;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // authentication
  Account &account_;
  // shared
  Shared &shared_;
  Request &request_;
  // state
  bool ready_ = false;
  ConnectionStatus connection_status_ = {};
  core::Download2<State> download_;
  // ...
  std::chrono::nanoseconds balance_refresh_ = {};
  //
  std::string external_order_id_;
  //
  bool stop_ = {};
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
