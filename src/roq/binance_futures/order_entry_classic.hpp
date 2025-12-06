/* Copyright (c) 2017-2025, Hans Erik Thrane */

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

#include "roq/binance_futures/account.hpp"
#include "roq/binance_futures/order_entry.hpp"
#include "roq/binance_futures/order_entry_state.hpp"
#include "roq/binance_futures/request.hpp"
#include "roq/binance_futures/shared.hpp"

#include "roq/binance_futures/json/listen_key_ack.hpp"

#include "roq/binance_futures/json/account_balance_ack.hpp"
#include "roq/binance_futures/json/account_status_ack.hpp"
#include "roq/binance_futures/json/open_orders_ack.hpp"
#include "roq/binance_futures/json/trades_ack.hpp"

#include "roq/binance_futures/json/open_orders_cancel_all_ack.hpp"
#include "roq/binance_futures/json/order_cancel_ack.hpp"
#include "roq/binance_futures/json/order_modify_ack.hpp"
#include "roq/binance_futures/json/order_place_ack.hpp"

#include "roq/binance_futures/json/countdown_cancel_all_ack.hpp"

namespace roq {
namespace binance_futures {

struct OrderEntryClassic final : public OrderEntry, public web::rest::Client::Handler {
  struct ListenKeyUpdate final {
    std::string_view account;
    std::string_view listen_key;
  };

  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<RateLimitsUpdate> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(ListenKeyUpdate const &) = 0;
  };

  OrderEntryClassic(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &);

  OrderEntryClassic(OrderEntryClassic &&) = delete;
  OrderEntryClassic(OrderEntryClassic const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }
  bool downloading() const { return download_balance_ || download_account_ || download_orders_ || download_trades_; }

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;
  void operator()(Trace<web::rest::Client::MessageBegin> const &) override;
  void operator()(Trace<web::rest::Client::Header> const &) override;
  void operator()(Trace<web::rest::Client::MessageEnd> const &) override;

  void operator()(ConnectionStatus);

  uint32_t download(OrderEntryState state);

  // listen-key

  void get_listen_key();
  void get_listen_key_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::ListenKeyAck> const &);

  // account-balance

  void get_account_balance();
  void get_account_balance_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::AccountBalanceAck> const &);

  // account-status

  void get_account_status();
  void get_account_status_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::AccountStatusAck> const &);

  // open-orders

  void get_open_orders();
  void get_open_orders_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::OpenOrdersAck> const &);

  // trades

  void get_trades();
  void get_trades_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::TradesAck> const &);

  // refresh-listen-key

  void refresh_listen_key();

  // order-place

  void order_place(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  void order_place_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::OrderPlaceAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // order-modify

  void order_modify(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void order_modify_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::OrderModifyAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // order-cancel

  void order_cancel(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void order_cancel_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::OrderCancelAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // open-orders-cancel-all

  void open_orders_cancel_all(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void open_orders_cancel_all_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);
  void operator()(Trace<json::OpenOrdersCancelAllAck> const &, std::string_view const &request_id);

  // countdown-cancel-all

  void countdown_cancel_all();
  void countdown_cancel_all_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::CountdownCancelAllAck> const &);

  // helpers

  void process_response(web::rest::Response const &, auto error_handler, auto success_handler);

  template <typename... Args>
  void operator()(Trace<server::oms::Response> const &, uint8_t user_id, uint64_t order_id, Args &&...);

  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

  void waf_limit_violation();

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
        listen_key,
        listen_key_ack,                                      //
        account_balance, account_balance_ack,                //
        account_status, account_status_ack,                  //
        open_orders, open_orders_ack,                        //
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
  // account
  Account &account_;
  // shared
  Shared &shared_;
  Request &request_;
  std::string listen_key_;
  // state
  std::chrono::nanoseconds listen_key_refresh_ = {};
  ConnectionStatus status_ = {};
  core::Download<OrderEntryState> download_;
  // experimental
  utils::unordered_set<std::string> open_orders_symbols_;
  std::chrono::nanoseconds next_auto_cancel_ = {};
  bool download_balance_ = false;
  bool download_account_ = false;
  bool download_orders_ = false;
  bool download_trades_ = false;
  std::vector<char> encode_buffer_;
  bool download_trades_is_first_ = true;
};

}  // namespace binance_futures
}  // namespace roq
