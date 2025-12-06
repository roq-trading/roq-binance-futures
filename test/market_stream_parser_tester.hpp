/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/binance_futures/json/market_stream_parser.hpp"

namespace roq {
namespace binance_futures {

template <typename T>
struct MarketStreamParserTester final : public json::MarketStreamParser::Handler {
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
    MarketStreamParserTester handler{callback};
    auto res = json::MarketStreamParser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit MarketStreamParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<json::Error> const &event, [[maybe_unused]] int32_t request_id) override { dispatch_helper(event); }
  void operator()(Trace<json::Result> const &event, [[maybe_unused]] int32_t request_id) override { dispatch_helper(event); }
  // update
  void operator()(Trace<json::AggTrade> const &event) override { dispatch_helper(event); }
  void operator()(Trace<json::MiniTicker> const &event) override { dispatch_helper(event); }
  void operator()(Trace<json::BookTicker> const &event) override { dispatch_helper(event); }
  void operator()(Trace<json::DepthUpdate> const &event) override { dispatch_helper(event); }
  void operator()(Trace<json::MarkPriceUpdate> const &event) override { dispatch_helper(event); }
  void operator()(Trace<json::Kline> const &event) override { dispatch_helper(event); }

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
