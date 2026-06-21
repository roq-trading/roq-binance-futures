/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>

#include "roq/core/timer_queue.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/protocol/json/market_stream_parser.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct MarketData2 final : public web::socket::Client::Handler, public protocol::json::MarketStreamParser::Handler {
  struct Handler {};

  MarketData2(Handler &, io::Context &, uint16_t stream_id, Shared &, size_t index);

  MarketData2(MarketData2 const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  void subscribe(size_t start_from = 0);

 protected:
  // helpers
  void check_subscribe_queue(std::chrono::nanoseconds now);

  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::span<Symbol const> const &symbols, std::string_view const &channel, std::chrono::nanoseconds const freq = {});

  void parse(std::string_view const &message);

  // protocol::json::MarketStreamParser::Handler

  // response
  void operator()(Trace<protocol::json::Error> const &, int32_t id) override;
  void operator()(Trace<protocol::json::Result> const &, int32_t id) override;

  // update
  void operator()(Trace<protocol::json::Trade2> const &) override;
  void operator()(Trace<protocol::json::AggTrade> const &) override;
  void operator()(Trace<protocol::json::MarkPriceUpdate> const &) override;
  void operator()(Trace<protocol::json::MiniTicker> const &) override;
  void operator()(Trace<protocol::json::BookTicker> const &) override;
  void operator()(Trace<protocol::json::DepthUpdate> const &) override;
  void operator()(Trace<protocol::json::Kline> const &) override;

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  size_t const index_;
  // web socket
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // session
  uint64_t request_id_ = {};
  // metrics
  struct {
    utils::metrics::Counter disconnect, total_bytes_received;
  } counter_;
  struct {
    utils::metrics::Profile parse, error, result, agg_trade, mark_price_update, mini_ticker, kline;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  // queue
  core::TimerQueue<std::string> subscribe_queue_;
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
