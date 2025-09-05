/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/user_stream_parser.hpp"

#include "roq/logging.hpp"

#include "roq/core/json/parser.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool UserStreamParser::dispatch(
    UserStreamParser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    TraceInfo const &trace_info,
    bool continue_with_unknown_event_type) {
  core::json::Parser parser{message};
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    if (key != "e"sv) {
      continue;
    }
    EventType event_type{value};
    if (try_dispatch(handler, message, buffer_stack, event_type, trace_info, continue_with_unknown_event_type)) {
      return true;
    }
    break;
  }
  return false;
}

bool UserStreamParser::try_dispatch(
    UserStreamParser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    EventType event_type,
    TraceInfo const &trace_info,
    bool continue_with_unknown_event_type) {
  switch (event_type) {
    using enum EventType::type_t;
    case UNKNOWN_INTERNAL:
      if (!continue_with_unknown_event_type) {
        log::fatal("Unexpected"sv);
      }
      return false;
    case UNDEFINED_INTERNAL:
    case AGG_TRADE:
    case _24HR_MINI_TICKER:
    case BOOK_TICKER:
    case DEPTH_UPDATE:
    case MARK_PRICE_UPDATE:
    case KLINE:
      log::fatal("Unexpected"sv);
      break;
    case ORDER_TRADE_UPDATE:
      dispatch_helper<OrderTradeUpdate>(handler, message, buffer_stack, trace_info);
      break;
    case ACCOUNT_UPDATE:
      dispatch_helper<AccountUpdate>(handler, message, buffer_stack, trace_info);
      break;
    case MARGIN_CALL:
      dispatch_helper<MarginCall>(handler, message, buffer_stack, trace_info);
      break;
    case STRATEGY_UPDATE:
      dispatch_helper<StrategyUpdate>(handler, message, buffer_stack, trace_info);
      break;
    case GRID_UPDATE:
      dispatch_helper<GridUpdate>(handler, message, buffer_stack, trace_info);
      break;
    case ACCOUNT_CONFIG_UPDATE:
      dispatch_helper<AccountConfigUpdate>(handler, message, buffer_stack, trace_info);
      break;
    case LISTEN_KEY_EXPIRED: {
      // XXX need parsing
      break;
    }
    case TRADE_LITE:
      dispatch_helper<TradeLite>(handler, message, buffer_stack, trace_info);
      break;
    case BALANCE_UPDATE:
      dispatch_helper<BalanceUpdate>(handler, message, buffer_stack, trace_info);
      break;
    case EXECUTION_REPORT:
      dispatch_helper<ExecutionReport2>(handler, message, buffer_stack, trace_info);
      break;
    case LIABILITY_CHANGE:
      dispatch_helper<LiabilityChange>(handler, message, buffer_stack, trace_info);
      break;
  }
  return true;
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
