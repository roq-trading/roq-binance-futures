/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/utils/container.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/gauge.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/binance_futures/gateway/account.hpp"
#include "roq/binance_futures/gateway/request.hpp"
#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/protocol/json/account_balance_ack.hpp"
#include "roq/binance_futures/protocol/json/account_status_ack.hpp"
#include "roq/binance_futures/protocol/json/open_orders_ack.hpp"
#include "roq/binance_futures/protocol/json/trades_ack.hpp"

#include "roq/binance_futures/protocol/json/open_orders_cancel_all_ack.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct RestTrade final : public Base<RestTrade>, public server::OrderActionStream, public web::rest::Client::Handler {
  struct Handler {};

  RestTrade(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::OrderActionStream

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Connected> const &) override;
  void operator()(Trace<web::rest::Disconnected> const &) override;
  void operator()(Trace<web::rest::Latency> const &) override;
  void operator()(Trace<web::rest::MessageBegin> const &) override;
  void operator()(Trace<web::rest::MessageHeader> const &) override;
  void operator()(Trace<web::rest::MessageEnd> const &) override;

  // account-balance

  void get_account_balance();
  void get_account_balance_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::AccountBalanceAck> const &);

  // account-status

  void get_account_status();
  void get_account_status_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::AccountStatusAck> const &);

  // open-orders

  void get_open_orders();
  void get_open_orders_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::OpenOrdersAck> const &);

  // trades

  void get_trades();
  void get_trades_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::TradesAck> const &);

  // open-orders-cancel-all

  void open_orders_cancel_all(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void open_orders_cancel_all_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);
  void operator()(Trace<protocol::json::OpenOrdersCancelAllAck> const &, std::string_view const &request_id);

  // helpers

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

  void operator()(Trace<server::oms::OrderUpdate> const &);

  void waf_limit_violation();

  bool downloading() const { return download_balance_ || download_account_ || download_orders_ || download_trades_; }

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile  //
        account_balance,
        account_balance_ack,                 //
        account_status, account_status_ack,  //
        open_orders, open_orders_ack,        //
        trades, trades_ack,                  //
        open_orders_cancel_all, open_orders_cancel_all_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  struct {
    utils::metrics::Gauge request_weight_1m, create_order_1m;
  } rate_limiter_;
  // account
  Account &account_;
  // shared
  Shared &shared_;
  Request &request_;
  // state
  ConnectionStatus connection_status_ = {};
  // experimental
  utils::unordered_set<std::string> open_orders_symbols_;
  bool download_balance_ = false;
  bool download_account_ = false;
  bool download_orders_ = false;
  bool download_trades_ = false;
  std::string encode_buffer_;
  bool download_trades_is_first_ = true;
  //
  std::string external_order_id_;
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
