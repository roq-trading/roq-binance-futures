/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/user_stream_parser.hpp"

#include "roq/logging.hpp"

#include "roq/core/json/parser.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

bool UserStreamParser::dispatch(
    UserStreamParser::Handler &handler,
    std::string_view const &message,
    std::span<std::byte> const &buffer,
    TraceInfo const &trace_info,
    bool continue_with_unknown_event_type) {
  core::json::Parser parser{message};
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    if (key.compare("e"sv) != 0)
      continue;
    EventType event_type{value};
    if (try_dispatch(handler, message, buffer, event_type, trace_info, continue_with_unknown_event_type))
      return true;
    break;
  }
  return false;
}

bool UserStreamParser::try_dispatch(
    UserStreamParser::Handler &handler,
    std::string_view const &message,
    std::span<std::byte> const &buffer,
    EventType event_type,
    TraceInfo const &trace_info,
    bool continue_with_unknown_event_type) {
  switch (event_type) {
    using enum EventType::type_t;
    case UNDEFINED__:
    case AGG_TRADE:
    case _24HR_MINI_TICKER:
    case BOOK_TICKER:
    case DEPTH_UPDATE:
    case MARK_PRICE_UPDATE:
      log::fatal("Unexpected"sv);
      break;
    case ORDER_TRADE_UPDATE: {
      OrderTradeUpdate order_trade_update{message, buffer};
      Trace event{trace_info, order_trade_update};
      handler(event);
      break;
    }
    case ACCOUNT_UPDATE: {
      AccountUpdate account_update{message, buffer};
      Trace event{trace_info, account_update};
      handler(event);
      break;
    }
    case MARGIN_CALL: {
      MarginCall margin_call{message, buffer};
      Trace event{trace_info, margin_call};
      handler(event);
      break;
    }
    case STRATEGY_UPDATE: {
      StrategyUpdate strategy_update{message, buffer};
      Trace event{trace_info, strategy_update};
      handler(event);
      break;
    }
    case GRID_UPDATE: {
      GridUpdate grid_update{message, buffer};
      Trace event{trace_info, grid_update};
      handler(event);
      break;
    }
    case ACCOUNT_CONFIG_UPDATE: {
      AccountConfigUpdate account_config_update{message, buffer};
      Trace event{trace_info, account_config_update};
      handler(event);
      break;
    }
    case LISTEN_KEY_EXPIRED: {
      // XXX need parsing
      break;
    }
    case TRADE_LITE: {
      // XXX need parsing
      break;
    }
    case UNKNOWN__:
      if (!continue_with_unknown_event_type)
        log::fatal("Unexpected"sv);
      return false;
    default:
      return false;
  }
  return true;
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
