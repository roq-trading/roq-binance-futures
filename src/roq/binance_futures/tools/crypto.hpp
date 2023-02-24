/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <array>
#include <string>
#include <string_view>

#include "roq/core/mac/hmac.hpp"

namespace roq {
namespace binance_futures {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  std::string create_query(std::string_view const &body);

  std::string_view create_headers() const { return headers_; }

 private:
  using MAC = core::mac::HMAC<core::hash::SHA256>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  std::string const key_;
  MAC mac_;
  Digest digest_;
  std::string const headers_;
};

}  // namespace tools
}  // namespace binance_futures
}  // namespace roq
