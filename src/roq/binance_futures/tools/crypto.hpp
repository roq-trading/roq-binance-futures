/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <array>
#include <span>
#include <string>
#include <string_view>

#include "roq/utils/mac/hmac.hpp"

namespace roq {
namespace binance_futures {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  std::string create_query(std::string_view const &body);
  std::string create_query_2(std::string_view const &body);

  std::string_view create_headers() const { return headers_; }

  // WS API

  std::string_view create_ws_api_signature(std::span<std::byte> const &buffer, std::string_view const &body);

  static constexpr auto const QUERY_BUFFER_LENGTH = 128uz;  // note! expected length == 99

  std::string const key;

 private:
  using MAC = utils::mac::HMAC<utils::hash::SHA256>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  MAC mac_;
  Digest digest_;
  std::string const headers_;
};

}  // namespace tools
}  // namespace binance_futures
}  // namespace roq
