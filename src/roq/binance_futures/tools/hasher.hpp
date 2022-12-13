/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/core/mac/hmac_sha256.hpp"

namespace roq {
namespace binance_futures {
namespace tools {

class Hasher final {
 public:
  Hasher(std::string_view const &key, std::string_view const &secret);

  Hasher(Hasher &&) = delete;
  Hasher(Hasher const &) = delete;

  std::string create_query(std::string_view const &body);

  std::string_view create_headers() const { return headers_; }

 private:
  const std::string key_;
  core::mac::HMAC_SHA256 hmac_;
  const std::string headers_;
};

}  // namespace tools
}  // namespace binance_futures
}  // namespace roq
