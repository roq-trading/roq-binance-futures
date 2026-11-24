/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/gateway/controller.hpp"

#include "roq/logging.hpp"

#include "roq/server/oms/exceptions.hpp"

// #include "roq/binance_futures/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace gateway {

// === CONSTANTS ===

namespace {
uint8_t const FAPI = 0x1;
uint8_t const DAPI = 0x2;
}  // namespace

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto &config) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    auto obj = std::make_unique<Account>(config, account.name, account.margin_mode);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

template <typename R>
R create_requests(auto &config, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    auto obj = std::make_unique<Request>(shared);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

Request &get_request(auto &requests, auto &account) {
  auto iter = requests.find(account);
  if (iter == std::end(requests)) {
    log::fatal("Unexpected"sv);
  }
  return *(*iter).second;
}

auto create_rest(auto &gateway, auto &context, auto &stream_id, auto &shared) {
  return std::make_unique<Rest>(gateway, context, ++stream_id, shared);
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &requests) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = get_request(requests, account.name);
    switch (account.margin_mode) {
      using enum MarginMode;
      case UNDEFINED:
      case ISOLATED:
      case CROSS:
        if (shared.settings.misc.disable_ws_api) {
          auto obj = std::make_unique<OrderEntryClassic>(gateway, context, ++stream_id, account, shared, request);
          result.try_emplace(account.name, std::move(obj));
        } else {
          auto obj = std::make_unique<WebSocket>(gateway, context, ++stream_id, account, shared, request);
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
R create_download(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &requests) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = get_request(requests, account.name);
    switch (account.margin_mode) {
      using enum MarginMode;
      case UNDEFINED:
      case ISOLATED:
      case CROSS:
        if (shared.settings.misc.disable_ws_api) {
          // do nothing
        } else {
          auto obj = std::make_unique<RestTrade>(gateway, context, ++stream_id, account, shared, request);
          result.try_emplace(account.name, std::move(obj));
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

std::unique_ptr<server::Handler> Controller::create(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context) {
  return std::make_unique<Controller>(dispatcher, settings, config, context);
}

uint8_t Controller::parse_api(Settings const &settings) {
  if (settings.api == "fapi"sv) {
    return FAPI;
  }
  if (settings.api == "dapi"sv) {
    return DAPI;
  }
  log::fatal(R"(Unexpected: api="{}")"sv, settings.api);
}

Controller::Controller(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, context_{context}, shared_{dispatcher, settings}, accounts_{create_accounts<decltype(accounts_)>(config)},
      requests_{create_requests<decltype(requests_)>(config, shared_)}, rest_{create_rest(*this, context_, stream_id_, shared_)},
      order_entry_{create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_, requests_)},
      drop_copy_{create_drop_copy<decltype(drop_copy_)>(accounts_)},
      rest_trade_{create_download<decltype(rest_trade_)>(*this, context_, stream_id_, accounts_, shared_, requests_)} {
  if (settings.rest.cancel_on_disconnect) {
    log::fatal("Exchange does *NOT* support cancel on disconnect"sv);
  }
}

// server::Handler

void Controller::operator()(Trace<Start> const &event) {
  log::info("Starting..."sv);
  assert(std::empty(market_data_a_));
  assert(std::empty(market_data_b_));
  assert(std::empty(market_data_2_));
  dispatch(event);
}

void Controller::operator()(Trace<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Controller::operator()(Trace<Timer> const &event) {
  dispatch(event);
}

void Controller::operator()(Trace<Connected> const &, [[maybe_unused]] uint8_t user_id) {
}

void Controller::operator()(Trace<Disconnected> const &, [[maybe_unused]] uint8_t user_id) {
}

void Controller::operator()(Trace<Subscribe> const &event) {
  auto &[trace_info, subscribe] = event;
  std::vector<Symbol> symbols;
  for (auto &item : subscribe.symbols) {
    if (shared_.all_symbols.emplace(item).second) {
      symbols.emplace_back(item);
    } else {
      log::warn(R"(*** DUPLICATE SUBSCRIPTION *** (symbol="{}")"sv, item);
    }
  }
  auto symbols_update = SymbolsUpdate{
      .symbols = symbols,
  };
  create_trace_and_dispatch_2(trace_info, symbols_update);
}

void Controller::operator()(Trace<Control> const &event) {
  auto &[trace_info, control] = event;
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

void Controller::operator()(Trace<server::oms::OrderActionBatchBegin> const &event) {
  dispatch_order_action(event);
}

void Controller::operator()(Trace<server::oms::OrderActionBatchCommit> const &event) {
  dispatch_order_action(event);
}

void Controller::operator()(Trace<server::oms::OrderActionBatchRollback> const &event) {
  dispatch_order_action(event);
}

uint16_t Controller::operator()(
    Trace<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  return dispatch_order_action(event, order, ref_data, request_id);
}

uint16_t Controller::operator()(
    Trace<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  return dispatch_order_action(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(
    Trace<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  return dispatch_order_action(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(Trace<CancelAllOrders> const &event, std::string_view const &request_id) {
  if (shared_.settings.misc.disable_ws_api) {
    return dispatch_order_action(event, request_id);
  } else {
    return dispatch_rest_trade(event, request_id);
  }
}

uint16_t Controller::operator()(Trace<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Trace<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Controller::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

// Rest::Handler

void Controller::operator()(Trace<SymbolsUpdate> const &event) {
  auto &[trace_info, symbols_update] = event;
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(trace_info, size);
  for (auto &item : market_data_a_) {
    dynamic_cast<server::MarketDataStream &>(*item).subscribe(start_from);
  }
  for (auto &item : market_data_b_) {
    dynamic_cast<server::MarketDataStream &>(*item).subscribe(start_from);
  }
  for (auto &item : market_data_2_) {
    dynamic_cast<server::MarketDataStream &>(*item).subscribe(start_from);
  }
}

void Controller::operator()(Trace<AssetsUpdate> const &event) {
  auto &[trace_info, assets_update] = event;
  if (!std::empty(market_data_2_)) {
    (*market_data_2_[0])(assets_update.assets);
  }
}

// WebSocket::Handler

void Controller::operator()(Trace<WebSocket::ListenKeyUpdate> const &event) {
  create_or_update_drop_copy<DropCopyClassic>(event);
}

// OrderEntryClassic::Handler

void Controller::operator()(Trace<OrderEntryClassic::ListenKeyUpdate> const &event) {
  create_or_update_drop_copy<DropCopyClassic>(event);
}

// OrderEntryPortfolio::Handler

void Controller::operator()(Trace<OrderEntryPortfolio::ListenKeyUpdate> const &event) {
  create_or_update_drop_copy<DropCopyPortfolio>(event);
}

// utilities

template <typename T>
void Controller::create_or_update_drop_copy(auto &event) {
  auto &[trace_info, listen_key_update] = event;
  auto &account = listen_key_update.account;
  assert(!std::empty(account));
  auto iter = drop_copy_.find(account);
  if (iter == std::end(drop_copy_)) [[unlikely]] {
    log::fatal(R"(Unexpected: account="{}")"sv, account);
  } else if (!static_cast<bool>((*iter).second)) {
    log::info(R"(Create DropCopy (user-stream) for account="{}" using listen_key="{}")"sv, account, listen_key_update.listen_key);
    auto obj = std::make_unique<T>(*this, context_, ++stream_id_, get_account(account), shared_, get_request(account), listen_key_update.listen_key);
    Start start;
    create_trace_and_dispatch(dynamic_cast<server::Stream &>(*obj), trace_info, start);
    (*iter).second = std::move(obj);
  } else {
    (*(*iter).second).update_listen_key(trace_info, listen_key_update.listen_key);
  }
}

void Controller::ensure_symbol_slices(TraceInfo const &trace_info, size_t size) {
  auto helper = [&](auto &container, auto priority) {
    auto stream_id = ++stream_id_;
    auto index = std::size(container);
    log::info("Create MarketData (stream_id={}, priority={}, index={})"sv, stream_id, priority, index);
    auto obj = std::make_unique<MarketData>(*this, context_, stream_id_, priority, shared_, index);
    Start start;
    create_trace_and_dispatch(dynamic_cast<server::Stream &>(*obj), trace_info, start);
    container.emplace_back(std::move(obj));
  };
  while (std::size(market_data_a_) < size) {
    helper(market_data_a_, Priority::PRIMARY);
  }
  if (shared_.settings.ws.enable_secondary) {
    while (std::size(market_data_b_) < size) {
      helper(market_data_b_, Priority::SECONDARY);
    }
  }
  auto helper_2 = [&](auto &container) {
    auto stream_id = ++stream_id_;
    auto index = std::size(container);
    log::info("Create MarketData2 (stream_id={}, index={})"sv, stream_id, index);
    auto obj = std::make_unique<MarketData2>(*this, context_, stream_id_, shared_, index);
    Start start;
    create_trace_and_dispatch(dynamic_cast<server::Stream &>(*obj), trace_info, start);
    container.emplace_back(std::move(obj));
  };
  while (std::size(market_data_2_) < size) {
    helper_2(market_data_2_);
  }
}

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Controller::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) {
    if constexpr (std::is_const_v<decltype(self)>) {
      dynamic_cast<server::Stream const &>(target)(std::forward<Args>(args)...);
    } else {
      dynamic_cast<server::Stream &>(target)(std::forward<Args>(args)...);
    }
  };
  helper(*self.rest_);
  for (auto &item : self.market_data_a_) {
    helper(*item);
  }
  for (auto &item : self.market_data_b_) {
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
  for (auto &[_, item] : self.rest_trade_) {
    helper(*item);
  }
}

Account &Controller::get_account(std::string_view const &account) const {
  auto iter = accounts_.find(account);
  if (iter != std::end(accounts_)) {
    return *(*iter).second;
  }
  log::fatal(R"(Unknown account="{}")"sv, account);
}

Request &Controller::get_request(std::string_view const &account) {
  auto iter = requests_.find(account);
  if (iter != std::end(requests_)) {
    return *(*iter).second;
  }
  log::fatal(R"(Unknown account="{}")"sv, account);
}

template <typename T, typename... Args>
uint16_t Controller::dispatch_order_action(Trace<T> const &event, Args &&...args) {
  auto &[trace_info, value] = event;
  auto iter = order_entry_.find(value.account);
  if (iter == std::end(order_entry_)) [[unlikely]] {
    throw RuntimeError{R"(Unknown account="{}")"sv, value.account};
  }
  auto &stream = dynamic_cast<server::OrderActionStream &>(*(*iter).second);
  stream(event, args...);
  return stream.stream_id;
}

template <typename T, typename... Args>
uint16_t Controller::dispatch_rest_trade(Trace<T> const &event, Args &&...args) {
  auto &[trace_info, value] = event;
  auto iter = rest_trade_.find(value.account);
  if (iter == std::end(rest_trade_)) [[unlikely]] {
    throw RuntimeError{R"(Unknown account="{}")"sv, value.account};
  }
  auto &stream = dynamic_cast<server::OrderActionStream &>(*(*iter).second);
  stream(event, args...);
  return stream.stream_id;
}

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
