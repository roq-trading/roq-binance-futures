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

#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/protocol/json/asset_index_ack.hpp"
#include "roq/binance_futures/protocol/json/depth_ack.hpp"
#include "roq/binance_futures/protocol/json/exchange_info_ack.hpp"
#include "roq/binance_futures/protocol/json/kline_ack.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct Rest final : public Base<Rest>, public server::Stream, public web::rest::Client::Handler {
  struct SymbolsUpdate final {
    std::span<Symbol const> symbols;
  };

  struct AssetsUpdate final {
    std::span<std::string_view> assets;
  };

  struct Handler {
    virtual void operator()(SymbolsUpdate &) = 0;
    virtual void operator()(AssetsUpdate &) = 0;
  };

  Rest(Handler &, io::Context &, uint16_t stream_id, Shared &);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

 protected:
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
    EXCHANGE_INFO,
    ASSET_INDEX,
    DONE,
  };

  int32_t download(Trace<State> const &);

  // exchange-info

  void get_exchange_info();
  void get_exchange_info_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::ExchangeInfoAck> const &);

  // asset-index

  void get_asset_index();
  void get_asset_index_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::AssetIndexAck> const &);

  // depth

  void get_depth(std::string_view const &symbol);
  void get_depth_ack(Trace<web::rest::Response> const &, std::string_view const &symbol);
  void operator()(Trace<protocol::json::DepthAck> const &, std::string_view const &symbol);

  // kline

  void get_kline(std::string_view const &symbol);
  void get_kline_ack(Trace<web::rest::Response> const &, std::string_view const &symbol);
  void operator()(Trace<protocol::json::KlineAck> const &, std::string_view const &symbol);

  // helpers

  void check_request_queue(std::chrono::nanoseconds now);

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

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
    utils::metrics::Profile exchange_info, exchange_info_ack, asset_index, asset_index_ack, depth, depth_ack, kline, kline_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  struct {
    utils::metrics::Gauge request_weight_1m;
  } rate_limiter_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download2<State> download_;
  // EXPERIMENTAL
  utils::unordered_set<std::string> assets_;
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
