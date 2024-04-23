/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/binance_futures/config.hpp"

#include "roq/binance_futures/tools/crypto.hpp"

namespace roq {
namespace binance_futures {

struct Account final {
  Account(Config const &, std::string_view const &name, MarginMode);

  Account(Account &&) = delete;
  Account(Account const &) = delete;

  std::string_view get_key() const { return crypto_.key; }

  std::string create_query(std::string_view const &body) { return crypto_.create_query(body); }
  std::string create_query() { return create_query({}); }

  // XXX
  std::string create_query_2(std::string_view const &body) { return crypto_.create_query_2(body); }

  std::string_view create_headers() const { return crypto_.create_headers(); }

  std::string_view create_ws_api_signature(std::string_view const &body) {
    return crypto_.create_ws_api_signature(query_encode_buffer_, body);
  }

  std::string const name;
  MarginMode const margin_mode;

 private:
  tools::Crypto crypto_;
  std::vector<std::byte> query_encode_buffer_;
};

}  // namespace binance_futures
}  // namespace roq
