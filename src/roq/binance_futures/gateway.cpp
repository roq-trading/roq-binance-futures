/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/gateway.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "roq/logging.hpp"

#include "roq/clock.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/binance_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto &config) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    auto obj = std::make_unique<Account>(config, account.name, account.margin_mode);
    result.try_emplace(static_cast<std::string_view>(account.name), std::move(obj));
  }
  return result;
}

template <typename R>
R create_requests(auto &config) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    result.try_emplace(static_cast<std::string_view>(account.name), Request{});
  }
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &request_by_account) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = request_by_account[account.name];
    switch (account.margin_mode) {
      using enum MarginMode;
      case UNDEFINED:
      case ISOLATED:
      case CROSS:
        if (shared.settings.ws_api) {
          auto obj = std::make_unique<WebSocket>(gateway, context, ++stream_id, account, shared, request);
          result.try_emplace(account.name, std::move(obj));
        } else {
          auto obj = std::make_unique<OrderEntryClassic>(gateway, context, ++stream_id, account, shared, request);
          result.try_emplace(account.name, std::move(obj));
        }
        break;
      case PORTFOLIO:
        auto obj = std::make_unique<OrderEntryPortfolio>(gateway, context, ++stream_id, account, shared, request);
        result.try_emplace(account.name, std::move(obj));
        break;
    }
  }
  return result;
}

template <typename R>
R create_drop_copy(auto &accounts) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    result.try_emplace(account.name, nullptr);
  }
  return result;
}

template <typename R>
R create_download(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &request_by_account) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = request_by_account[account.name];
    switch (account.margin_mode) {
      using enum MarginMode;
      case UNDEFINED:
      case ISOLATED:
      case CROSS:
        if (shared.settings.ws_api) {
          auto obj = std::make_unique<RestTrade>(gateway, context, ++stream_id, account, shared, request);
          result.try_emplace(account.name, std::move(obj));
        } else {
          // do nothing
        }
        break;
      case PORTFOLIO:
        // do nothing
        break;
    }
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Gateway::Gateway(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(config)}, context_{context}, shared_{dispatcher, settings},
      requests_{create_requests<decltype(requests_)>(config)}, rest_{*this, context_, ++stream_id_, shared_},
      order_entry_{create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_, requests_)},
      drop_copy_{create_drop_copy<decltype(drop_copy_)>(accounts_)},
      download_{create_download<decltype(download_)>(*this, context_, stream_id_, accounts_, shared_, requests_)} {
  if (settings.rest.cancel_on_disconnect) {
    log::fatal("Exchange does *NOT* support cancel on disconnect"sv);
  }
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  assert(std::empty(market_data_1_));
  assert(std::empty(market_data_2_));
  dispatch(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Gateway::operator()(Event<Control> const &event) {
  auto &[message_info, control] = event;
  switch (control.action) {
    using enum Action;
    case UNDEFINED:
      assert(false);
      break;
    case ENABLE:
      dispatcher_(State::ENABLED);
      break;
    case DISABLE:
      dispatcher_(State::DISABLED);
      break;
  }
}

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &) {
}

void Gateway::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<RateLimitsUpdate> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  dispatcher_(event, is_last, bids_, asks_, callback);
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TimeSeriesUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TradeUpdate> const &event, bool is_last, uint8_t user_id, std::string_view const &request_id) {
  dispatcher_(event, is_last, user_id, request_id);
}

void Gateway::operator()(Trace<FundsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<PositionUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Rest::SymbolsUpdate &symbols_update) {
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(size);
  for (auto &item : market_data_1_) {
    (*item).subscribe(start_from);
  }
  for (auto &item : market_data_2_) {
    (*item).subscribe(start_from);
  }
}

void Gateway::ensure_symbol_slices(size_t size) {
  auto helper = [&](auto &container, auto priority) {
    auto stream_id = ++stream_id_;
    auto index = std::size(container);
    log::info("Create MarketData (stream_id={}, priority={}, index={})"sv, stream_id, priority, index);
    auto market_data = std::make_unique<MarketData>(*this, context_, stream_id_, priority, shared_, index);
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*market_data, message_info, start);
    container.emplace_back(std::move(market_data));
  };
  while (std::size(market_data_1_) < size) {
    helper(market_data_1_, Priority::PRIMARY);
  }
  if (!shared_.settings.ws.enable_secondary) {
    return;  // note!
  }
  while (std::size(market_data_2_) < size) {
    helper(market_data_2_, Priority::SECONDARY);
  }
}

void Gateway::operator()(WebSocket::ListenKeyUpdate const &listen_key_update) {
  create_drop_copy_helper<DropCopyClassic>(listen_key_update);
}

void Gateway::operator()(OrderEntryClassic::ListenKeyUpdate const &listen_key_update) {
  create_drop_copy_helper<DropCopyClassic>(listen_key_update);
}

void Gateway::operator()(OrderEntryPortfolio::ListenKeyUpdate const &listen_key_update) {
  create_drop_copy_helper<DropCopyPortfolio>(listen_key_update);
}

template <typename T>
void Gateway::create_drop_copy_helper(auto &listen_key_update) {
  auto &account = listen_key_update.account;
  assert(!std::empty(account));
  auto iter = drop_copy_.find(account);
  if (iter == std::end(drop_copy_)) {
    log::fatal(R"(Unexpected: account="{}")"sv, account);
  } else if (!static_cast<bool>((*iter).second)) {
    log::info(R"(Create DropCopy (user-stream) for account="{}")"sv, account);
    auto drop_copy = std::make_unique<T>(*this, context_, ++stream_id_, get_account(account), shared_, get_request(account), listen_key_update.listen_key);
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*drop_copy, message_info, start);
    (*iter).second = std::move(drop_copy);
  }
}

uint16_t Gateway::operator()(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  auto &create_order = event.value;
  assert(!std::empty(create_order.account));
  return get_order_entry(create_order.account)(event, order, request_id);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  auto &modify_order = event.value;
  assert(!std::empty(modify_order.account));
  assert(modify_order.account == order.account);
  return get_order_entry(modify_order.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  auto &cancel_order = event.value;
  assert(!std::empty(cancel_order.account));
  assert(cancel_order.account == order.account);
  return get_order_entry(cancel_order.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  auto &cancel_all_orders = event.value;
  assert(!std::empty(cancel_all_orders.account));
  if (shared_.settings.ws_api) {
    return get_rest_trade(cancel_all_orders.account)(event, request_id);
  } else {
    return get_order_entry(cancel_all_orders.account)(event, request_id);
  }
}

uint16_t Gateway::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Gateway::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Gateway::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  helper(self.rest_);
  for (auto &item : self.market_data_1_) {
    helper(*item);
  }
  for (auto &item : self.market_data_2_) {
    helper(*item);
  }
  for (auto &[_, item] : self.order_entry_) {
    helper(*item);
  }
  for (auto &[_, item] : self.drop_copy_) {
    if (static_cast<bool>(item)) {
      helper(*item);
    }
  }
  for (auto &[_, item] : self.download_) {
    helper(*item);
  }
}

Account &Gateway::get_account(std::string_view const &account) const {
  auto iter = accounts_.find(account);
  if (iter != std::end(accounts_)) {
    return *(*iter).second;
  }
  log::fatal(R"(Unknown account="{}")"sv, account);
}

Request &Gateway::get_request(std::string_view const &account) {
  auto iter = requests_.find(account);
  if (iter != std::end(requests_)) {
    return (*iter).second;
  }
  log::fatal(R"(Unknown account="{}")"sv, account);
}

OrderEntry &Gateway::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter != std::end(order_entry_)) {
    return *(*iter).second;
  }
  throw RuntimeError{R"(Unknown account="{}")"sv, account};
}

RestTrade &Gateway::get_rest_trade(std::string_view const &account) {
  auto iter = download_.find(account);
  if (iter != std::end(download_)) {
    return *(*iter).second;
  }
  throw RuntimeError{R"(Unknown account="{}")"sv, account};
}

}  // namespace binance_futures
}  // namespace roq
