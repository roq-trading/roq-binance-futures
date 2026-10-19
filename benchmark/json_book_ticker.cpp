/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <benchmark/benchmark.h>

#include "roq/binance_futures/protocol/json/market_stream_parser.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using value_type = protocol::json::BookTicker;

// === HELPERS ===

namespace {
template <typename T>
struct MarketStreamParserTester final : public protocol::json::MarketStreamParser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    MarketStreamParserTester handler{callback};
    protocol::json::MarketStreamParser::dispatch(handler, message, buffers, {}, false);
  }

 protected:
  explicit MarketStreamParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<protocol::json::Error> const &event, [[maybe_unused]] int32_t request_id) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Result> const &event, [[maybe_unused]] int32_t request_id) override { dispatch_helper(event); }
  // update
  void operator()(Trace<protocol::json::Trade2> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::AggTrade> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::MiniTicker> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::BookTicker> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::DepthUpdate> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::MarkPriceUpdate> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Kline> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::AssetIndexUpdate> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::ForceOrder> const &event) override { dispatch_helper(event); }

  template <typename U>
  void dispatch_helper(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};
}  // namespace

// === IMPLEMENTATION ===

void BM_json_book_ticker(benchmark::State &state) {
  auto message = R"({)"
                 R"("e":"bookTicker",)"
                 R"("u":847033385825,)"
                 R"("s":"BTCUSDT",)"
                 R"("b":"58950.76",)"
                 R"("B":"0.172",)"
                 R"("a":"58955.21",)"
                 R"("A":"0.191",)"
                 R"("T":1634288226709,)"
                 R"("E":1634288226716)"
                 R"(})";
  auto helper = [](value_type const &obj) { int a = 1; };
  for (auto _ : state) {
    MarketStreamParserTester<value_type>::dispatch(helper, message, 8192, 1);
  }
}

BENCHMARK(BM_json_book_ticker);
