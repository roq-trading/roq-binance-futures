/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/binance_futures/config.h"

#include "roq/binance_futures/tools/hasher.h"

namespace roq {
namespace binance_futures {

class Security final {
 public:
  Security(const Config &, const std::string_view &account);

  Security(Security &&) = delete;
  Security(const Security &) = delete;

  std::string_view get_account() const { return account_; }

  std::string create_query(const std::string_view &body) { return hasher_.create_query(body); }
  std::string create_query() { return create_query({}); }

  std::string_view create_headers() const { return hasher_.create_headers(); }

 private:
  const std::string account_;
  tools::Hasher hasher_;
};

}  // namespace binance_futures
}  // namespace roq
