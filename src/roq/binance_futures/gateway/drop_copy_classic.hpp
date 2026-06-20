/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/binance_futures/gateway/account.hpp"
#include "roq/binance_futures/gateway/drop_copy.hpp"
#include "roq/binance_futures/gateway/request.hpp"
#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/protocol/json/user_stream_parser.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct DropCopyClassic final : public DropCopy, public web::socket::Client::Handler, public protocol::json::UserStreamParser::Handler {
  struct Handler {};

  DropCopyClassic(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &, std::string_view const &listen_key);

  DropCopyClassic(DropCopyClassic &&) = delete;
  DropCopyClassic(DropCopyClassic const &) = delete;

  // DropCopy

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

 protected:
  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;
  //
  std::string_view get_query() const override { return query_; }

  // helpers

  bool ready() const;

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  enum class State {
    UNDEFINED = 0,
    BALANCE,
    ACCOUNT,
    ORDERS,
    TRADES,
    DONE,
  };

  uint32_t download(State);

  void parse(std::string_view const &message);

  // protocol::json::UserStreamParser::Handler

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

  void check_response_balance();
  void check_response_account();
  void check_response_orders();
  void check_response_trades();

 private:
  Handler &handler_;
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
  core::Download<State> download_;
  // ...
  std::chrono::nanoseconds balance_refresh_ = {};
  //
  std::string external_order_id_;
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
