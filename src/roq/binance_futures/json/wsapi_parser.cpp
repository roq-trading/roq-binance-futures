/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/wsapi_parser.hpp"

#include "roq/logging.hpp"

#include "roq/utils/hash/fnv.hpp"

#include "roq/binance_futures/json/wsapi_type.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// === CONSTANTS ===

namespace {
constexpr auto const KEY_ID = "id"sv;
constexpr auto const KEY_ERROR = "error"sv;
}  // namespace

// === HELPERS ===

namespace {
template <typename T, typename... Args>
bool dispatch_helper(auto &handler, auto &value, auto &buffer_stack, auto &trace_info, Args &&...args) {
  T obj{value, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj, std::forward<Args>(args)...);
  return true;
}
}  // namespace

// === IMPLEMENTATION ===

bool WSAPIParser::dispatch(
    WSAPIParser::Handler &handler,
    std::string_view const &message,
    core::json::BufferStack &buffer_stack,
    TraceInfo const &trace_info,
    bool allow_unknown_event_types) {
  auto result = false;
  auto helper = [&](auto &key, auto &value) {
    auto key_2 = utils::hash::FNV::compute(key);
    switch (key_2) {
      case utils::hash::FNV::compute(KEY_ERROR):
        result = dispatch_helper<WSAPIError>(handler, message, buffer_stack, trace_info);
        return true;
      case utils::hash::FNV::compute(KEY_ID): {
        auto value_2 = core::json::get<std::string_view>(value);
        if (!std::empty(value_2)) {
          auto request = WSAPIRequest::decode(value_2);
          switch (request.type) {
            using enum WSAPIType::type_t;
            case UNDEFINED_INTERNAL:
              log::fatal("Unexpected"sv);
            case UNKNOWN_INTERNAL:
              return true;
            case SESSION_LOGON:
              result = dispatch_helper<WSAPISessionLogon>(handler, message, buffer_stack, trace_info);
              return true;
            case USER_DATA_STREAM_START:
              result = dispatch_helper<WSAPIListenKey>(handler, message, buffer_stack, trace_info);
              return true;
            case USER_DATA_STREAM_PING:
              // note! drop
              return false;
            case ACCOUNT_BALANCE:
              result = dispatch_helper<WSAPIAccountBalance>(handler, message, buffer_stack, trace_info);
              return true;
            case ACCOUNT_STATUS:
              result = dispatch_helper<WSAPIAccountStatus>(handler, message, buffer_stack, trace_info);
              return true;
            case ACCOUNT_POSITION:
              result = dispatch_helper<WSAPIAccountPosition>(handler, message, buffer_stack, trace_info);
              return true;
            case ORDERS_STATUS:
              result = dispatch_helper<WSAPIOpenOrders>(handler, message, buffer_stack, trace_info);
              return true;
            case MY_TRADES:
              result = dispatch_helper<WSAPITrades>(handler, message, buffer_stack, trace_info);
              return true;
            case OPEN_ORDERS_CANCEL_ALL:
              result = dispatch_helper<WSAPIOpenOrdersCancelAll>(handler, message, buffer_stack, trace_info, request);
              return true;
            case ORDER_PLACE:
              result = dispatch_helper<WSAPIOrderPlace>(handler, message, buffer_stack, trace_info, request);
              return true;
            case ORDER_MODIFY:
              result = dispatch_helper<WSAPIOrderModify>(handler, message, buffer_stack, trace_info, request);
              return true;
            case ORDER_CANCEL:
              result = dispatch_helper<WSAPIOrderCancel>(handler, message, buffer_stack, trace_info, request);
              return true;
          }
        }
        break;
      }
    }
    return result;
  };
  core::json::Parser::dispatch<core::json::Object>(helper, message);
  if (result || allow_unknown_event_types) {
    return result;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
