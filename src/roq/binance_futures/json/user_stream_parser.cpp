/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/binance_futures/json/user_stream_parser.hpp"

#include "roq/logging.hpp"

#include "roq/core/json/parser.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

void UserStreamParser::dispatch(
    UserStreamParser::Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::object_t>(root)) {
    if (key.compare("e"sv) != 0)
      continue;
    EventType event_type(value);
    if (try_dispatch(handler, message, buffer, event_type, trace_info))
      return;
    break;
  }
  log::warn(R"(message="{}")"sv, message);
  log::fatal("Unexpected"sv);
}

bool UserStreamParser::try_dispatch(
    UserStreamParser::Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    EventType event_type,
    const TraceInfo &trace_info) {
  switch (event_type) {
    using enum EventType::type_t;
    case UNDEFINED:
    case UNKNOWN:
    case AGG_TRADE:
    case _24HR_MINI_TICKER:
    case BOOK_TICKER:
    case DEPTH_UPDATE:
    case MARK_PRICE_UPDATE:
      log::fatal("Unexpected"sv);
      break;
    case ORDER_TRADE_UPDATE: {
      auto order_trade_update = core::json::Parser::create<OrderTradeUpdate>(message, buffer);
      Trace event(trace_info, order_trade_update);
      handler(event);
      break;
    }
    case ACCOUNT_UPDATE: {
      auto account_update = core::json::Parser::create<AccountUpdate>(message, buffer);
      Trace event(trace_info, account_update);
      handler(event);
      break;
    }
    case MARGIN_CALL: {
      auto margin_call = core::json::Parser::create<MarginCall>(message, buffer);
      Trace event(trace_info, margin_call);
      handler(event);
      break;
    }
    default:
      return false;
  }
  return true;
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
