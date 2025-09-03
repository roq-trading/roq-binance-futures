/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "roq/core/timer_queue.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/binance_futures/shared.hpp"

#include "roq/binance_futures/json/market_stream_parser.hpp"

namespace roq {
namespace binance_futures {

struct MarketData final : public web::socket::Client::Handler, public json::MarketStreamParser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<TradeSummary> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<TimeSeriesUpdate> const &, bool is_last) = 0;
  };

  MarketData(Handler &, io::Context &, uint16_t stream_id, Priority, Shared &, size_t index);

  MarketData(MarketData const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  void subscribe(size_t start_from = 0);

  void check_subscribe_queue(std::chrono::nanoseconds now);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::span<Symbol const> const &symbols, std::string_view const &channel);

  void parse(std::string_view const &message);

  // response
  void operator()(Trace<json::Error> const &, int32_t id) override;
  void operator()(Trace<json::Result> const &, int32_t id) override;

  // update
  void operator()(Trace<json::AggTrade> const &) override;
  void operator()(Trace<json::MarkPriceUpdate> const &) override;
  void operator()(Trace<json::MiniTicker> const &) override;
  void operator()(Trace<json::BookTicker> const &) override;
  void operator()(Trace<json::DepthUpdate> const &) override;
  void operator()(Trace<json::Kline> const &) override;

  Handler &handler_;
  // config
  uint16_t const stream_id_;
  Priority const priority_;
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
    utils::metrics::Profile parse, error, result, agg_trade, mark_price_update, mini_ticker, book_ticker, depth_update, kline;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  // queue
  core::TimerQueue<std::string> subscribe_queue_;
};

}  // namespace binance_futures
}  // namespace roq
