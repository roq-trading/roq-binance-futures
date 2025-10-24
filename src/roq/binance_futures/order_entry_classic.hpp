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

#include "roq/binance_futures/json/account.hpp"
#include "roq/binance_futures/json/auto_cancel_all_open_orders.hpp"
#include "roq/binance_futures/json/balance.hpp"
#include "roq/binance_futures/json/cancel_all_open_orders.hpp"
#include "roq/binance_futures/json/cancel_order.hpp"
#include "roq/binance_futures/json/listen_key.hpp"
#include "roq/binance_futures/json/modify_order.hpp"
#include "roq/binance_futures/json/new_order.hpp"
#include "roq/binance_futures/json/open_orders.hpp"
#include "roq/binance_futures/json/trades.hpp"

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

  void get_listen_key();
  void get_listen_key_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::ListenKey> const &);

  void get_balance();
  void get_balance_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::Balance> const &);

  void get_account();
  void get_account_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::Account> const &);

  void get_open_orders();
  void get_open_orders_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::OpenOrders> const &);

  void get_trades();
  void get_trades_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::Trades> const &);

  void refresh_listen_key();

  void new_order(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  void new_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::NewOrder> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  void modify_order(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void modify_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::ModifyOrder> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  void cancel_order(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::CancelOrder> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  void cancel_all_open_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_open_orders_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);
  void operator()(Trace<json::CancelAllOpenOrders> const &, std::string_view const &request_id);

  void auto_cancel_all_open_orders();
  void auto_cancel_all_open_orders_ack(Trace<web::rest::Response> const &);
  void operator()(Trace<json::AutoCancelAllOpenOrders> const &);

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
        balance, balance_ack,                                //
        account, account_ack,                                //
        open_orders, open_orders_ack,                        //
        trades, trades_ack,                                  //
        new_order, new_order_ack,                            //
        modify_order, modify_order_ack,                      //
        cancel_order, cancel_order_ack,                      //
        cancel_all_open_orders, cancel_all_open_orders_ack,  //
        auto_cancel_all_open_orders, auto_cancel_all_open_orders_ack;
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
