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
  Account(Config const &, std::string_view const &name);

  Account(Account &&) = delete;
  Account(Account const &) = delete;

  std::string_view get_name() const { return name_; }

  std::string create_query(std::string_view const &body) { return crypto_.create_query(body); }
  std::string create_query() { return create_query({}); }

  std::string_view create_headers() const { return crypto_.create_headers(); }

 private:
  std::string const name_;
  tools::Crypto crypto_;
};

}  // namespace binance_futures
}  // namespace roq
