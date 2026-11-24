/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/container.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/gauge.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/binance_futures/gateway/account.hpp"
#include "roq/binance_futures/gateway/order_entry.hpp"
#include "roq/binance_futures/gateway/request.hpp"
#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/protocol/json/listen_key_ack.hpp"

#include "roq/binance_futures/protocol/json/account_balance_ack.hpp"
#include "roq/binance_futures/protocol/json/account_status_ack.hpp"
#include "roq/binance_futures/protocol/json/open_orders_ack.hpp"
#include "roq/binance_futures/protocol/json/order_ack.hpp"
#include "roq/binance_futures/protocol/json/trades_ack.hpp"

#include "roq/binance_futures/protocol/json/open_orders_cancel_all_ack.hpp"
#include "roq/binance_futures/protocol/json/order_cancel_ack.hpp"
#include "roq/binance_futures/protocol/json/order_modify_ack.hpp"
#include "roq/binance_futures/protocol/json/order_place_ack.hpp"

#include "roq/binance_futures/protocol/json/countdown_cancel_all_ack.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct OrderEntryClassic final : public Base<OrderEntryClassic>, public OrderEntry, public web::rest::Client::Handler {
  struct ListenKeyUpdate final {
    std::string_view account;
    std::string_view listen_key;
  };

  struct Handler {
    virtual void operator()(Trace<ListenKeyUpdate> const &) = 0;
  };

  OrderEntryClassic(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &);

  void force_listen_key_refresh(TraceInfo const &) override;

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

  void operator()(Trace<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  void operator()(
      Trace<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  void operator()(
      Trace<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  void operator()(Trace<CancelAllOrders> const &, std::string_view const &request_id) override;

  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Connected> const &) override;
  void operator()(Trace<web::rest::Disconnected> const &) override;
  void operator()(Trace<web::rest::Latency> const &) override;
  void operator()(Trace<web::rest::MessageBegin> const &) override;
  void operator()(Trace<web::rest::MessageHeader> const &) override;
  void operator()(Trace<web::rest::MessageEnd> const &) override;

  // core::Download

  enum class State {
    UNDEFINED = 0,
    LISTEN_KEY,
    DONE,
  };

  int32_t download(Trace<State> const &);

  // listen-key

  void get_listen_key(TraceInfo const &);
  void get_listen_key_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::ListenKeyAck> const &);

  void delete_listen_key(TraceInfo const &);

  // account-balance

  void get_account_balance(TraceInfo const &);
  void get_account_balance_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::AccountBalanceAck> const &);

  // account-status

  void get_account_status(TraceInfo const &);
  void get_account_status_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::AccountStatusAck> const &);

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

  // refresh-listen-key

  void refresh_listen_key(TraceInfo const &);

  // order-place

  void order_place(Trace<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  void order_place_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<protocol::json::OrderPlaceAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // order-modify

  void order_modify(
      Trace<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void order_modify_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<protocol::json::OrderModifyAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // order-cancel

  void order_cancel(
      Trace<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void order_cancel_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<protocol::json::OrderCancelAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // open-orders-cancel-all

  void open_orders_cancel_all(Trace<CancelAllOrders> const &, std::string_view const &request_id);
  void open_orders_cancel_all_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);
  void operator()(Trace<protocol::json::OpenOrdersCancelAllAck> const &, std::string_view const &request_id);

  // countdown-cancel-all

  void countdown_cancel_all(TraceInfo const &);
  void countdown_cancel_all_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<protocol::json::CountdownCancelAllAck> const &);

  // helpers

  void check_request_queue(TraceInfo const &);

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

  void waf_limit_violation();

 private:
  Handler &handler_;
  //
  std::string const name_;
  std::unique_ptr<web::rest::Client> const connection_;
  //
  core::json::BufferStack decode_buffer_;
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile  //
        listen_key,
        listen_key_ack,                                      //
        account_balance, account_balance_ack,                //
        account_status, account_status_ack,                  //
        open_orders, open_orders_ack,                        //
        order, order_ack,                                    //
        trades, trades_ack,                                  //
        order_place, order_place_ack,                        //
        order_modify, order_modify_ack,                      //
        order_cancel, order_cancel_ack,                      //
        open_orders_cancel_all, open_orders_cancel_all_ack,  //
        countdown_cancel_all, countdown_cancel_all_ack;
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
  std::string listen_key_;
  std::chrono::nanoseconds listen_key_refresh_ = {};
  core::Download2<State> download_;
  utils::unordered_set<std::string> open_orders_symbols_;
  std::chrono::nanoseconds next_auto_cancel_ = {};
  std::string encode_buffer_;
  bool download_trades_is_first_ = true;
  std::string external_order_id_;
  size_t download_stale_orders_counter_ = {};
  // TEST
  std::chrono::nanoseconds next_listen_key_stop_ = {};
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
