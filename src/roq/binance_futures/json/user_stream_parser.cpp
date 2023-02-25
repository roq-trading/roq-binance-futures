/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/json/user_stream_parser.hpp"

#include "roq/logging.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

void UserStreamParser::dispatch(
    UserStreamParser::Handler &handler,
    std::string_view const &message,
    core::json::Buffer &buffer,
    TraceInfo const &trace_info) {
  core::json::Parser parser{message};
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    if (key.compare("e"sv) != 0)
      continue;
    EventType event_type{value};
    if (try_dispatch(handler, message, buffer, event_type, trace_info))
      return;
    break;
  }
  log::warn(R"(message="{}")"sv, message);
  log::fatal("Unexpected"sv);
}

bool UserStreamParser::try_dispatch(
    UserStreamParser::Handler &handler,
    std::string_view const &message,
    core::json::Buffer &buffer,
    EventType event_type,
    TraceInfo const &trace_info) {
  switch (event_type) {
    using enum EventType::type_t;
    case UNDEFINED:
    case AGG_TRADE:
    case _24HR_MINI_TICKER:
    case BOOK_TICKER:
    case DEPTH_UPDATE:
    case MARK_PRICE_UPDATE:
      log::fatal("Unexpected"sv);
      break;
    case ORDER_TRADE_UPDATE: {
      auto const order_trade_update = core::json::Parser::create<OrderTradeUpdate>(message, buffer);
      Trace event{trace_info, order_trade_update};
      handler(event);
      break;
    }
    case ACCOUNT_UPDATE: {
      auto const account_update = core::json::Parser::create<AccountUpdate>(message, buffer);
      Trace event{trace_info, account_update};
      handler(event);
      break;
    }
    case MARGIN_CALL: {
      auto const margin_call = core::json::Parser::create<MarginCall>(message, buffer);
      Trace event{trace_info, margin_call};
      handler(event);
      break;
    }
    case GRID_UPDATE: {
      log::warn(R"(Unknown: "{}")"sv, message);
      break;
    }
    case UNKNOWN:
      if (!flags::Flags::continue_with_unknown_event_type())
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
