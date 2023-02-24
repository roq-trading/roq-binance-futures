/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/binance_futures/config.hpp"

#include "roq/binance_futures/tools/crypto.hpp"

namespace roq {
namespace binance_futures {

struct Authenticator final {
  Authenticator(Config const &, std::string_view const &account);

  Authenticator(Authenticator &&) = delete;
  Authenticator(Authenticator const &) = delete;

  std::string_view get_account() const { return account_; }

  std::string create_query(std::string_view const &body) { return crypto_.create_query(body); }
  std::string create_query() { return create_query({}); }

  std::string_view create_headers() const { return crypto_.create_headers(); }

 private:
  std::string const account_;
  tools::Crypto crypto_;
};

}  // namespace binance_futures
}  // namespace roq
