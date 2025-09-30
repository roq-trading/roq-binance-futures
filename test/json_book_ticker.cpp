/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/binance_futures/json/market_stream_parser.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_book_ticker_simple", "[json_book_ticker]") {
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
  core::json::BufferStack buffer{8192, 1};
  json::BookTicker obj{message, buffer};
  CHECK(obj.event_type == json::EventType::BOOK_TICKER);
  CHECK(obj.order_book_update_id == 847033385825);
  CHECK(obj.symbol == "BTCUSDT"sv);
  CHECK(obj.best_bid_price == 58950.76_a);
  CHECK(obj.best_bid_qty == 0.172_a);
  CHECK(obj.best_ask_price == 58955.21_a);
  CHECK(obj.best_ask_qty == 0.191_a);
  CHECK(obj.transaction_time == 1634288226709ms);
  CHECK(obj.event_time == 1634288226716ms);
}

TEST_CASE("json_book_ticker_parse_stream_simple", "[json_book_ticker]") {
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
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  struct MyHandler final : public json::MarketStreamParser::Handler {
    void operator()(Trace<json::Error> const &, [[maybe_unused]] int32_t id) override { FAIL(); }
    void operator()(Trace<json::Result> const &, [[maybe_unused]] int32_t id) override { FAIL(); }
    void operator()(Trace<json::AggTrade> const &) override { FAIL(); }
    void operator()(Trace<json::MiniTicker> const &) override { FAIL(); }
    void operator()(Trace<json::BookTicker> const &) override { found_ = true; }
    void operator()(Trace<json::DepthUpdate> const &) override { FAIL(); }
    void operator()(Trace<json::MarkPriceUpdate> const &) override { FAIL(); }
    void operator()(Trace<json::Kline> const &) override { FAIL(); }

    operator bool() const { return found_; }

   private:
    bool found_ = false;
  } handler;
  json::MarketStreamParser::dispatch(handler, message, buffer, trace_info, false);
  CHECK(static_cast<bool>(handler) == true);
}

TEST_CASE("json_book_ticker_parse_stream_wrapped", "[json_book_ticker]") {
  auto message = R"({)"
                 R"("stream":"btcusdt@bookTicker",)"
                 R"("data":{)"
                 R"("e":"bookTicker",)"
                 R"("u":847033385825,)"
                 R"("s":"BTCUSDT",)"
                 R"("b":"58950.76",)"
                 R"("B":"0.172",)"
                 R"("a":"58955.21",)"
                 R"("A":"0.191",)"
                 R"("T":1634288226709,)"
                 R"("E":1634288226716)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  struct MyHandler final : public json::MarketStreamParser::Handler {
    void operator()(Trace<json::Error> const &, [[maybe_unused]] int32_t id) override { FAIL(); }
    void operator()(Trace<json::Result> const &, [[maybe_unused]] int32_t id) override { FAIL(); }
    void operator()(Trace<json::AggTrade> const &) override { FAIL(); }
    void operator()(Trace<json::MiniTicker> const &) override { FAIL(); }
    void operator()(Trace<json::BookTicker> const &) override { found_ = true; }
    void operator()(Trace<json::DepthUpdate> const &) override { FAIL(); }
    void operator()(Trace<json::MarkPriceUpdate> const &) override { FAIL(); }
    void operator()(Trace<json::Kline> const &) override { FAIL(); }

    operator bool() const { return found_; }

   private:
    bool found_ = false;
  } handler;
  json::MarketStreamParser::dispatch(handler, message, buffer, trace_info, false);
  CHECK(static_cast<bool>(handler) == true);
}

TEST_CASE("json_book_ticker_simple_coin_m", "[json_book_ticker]") {
  auto message = R"({)"
                 R"("u":300683916630,)"
                 R"("e":"bookTicker",)"
                 R"("s":"BTCUSD_220325",)"
                 R"("ps":"BTCUSD",)"
                 R"("b":"49387.1",)"
                 R"("B":"34",)"
                 R"("a":"49387.2",)"
                 R"("A":"577",)"
                 R"("T":1640247707198,)"
                 R"("E":1640247707203)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::BookTicker obj{message, buffer};
  CHECK(obj.order_book_update_id == 300683916630);
  CHECK(obj.event_type == json::EventType::BOOK_TICKER);
  CHECK(obj.symbol == "BTCUSD_220325"sv);
  CHECK(obj.pair == "BTCUSD"sv);
  CHECK(obj.best_bid_price == 49387.1_a);
  CHECK(obj.best_bid_qty == 34.0_a);
  CHECK(obj.best_ask_price == 49387.2_a);
  CHECK(obj.best_ask_qty == 577.0_a);
  CHECK(obj.transaction_time == 1640247707198ms);
  CHECK(obj.event_time == 1640247707203ms);
}
