/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/user_stream_parser.hpp"

#include "roq/logging.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/event_type.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// === CONSTANTS ===

namespace {
auto const EVENT_TYPE = "e"sv;
}

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}

auto try_dispatch(auto &handler, auto &message, auto &buffer_stack, auto event_type, auto &trace_info, auto allow_unknown_event_types) {
  switch (event_type) {
    using enum EventType::type_t;
    case UNKNOWN_INTERNAL:
      if (allow_unknown_event_types) {
        return false;
      }
      break;
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
      return true;
    case ACCOUNT_UPDATE:
      dispatch_helper<AccountUpdate>(handler, message, buffer_stack, trace_info);
      return true;
    case MARGIN_CALL:
      dispatch_helper<MarginCall>(handler, message, buffer_stack, trace_info);
      return true;
    case STRATEGY_UPDATE:
      dispatch_helper<StrategyUpdate>(handler, message, buffer_stack, trace_info);
      return true;
    case GRID_UPDATE:
      dispatch_helper<GridUpdate>(handler, message, buffer_stack, trace_info);
      return true;
    case ACCOUNT_CONFIG_UPDATE:
      dispatch_helper<AccountConfigUpdate>(handler, message, buffer_stack, trace_info);
      return true;
    case LISTEN_KEY_EXPIRED:
      // XXX FIXME TODO need parsing
      return true;
    case TRADE_LITE:
      dispatch_helper<TradeLite>(handler, message, buffer_stack, trace_info);
      return true;
    case BALANCE_UPDATE:
      dispatch_helper<BalanceUpdate>(handler, message, buffer_stack, trace_info);
      return true;
    case EXECUTION_REPORT:
      dispatch_helper<ExecutionReport2>(handler, message, buffer_stack, trace_info);
      return true;
    case LIABILITY_CHANGE:
      dispatch_helper<LiabilityChange>(handler, message, buffer_stack, trace_info);
      return true;
    case OUTBOUND_ACCOUNT_POSITION:
      dispatch_helper<OutboundAccountPosition>(handler, message, buffer_stack, trace_info);
      return true;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}
}  // namespace

// === IMPLEMENTATION ===

bool UserStreamParser::dispatch(
    UserStreamParser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    TraceInfo const &trace_info,
    bool allow_unknown_event_types) {
  core::json::Parser parser{message};
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    if (key != EVENT_TYPE) {
      continue;
    }
    EventType event_type{value};
    if (try_dispatch(handler, message, buffer_stack, event_type, trace_info, allow_unknown_event_types)) {
      return true;
    }
    break;
  }
  return false;
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
