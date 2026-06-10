/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/binance_futures/protocol/json/wsapi_parser.hpp"

namespace roq {
namespace binance_futures {

template <typename T>
struct WSAPIParserTester final : public protocol::json::WSAPIParser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    // simple
    // XXX FIXME TODO catch2 block ???
    T obj{message, buffers};
    callback(obj);
    // parser
    // XXX FIXME TODO catch2 block ???
    WSAPIParserTester handler{callback};
    auto res = protocol::json::WSAPIParser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit WSAPIParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<protocol::json::WSAPIError> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPISessionLogon> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIListenKey> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIAccountBalance> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIAccountStatus> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIAccountPosition> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIOpenOrders> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPITrades> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIOpenOrdersCancelAll> const &event, protocol::json::WSAPIRequest const &) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIOrderPlace> const &event, protocol::json::WSAPIRequest const &) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIOrderModify> const &event, protocol::json::WSAPIRequest const &) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::WSAPIOrderCancel> const &event, protocol::json::WSAPIRequest const &) override { dispatch_helper(event); }

  template <typename U>
  void dispatch_helper(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace binance_futures
}  // namespace roq
