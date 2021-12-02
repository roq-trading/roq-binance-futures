/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/binance_futures/json/market_stream_parser.h"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_book_ticker, simple) {
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
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto obj = core::json::Parser::create<json::BookTicker>(message, buffer);
  EXPECT_EQ(obj.event_type, json::EventType::BOOK_TICKER);
  EXPECT_EQ(obj.order_book_update_id, 847033385825);
  EXPECT_EQ(obj.symbol, "BTCUSDT"sv);
  EXPECT_DOUBLE_EQ(obj.best_bid_price, 58950.76);
  EXPECT_DOUBLE_EQ(obj.best_bid_qty, 0.172);
  EXPECT_DOUBLE_EQ(obj.best_ask_price, 58955.21);
  EXPECT_DOUBLE_EQ(obj.best_ask_qty, 0.191);
  EXPECT_EQ(obj.transaction_time, 1634288226709ms);
  EXPECT_EQ(obj.event_time, 1634288226716ms);
}

TEST(json_book_ticker, parse_stream_simple) {
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
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto trace_info = server::create_trace_info();
  struct MyHandler final : public json::MarketStreamParser::Handler {
    void operator()(const server::Trace<json::Error> &, [[maybe_unused]] int32_t id) override {
      FAIL();
    }
    void operator()(const server::Trace<json::Result> &, [[maybe_unused]] int32_t id) override {
      FAIL();
    }
    void operator()(const server::Trace<json::AggTrade> &) override { FAIL(); }
    void operator()(const server::Trace<json::MiniTicker> &) override { FAIL(); }
    void operator()(const server::Trace<json::BookTicker> &) override { found_ = true; }
    void operator()(const server::Trace<json::DepthUpdate> &) override { FAIL(); }
    void operator()(const server::Trace<json::MarkPriceUpdate> &) override { FAIL(); }

    operator bool() const { return found_; }

   private:
    bool found_ = false;
  } handler;
  json::MarketStreamParser::dispatch(handler, message, buffer, trace_info);
  EXPECT_TRUE(static_cast<bool>(handler));
}

TEST(json_book_ticker, parse_stream_wrapped) {
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
  core::Buffer buffer_(65536);
  core::json::Buffer buffer(buffer_);
  auto trace_info = server::create_trace_info();
  struct MyHandler final : public json::MarketStreamParser::Handler {
    void operator()(const server::Trace<json::Error> &, [[maybe_unused]] int32_t id) override {
      FAIL();
    }
    void operator()(const server::Trace<json::Result> &, [[maybe_unused]] int32_t id) override {
      FAIL();
    }
    void operator()(const server::Trace<json::AggTrade> &) override { FAIL(); }
    void operator()(const server::Trace<json::MiniTicker> &) override { FAIL(); }
    void operator()(const server::Trace<json::BookTicker> &) override { found_ = true; }
    void operator()(const server::Trace<json::DepthUpdate> &) override { FAIL(); }
    void operator()(const server::Trace<json::MarkPriceUpdate> &) override { FAIL(); }

    operator bool() const { return found_; }

   private:
    bool found_ = false;
  } handler;
  json::MarketStreamParser::dispatch(handler, message, buffer, trace_info);
  EXPECT_TRUE(static_cast<bool>(handler));
}
