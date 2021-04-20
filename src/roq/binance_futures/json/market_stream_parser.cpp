/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/binance_futures/json/market_stream_parser.h"

#include <cctype>
#include <string>

#include "roq/compat.h"

#include "roq/binance_futures/json/field.h"
#include "roq/binance_futures/json/stream.h"

#include "roq/logging.h"

using namespace roq::literals;

namespace roq {
namespace binance_futures {
namespace json {

namespace {
template <typename T>
static void dispatch_helper(
    MarketStreamParser::Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  T value(root, buffer);
  server::create_trace_and_dispatch(trace_info, value, handler);
}
}  // namespace

void MarketStreamParser::dispatch(
    MarketStreamParser::Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const server::TraceInfo &trace_info) {
  int64_t id = -1;
  std::string symbol;  // allocating because we need uppercase
  auto stream = Stream::UNDEFINED;
  bool dispatched = false;
  for (int i = 0; i < 2 && !dispatched; ++i) {
    core::json::Parser parser(message);
    auto root = parser.root();
    for (auto [key, value] : std::get<core::json::object_t>(root)) {
      auto field = Field(key);
      switch (field) {
        case Field::UNDEFINED:
          log::fatal("Unexpected"_sv);
          break;
        case Field::UNKNOWN:
#if !defined(NDEBUG)
          log::fatal(R"(Unknown key="{}")"_fmt, key);
#endif
          break;
        case Field::ID:
          // note! assuming id is the first field
          id = std::get<decltype(id)>(value);
          break;
        case Field::ERROR:
          if (id >= 0) {
            Error error(value);
            dispatched = true;
            handler(id, error);
          }
          break;
        case Field::RESULT:
          if (id >= 0) {
            Result result(value, buffer);
            dispatched = true;
            handler(id, result);
          }
          break;
        case Field::EVENT_TYPE: {
          // note! assuming event_type is the first field
          EventType event_type(value);
          switch (event_type) {
            case EventType::AGG_TRADE:
              dispatch_helper<AggTrade>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case EventType::_24HR_MINI_TICKER:
              dispatch_helper<MiniTicker>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case EventType::BOOK_TICKER:
              dispatch_helper<BookTicker>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case EventType::DEPTH_UPDATE:
              dispatch_helper<DepthUpdate>(handler, message, buffer, trace_info);
              dispatched = true;
              break;
            case EventType::UNDEFINED:
            case EventType::UNKNOWN:
            case EventType::OUTBOUND_ACCOUNT_INFO:
            case EventType::OUTBOUND_ACCOUNT_POSITION:
            case EventType::BALANCE_UPDATE:
            case EventType::EXECUTION_REPORT:
            case EventType::LIST_STATUS:
              log::fatal("Unexpected"_sv);
              break;
          }
          assert(dispatched);
          break;
        }
      }
      if (dispatched)
        break;
    }
  }
  if (dispatched)
    return;
  log::warn(R"(message="{}")"_fmt, message);
  log::fatal("Unexpected"_sv);
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
