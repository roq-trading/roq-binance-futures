/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/json/market_stream_parser.hpp"

#include <cctype>
#include <string>

#include "roq/compat.hpp"

#include "roq/binance_futures/json/field.hpp"
#include "roq/binance_futures/json/stream.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

namespace {
template <typename T>
void dispatch_helper(
    MarketStreamParser::Handler &handler,
    std::string_view const &message,
    std::span<std::byte> const &buffer,
    TraceInfo const &trace_info) {
  T value{message, buffer};
  create_trace_and_dispatch(handler, trace_info, value);
}
}  // namespace

void MarketStreamParser::dispatch(
    MarketStreamParser::Handler &handler,
    std::string_view const &message,
    std::span<std::byte> const &buffer,
    TraceInfo const &trace_info,
    bool continue_with_unknown_event_type) {
  int64_t id = -1;
  std::string symbol;  // allocating because we need uppercase
  // auto stream = Stream::UNDEFINED;
  bool dispatched = false;
  for (int i = 0; i < 2 && !dispatched; ++i) {
    core::json::Parser parser(message);
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::Object>(root)) {
      auto field = Field(key);
      switch (field) {
        using enum Field::type_t;
        case UNDEFINED__:
          log::fatal("Unexpected"sv);
          break;
        case UNKNOWN__:
#ifndef NDEBUG
          log::fatal(R"(Unknown key="{}")"sv, key);
#endif
          break;
        case ID:
          // note! assuming id is the first field
          id = std::get<decltype(id)>(value);
          break;
        case ERROR:
          if (id >= 0) {
            Error error{value};
            Trace event{trace_info, error};
            dispatched = true;
            handler(event, id);
          }
          break;
        case RESULT:
          if (id >= 0) {
            core::json::Buffer buffer_2{buffer};
            Result result{value, buffer_2};
            Trace event{trace_info, result};
            dispatched = true;
            handler(event, id);
          }
          break;
        case STREAM:
          break;
        case DATA: {
          dispatch(
              handler, core::json::get<std::string_view>(value), buffer, trace_info, continue_with_unknown_event_type);
          return;
        }
        case EVENT_TYPE: {
          // note! assuming event_type is the first field
          EventType event_type(value);
          switch (event_type) {
            using enum EventType::type_t;
            case AGG_TRADE:
              dispatch_helper<AggTrade>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case _24HR_MINI_TICKER:
              dispatch_helper<MiniTicker>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case BOOK_TICKER:
              dispatch_helper<BookTicker>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case DEPTH_UPDATE:
              dispatch_helper<DepthUpdate>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case MARK_PRICE_UPDATE:
              dispatch_helper<MarkPriceUpdate>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case UNDEFINED__:
            case ORDER_TRADE_UPDATE:
            case ACCOUNT_UPDATE:
            case MARGIN_CALL:
            case GRID_UPDATE:
            case STRATEGY_UPDATE:
            case ACCOUNT_CONFIG_UPDATE:
              log::fatal("Unexpected"sv);
              break;
            case LISTEN_KEY_EXPIRED: {
              // XXX need parsing
              break;
            }
            case UNKNOWN__:
              if (!continue_with_unknown_event_type)
                log::fatal("Unexpected"sv);
              return;
          }
          assert(dispatched);
          break;
        }
        case ORDER_BOOK_UPDATE_ID:
          break;
      }
      if (dispatched)
        break;
    }
  }
  if (dispatched)
    return;
  log::warn(R"(message="{}")"sv, message);
  log::fatal("Unexpected"sv);
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
