/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/io/context.hpp"

#include "roq/binance_futures/gateway/account.hpp"
#include "roq/binance_futures/gateway/config.hpp"
#include "roq/binance_futures/gateway/request.hpp"
#include "roq/binance_futures/gateway/settings.hpp"
#include "roq/binance_futures/gateway/shared.hpp"

#include "roq/binance_futures/gateway/drop_copy_classic.hpp"
#include "roq/binance_futures/gateway/drop_copy_portfolio.hpp"
#include "roq/binance_futures/gateway/market_data.hpp"
#include "roq/binance_futures/gateway/market_data_2.hpp"
#include "roq/binance_futures/gateway/order_entry_classic.hpp"
#include "roq/binance_futures/gateway/order_entry_portfolio.hpp"
#include "roq/binance_futures/gateway/rest.hpp"
#include "roq/binance_futures/gateway/rest_trade.hpp"
#include "roq/binance_futures/gateway/web_socket.hpp"

namespace roq {
namespace binance_futures {
namespace gateway {

struct Controller final : public server::Handler,
                          public Rest::Handler,
                          public MarketData::Handler,
                          public MarketData2::Handler,
                          public WebSocket::Handler,
                          public OrderEntryClassic::Handler,
                          public OrderEntryPortfolio::Handler,
                          public DropCopyClassic::Handler,
                          public DropCopyPortfolio::Handler,
                          public RestTrade::Handler {
  ROQ_PUBLIC static std::unique_ptr<server::Handler> create(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(Controller const &) = delete;

  virtual ~Controller() = default;

 protected:
  // server::Handler

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Control> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  void operator()(Event<Subscribe> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  uint16_t operator()(Event<MassQuote> const &) override;

  uint16_t operator()(Event<CancelQuotes> const &) override;

  void operator()(metrics::Writer &) const override;

  // streams

  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;
  void operator()(Trace<RateLimitsUpdate> const &) override;
  void operator()(Trace<ReferenceData> const &, bool is_last) override;
  void operator()(Trace<MarketStatus> const &, bool is_last) override;
  void operator()(Trace<TopOfBook> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeSummary> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate> const &, bool is_last) override;
  void operator()(Trace<TimeSeriesUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) override;
  void operator()(Trace<FundsUpdate> const &, bool is_last) override;
  void operator()(Trace<PositionUpdate> const &, bool is_last) override;

  void operator()(Rest::SymbolsUpdate &) override;

  void operator()(WebSocket::ListenKeyUpdate const &) override;
  void operator()(OrderEntryClassic::ListenKeyUpdate const &) override;
  void operator()(OrderEntryPortfolio::ListenKeyUpdate const &) override;

  // utilities

  template <typename T>
  void create_drop_copy_helper(auto &listen_key_update);

  void ensure_symbol_slices(size_t size);

  template <typename... Args>
  void dispatch(Args &&...);

  template <typename... Args>
  static void dispatch_helper(auto &self, Args &&...);

  Account &get_account(std::string_view const &account) const;
  Request &get_request(std::string_view const &account);
  OrderEntry &get_order_entry(std::string_view const &account);
  RestTrade &get_rest_trade(std::string_view const &account);

 private:
  server::Dispatcher &dispatcher_;
  // authentication
  utils::unordered_map<std::string, std::unique_ptr<Account>> const accounts_;
  // io
  io::Context &context_;
  // shared
  Shared shared_;
  utils::unordered_map<std::string, Request> requests_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  Rest rest_;
  std::vector<std::unique_ptr<MarketData>> market_data_a_, market_data_b_;
  std::vector<std::unique_ptr<MarketData2>> market_data_2_;
  utils::unordered_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
  utils::unordered_map<std::string, std::unique_ptr<DropCopy>> drop_copy_;
  utils::unordered_map<std::string, std::unique_ptr<RestTrade>> download_;
  // cache
  std::vector<MBPUpdate> bids_, asks_;
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
