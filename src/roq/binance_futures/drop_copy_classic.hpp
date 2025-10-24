/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/binance_futures/account.hpp"
#include "roq/binance_futures/drop_copy.hpp"
#include "roq/binance_futures/drop_copy_state.hpp"
#include "roq/binance_futures/request.hpp"
#include "roq/binance_futures/shared.hpp"

#include "roq/binance_futures/json/user_stream_parser.hpp"

namespace roq {
namespace binance_futures {

struct DropCopyClassic final : public DropCopy, public web::socket::Client::Handler, public json::UserStreamParser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
  };

  DropCopyClassic(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &, std::string_view const &listen_key);

  DropCopyClassic(DropCopyClassic &&) = delete;
  DropCopyClassic(DropCopyClassic const &) = delete;

  bool ready() const;

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void parse(std::string_view const &message);

  void operator()(Trace<json::OrderTradeUpdate> const &) override;
  void operator()(Trace<json::AccountUpdate> const &) override;
  void operator()(Trace<json::MarginCall> const &) override;
  void operator()(Trace<json::StrategyUpdate> const &) override;
  void operator()(Trace<json::GridUpdate> const &) override;
  void operator()(Trace<json::AccountConfigUpdate> const &) override;
  void operator()(Trace<json::TradeLite> const &) override;
  void operator()(Trace<json::ExecutionReport2> const &) override;
  void operator()(Trace<json::BalanceUpdate> const &) override;
  void operator()(Trace<json::LiabilityChange> const &) override;
  void operator()(Trace<json::OutboundAccountPosition> const &) override;

  void request_balance();
  void request_account();
  void request_orders();
  void request_trades();

  void check_response_balance();
  void check_response_account();
  void check_response_orders();
  void check_response_trades();

  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // web socket
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
  ConnectionStatus status_ = {};
  core::Download<DropCopyState> download_;
};

}  // namespace binance_futures
}  // namespace roq
