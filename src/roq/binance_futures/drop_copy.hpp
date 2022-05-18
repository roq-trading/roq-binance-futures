/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/binance_futures/drop_copy_state.hpp"
#include "roq/binance_futures/request.hpp"
#include "roq/binance_futures/security.hpp"
#include "roq/binance_futures/shared.hpp"

#include "roq/binance_futures/json/user_stream_parser.hpp"

namespace roq {
namespace binance_futures {

class DropCopy final : public core::web::ClientSocket::Handler, public json::UserStreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(Trace<FundsUpdate const> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate const> const &, bool is_last) = 0;
  };

  DropCopy(
      Handler &,
      core::io::Context &,
      uint16_t stream_id,
      Security &,
      Shared &,
      Request &,
      std::string_view const &listen_key);

  DropCopy(DropCopy &&) = delete;
  DropCopy(DropCopy const &) = delete;

  bool ready() const;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(core::web::ClientSocket::Connected const &) override;
  void operator()(core::web::ClientSocket::Disconnected const &) override;
  void operator()(core::web::ClientSocket::Ready const &) override;
  void operator()(core::web::ClientSocket::Close const &) override;
  void operator()(core::web::ClientSocket::Latency const &) override;
  void operator()(core::web::ClientSocket::Text const &) override;
  void operator()(core::web::ClientSocket::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void parse(std::string_view const &message);

  void operator()(Trace<json::OrderTradeUpdate const> const &) override;
  void operator()(Trace<json::AccountUpdate const> const &) override;
  void operator()(Trace<json::MarginCall const> const &) override;

  void request_balance();
  void check_response_balance();

  void request_account();
  void check_response_account();

  void request_orders();
  void check_response_orders();

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  core::web::ClientSocket connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, order_trade_update, account_update, margin_call;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // security
  Security &security_;
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
