/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/tools/hasher.hpp"

#include <fmt/format.h>

#include <array>
#include <cassert>

#include "roq/utils/safe_cast.hpp"

#include "roq/clock.hpp"

#include "roq/core/codec/hex.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace tools {

// === HELPERS ===

namespace {
auto create_headers_helper(auto const &key) {
  return fmt::format("X-MBX-APIKEY: {}\r\n"sv, key);
}
}  // namespace

// === IMPLEMENTATION ===

Hasher::Hasher(std::string_view const &key, std::string_view const &secret)
    : key_{key}, mac_{secret}, headers_{create_headers_helper(key_)} {
}

std::string Hasher::create_query(std::string_view const &body) {
  auto now = clock::get_realtime<std::chrono::milliseconds>();
  auto timestamp = fmt::format("timestamp={}"sv, now.count());
  mac_.clear();
  mac_.update(timestamp);
  if (!std::empty(body))
    mac_.update(body);
  auto digest = mac_.final(digest_);
  std::string signature;
  core::codec::Hex::encode(signature, digest);
  return fmt::format("?{}&signature={}"sv, timestamp, signature);
}

}  // namespace tools
}  // namespace binance_futures
}  // namespace roq
