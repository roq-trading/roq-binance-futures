/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <string>
#include <string_view>

#include "roq/core/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/client.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/binance_futures/order_entry_state.h"
#include "roq/binance_futures/security.h"
#include "roq/binance_futures/shared.h"

#include "roq/binance_futures/json/account.h"
#include "roq/binance_futures/json/auto_cancel_all_open_orders.h"
#include "roq/binance_futures/json/balance.h"
#include "roq/binance_futures/json/cancel_all_open_orders.h"
#include "roq/binance_futures/json/cancel_order.h"
#include "roq/binance_futures/json/listen_key.h"
#include "roq/binance_futures/json/new_order.h"
#include "roq/binance_futures/json/open_orders.h"

namespace roq {
namespace binance_futures {

class OrderEntry final : public core::web::Client::Handler {
 public:
  struct ListenKeyUpdate final {
    std::string_view account;
    std::string_view listen_key;
  };

  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<ReferenceData> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<MarketStatus> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<FundsUpdate> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<PositionUpdate> &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(const ListenKeyUpdate &) = 0;
  };

  OrderEntry(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  OrderEntry(OrderEntry &&) = delete;
  OrderEntry(const OrderEntry &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  uint16_t operator()(
      const Event<CreateOrder> &, const oms::Order &, const std::string_view &request_id);
  uint16_t operator()(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  uint16_t operator()(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);

  uint16_t operator()(const Event<CancelAllOrders> &, const std::string_view &request_id);

 protected:
  void operator()(const core::web::Client::Connected &) override;
  void operator()(const core::web::Client::Disconnected &) override;
  void operator()(const core::web::Client::Latency &) override;

  void operator()(ConnectionStatus);

  uint32_t download(OrderEntryState state);

  void get_listen_key();
  void get_listen_key_ack(const server::Trace<core::web::Response> &);
  void operator()(const server::Trace<json::ListenKey> &);

  void get_balance();
  void get_balance_ack(const server::Trace<core::web::Response> &);
  void operator()(const server::Trace<json::Balance> &);

  void get_account();
  void get_account_ack(const server::Trace<core::web::Response> &);
  void operator()(const server::Trace<json::Account> &);

  void get_open_orders();
  void get_open_orders_ack(const server::Trace<core::web::Response> &);
  void operator()(const server::Trace<json::OpenOrders> &);

  void refresh_listen_key();

  void new_order(
      const Event<CreateOrder> &, const oms::Order &order, const std::string_view &request_id);
  void new_order_ack(
      const server::Trace<core::web::Response> &,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);
  void operator()(
      const server::Trace<json::NewOrder> &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_order(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  void cancel_order_ack(
      const server::Trace<core::web::Response> &,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);
  void operator()(
      const server::Trace<json::CancelOrder> &,
      uint8_t user_id,
      uint32_t order_id,
      uint32_t version);

  void cancel_all_open_orders(const Event<CancelAllOrders> &, const std::string_view &request_id);
  void cancel_all_open_orders_ack(const server::Trace<core::web::Response> &);
  void operator()(const server::Trace<json::CancelAllOpenOrders> &);

  void auto_cancel_all_open_orders();
  void auto_cancel_all_open_orders_ack(const server::Trace<core::web::Response> &);
  void operator()(const server::Trace<json::AutoCancelAllOpenOrders> &);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  core::web::Client connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile listen_key, listen_key_ack,       //
        balance, balance_ack,                                //
        account, account_ack,                                //
        open_orders, open_orders_ack,                        //
        new_order, new_order_ack,                            //
        cancel_order, cancel_order_ack,                      //
        cancel_all_open_orders, cancel_all_open_orders_ack,  //
        auto_cancel_all_open_orders, auto_cancel_all_open_orders_ack;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  std::string listen_key_;
  // state
  std::chrono::nanoseconds listen_key_refresh_ = {};
  ConnectionStatus status_ = {};
  server::Download<OrderEntryState> download_;
  // experimental
  absl::flat_hash_set<std::string> open_orders_symbols_;
  std::chrono::nanoseconds next_auto_cancel_ = {};
};

}  // namespace binance_futures
}  // namespace roq
