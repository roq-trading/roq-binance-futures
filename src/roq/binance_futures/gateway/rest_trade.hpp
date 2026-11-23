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

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/binance_futures/gateway/account.hpp"
#include "roq/binance_futures/gateway/request.hpp"
#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/protocol/json/open_orders_ack.hpp"
#include "roq/binance_futures/protocol/json/order_ack.hpp"
#include "roq/binance_futures/protocol/json/trades_ack.hpp"

#include "roq/binance_futures/protocol/json/open_orders_cancel_all_ack.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct RestTrade final : public Base<RestTrade>, public server::OrderActionStream, public web::rest::Client::Handler {
  struct Handler {};

  RestTrade(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &);

 protected:
  friend base_type;

  // server::Stream

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::OrderActionStream

  void operator()(Trace<server::oms::OrderActionBatchBegin> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchCommit> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchRollback> const &) override;

  uint16_t operator()(Trace<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Trace<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Trace<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(Trace<CancelAllOrders> const &, std::string_view const &request_id) override;

  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Connected> const &) override;
  void operator()(Trace<web::rest::Disconnected> const &) override;
  void operator()(Trace<web::rest::Latency> const &) override;
  void operator()(Trace<web::rest::MessageBegin> const &) override;
  void operator()(Trace<web::rest::MessageHeader> const &) override;
  void operator()(Trace<web::rest::MessageEnd> const &) override;

  // open-orders

  void get_open_orders(TraceInfo const &);
  void get_open_orders_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::OpenOrdersAck> const &);

  // order

  void get_order(TraceInfo const &, std::string_view const &client_order_id, std::string_view const &external_order_id, std::string_view const &symbol);
  void get_order_ack(Trace<web::rest::Response> const &, std::string_view const &client_order_id);
  void operator()(Trace<protocol::json::OrderAck> const &);

  // trades

  void get_trades(TraceInfo const &);
  void get_trades_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::TradesAck> const &);

  // open-orders-cancel-all

  void open_orders_cancel_all(Trace<CancelAllOrders> const &, std::string_view const &request_id);
  void open_orders_cancel_all_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);
  void operator()(Trace<protocol::json::OpenOrdersCancelAllAck> const &, std::string_view const &request_id);

  // helpers

  void check_request_queue(TraceInfo const &);

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

  void operator()(Trace<server::oms::OrderUpdate> const &);

  void waf_limit_violation();

 private:
  [[maybe_unused]] Handler &handler_;
  //
  std::string const name_;
  std::unique_ptr<web::rest::Client> const connection_;
  //
  core::json::BufferStack decode_buffer_;
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile open_orders, open_orders_ack,  //
        order, order_ack,                                  //
        trades, trades_ack,                                //
        open_orders_cancel_all, open_orders_cancel_all_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  struct {
    utils::metrics::Gauge request_weight_1m, create_order_1m;
  } rate_limiter_;
  Account &account_;
  Shared &shared_;
  Request &request_;
  utils::unordered_set<std::string> open_orders_symbols_;
  std::string encode_buffer_;
  bool download_trades_is_first_ = true;
  std::string external_order_id_;
  size_t download_stale_orders_counter_ = {};
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
