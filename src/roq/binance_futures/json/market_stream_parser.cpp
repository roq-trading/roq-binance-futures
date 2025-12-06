/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/market_stream_parser.hpp"

#include "roq/logging.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/binance_futures/json/field.hpp"
#include "roq/binance_futures/json/stream.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// === HELPERS ===

namespace {
// note! for legacy reasons, the dispatcher below uses a mixture of (json) value or (string) message
template <typename T, typename... Args>
void dispatch_helper(auto &handler, auto &value_or_message, auto &buffer_stack, auto &trace_info, Args &&...args) {
  T obj{value_or_message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj, std::forward<Args>(args)...);
}
}  // namespace

// === IMPLEMENTATION ===

bool MarketStreamParser::dispatch(
    MarketStreamParser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    TraceInfo const &trace_info,
    bool allow_unknown_event_types) {
  int64_t id = -1;
  for (int i = 0; i < 2; ++i) {
    core::json::Parser parser{message};
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::Object>(root)) {
      Field field{key};
      switch (field) {
        using enum Field::type_t;
        case UNDEFINED_INTERNAL:
          log::fatal("Unexpected"sv);
          break;
        case UNKNOWN_INTERNAL:
#ifndef NDEBUG
          log::fatal(R"(Unknown key="{}")"sv, key);
#endif
          break;
        case ID:
          id = std::get<decltype(id)>(value);
          break;
        case ERROR:
          if (id >= 0) {
            dispatch_helper<Error>(handler, value, buffer_stack, trace_info, id);
            return true;
          }
          break;
        case RESULT:
          if (id >= 0) {
            dispatch_helper<Result>(handler, value, buffer_stack, trace_info, id);
            return true;
          }
          break;
        case STREAM:
          break;
        case DATA:
          // XXX FIXME TODO why the need for recursive here ???
          // 2025-12-06 -- maybe it was an old format where the event was wrapper
          dispatch(handler, core::json::get<std::string_view>(value), buffer_stack, trace_info, allow_unknown_event_types);
          return true;
        case EVENT_TYPE: {
          EventType event_type{value};
          switch (event_type) {
            using enum EventType::type_t;
            case UNDEFINED_INTERNAL:
              break;
            case UNKNOWN_INTERNAL:
              if (!allow_unknown_event_types) {
                log::fatal("Unexpected"sv);
              }
              return false;
            case AGG_TRADE:
              dispatch_helper<AggTrade>(handler, message, buffer_stack, trace_info);
              return true;
            case _24HR_MINI_TICKER:
              dispatch_helper<MiniTicker>(handler, message, buffer_stack, trace_info);
              return true;
            case BOOK_TICKER:
              dispatch_helper<BookTicker>(handler, message, buffer_stack, trace_info);
              return true;
            case DEPTH_UPDATE:
              dispatch_helper<DepthUpdate>(handler, message, buffer_stack, trace_info);
              return true;
            case MARK_PRICE_UPDATE:
              dispatch_helper<MarkPriceUpdate>(handler, message, buffer_stack, trace_info);
              return true;
            case KLINE:
              dispatch_helper<Kline>(handler, message, buffer_stack, trace_info);
              return true;
            case ORDER_TRADE_UPDATE:
            case ACCOUNT_UPDATE:
            case MARGIN_CALL:
            case GRID_UPDATE:
            case STRATEGY_UPDATE:
            case ACCOUNT_CONFIG_UPDATE:
              log::fatal("Unexpected"sv);
              break;
            case LISTEN_KEY_EXPIRED:
              // XXX FIXME TODO need parsing
              return true;
            case TRADE_LITE:
            case BALANCE_UPDATE:
            case EXECUTION_REPORT:
            case LIABILITY_CHANGE:
            case OUTBOUND_ACCOUNT_POSITION: {
              log::fatal("Unexpected"sv);
              break;
            }
          }
          break;
        }
        case ORDER_BOOK_UPDATE_ID:
          break;
      }
    }
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
