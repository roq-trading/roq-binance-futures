/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/download.hpp"

#include "roq/utils/container.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/gauge.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/cache/cancel_order_request.hpp"

#include "roq/binance_futures/order_entry.hpp"

#include "roq/binance_futures/account.hpp"
#include "roq/binance_futures/request.hpp"
#include "roq/binance_futures/shared.hpp"
#include "roq/binance_futures/web_socket_state.hpp"

#include "roq/binance_futures/json/wsapi_parser.hpp"

namespace roq {
namespace binance_futures {

struct WebSocket final : public OrderEntry, public web::socket::Client::Handler, public json::WSAPIParser::Handler {
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

  WebSocket(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &, Request &, bool master = true, std::string_view const &interface = {});

  bool ready() const { return status_ == ConnectionStatus::READY; }

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
  bool downloading() const { return download_balance_ || download_account_ | download_orders_; }

  void session_logon();

  void user_data_stream_start();
  void user_data_stream_ping(std::chrono::nanoseconds now);

  void account_balance();
  void account_status();
  void account_position();

  bool order_status();
  void order_status(std::string_view const &symbol);

  void open_orders_cancel_all(Event<CancelAllOrders> const &, std::string_view const &request_id);

  void order_place(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  void order_modify(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void order_cancel(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);

  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(WebSocketState state);

  void parse(std::string_view const &message);

  // json::WSAPIParser::Handler

  void operator()(Trace<json::WSAPIError> const &) override;
  void operator()(Trace<json::WSAPISessionLogon> const &) override;
  void operator()(Trace<json::WSAPIListenKey> const &) override;
  void operator()(Trace<json::WSAPIAccountBalance> const &) override;
  void operator()(Trace<json::WSAPIAccountStatus> const &) override;
  void operator()(Trace<json::WSAPIAccountPosition> const &) override;
  void operator()(Trace<json::WSAPIOpenOrders> const &) override;
  void operator()(Trace<json::WSAPITrades> const &) override;
  void operator()(Trace<json::WSAPIOpenOrdersCancelAll> const &, json::WSAPIRequest const &) override;
  void operator()(Trace<json::WSAPIOrderPlace> const &, json::WSAPIRequest const &) override;
  void operator()(Trace<json::WSAPIOrderModify> const &, json::WSAPIRequest const &) override;
  void operator()(Trace<json::WSAPIOrderCancel> const &, json::WSAPIRequest const &) override;

  // helpers

  void update_rate_limits(auto &event);

  template <typename... Args>
  void operator()(Trace<server::oms::Response> const &, uint8_t user_id, uint64_t order_id, Args &&...args);

  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  bool master_;
  // web socket
  std::unique_ptr<web::socket::Client> connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse, error,                    //
        session_logon, session_logon_ack,                    //
        user_data_stream_start, user_data_stream_start_ack,  //
        user_data_stream_ping, user_data_stream_ping_ack,    //
        account_balance, account_balance_ack,                //
        account_status, account_status_ack,                  //
        account_position, account_position_ack,              //
        order_status, order_status_ack,                      //
        open_orders_cancel_all,                              //
        open_orders_cancel_all_ack,                          //
        order_place, order_place_ack,                        //
        order_modify, order_modify_ack,                      //
        order_cancel, order_cancel_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  struct {
    utils::metrics::Gauge request_weight_1m, create_order_10s, create_order_1d;
  } rate_limiter_;
  // authentication
  Account &account_;
  // shared
  Shared &shared_;
  Request &request_;
  // experimental
  uint32_t request_id_;
  std::string listen_key_;
  std::chrono::nanoseconds listen_key_refresh_ = {};
  bool download_balance_ = false;
  bool download_account_ = false;
  bool download_orders_ = false;
  utils::unordered_set<std::string> open_orders_symbols_;
  std::vector<char> request_encode_buffer_;
  std::vector<char> encode_buffer_;
  // state
  bool ready_ = false;
  ConnectionStatus status_ = {};
  core::Download<WebSocketState> download_;
  [[maybe_unused]] bool download_trades_is_first_ = true;
};

}  // namespace binance_futures
}  // namespace roq
