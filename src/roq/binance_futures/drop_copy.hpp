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

class DropCopy final : public core::web::ClientSocket::Handler,
                       public json::UserStreamParser::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<TradeUpdate> &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(const server::Trace<FundsUpdate> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<PositionUpdate> &, bool is_last) = 0;
  };

  DropCopy(
      Handler &,
      core::io::Context &,
      uint16_t stream_id,
      Security &,
      Shared &,
      Request &,
      const std::string_view &listen_key);

  DropCopy(DropCopy &&) = delete;
  DropCopy(const DropCopy &) = delete;

  bool ready() const;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(const core::web::ClientSocket::Connected &) override;
  void operator()(const core::web::ClientSocket::Disconnected &) override;
  void operator()(const core::web::ClientSocket::Ready &) override;
  void operator()(const core::web::ClientSocket::Close &) override;
  void operator()(const core::web::ClientSocket::Latency &) override;
  void operator()(const core::web::ClientSocket::Text &) override;
  void operator()(const core::web::ClientSocket::Binary &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void parse(const std::string_view &message);

  void operator()(const server::Trace<json::OrderTradeUpdate> &) override;
  void operator()(const server::Trace<json::AccountUpdate> &) override;
  void operator()(const server::Trace<json::MarginCall> &) override;

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
