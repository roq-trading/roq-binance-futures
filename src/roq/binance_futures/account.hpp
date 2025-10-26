/* Copyright (c) 2017-2025, Hans Erik Thrane */

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

  Account(Account const &) = delete;

  std::string_view get_key() const { return crypto_.get_key(); }

  std::string_view get_rest_headers() const { return crypto_.get_rest_headers(); }

  // classic

  std::string create_rest_signature();
  std::string create_rest_signature_body(std::string_view const &body);
  std::string create_rest_signature_query(std::string_view const &query);

  // ed25519

  std::string_view create_session_logon_signature(std::chrono::milliseconds now_utc) { return crypto_.create_session_logon_signature(sign_buffer_, now_utc); }

  std::string const name;
  MarginMode const margin_mode;

 private:
  tools::Crypto crypto_;
  std::string sign_buffer_;
  std::vector<std::byte> query_encode_buffer_;
};

}  // namespace binance_futures
}  // namespace roq
