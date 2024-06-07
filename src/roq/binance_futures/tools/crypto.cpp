/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/tools/crypto.hpp"

#include <fmt/format.h>

#include <array>
#include <cassert>

#include "roq/utils/safe_cast.hpp"

#include "roq/utils/codec/hex.hpp"

#include "roq/utils/text/writer.hpp"

#include "roq/clock.hpp"

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

Crypto::Crypto(std::string_view const &key, std::string_view const &secret) : key{key}, mac_{secret}, headers_{create_headers_helper(key)} {
}

std::string Crypto::create_query(std::string_view const &body) {
  auto now = clock::get_realtime<std::chrono::milliseconds>();
  auto timestamp = fmt::format("timestamp={}"sv, now.count());
  mac_.clear();
  mac_.update(timestamp);
  if (!std::empty(body))
    mac_.update(body);
  auto digest = mac_.final(digest_);
  std::string signature;
  utils::codec::Hex::encode(signature, digest);
  return fmt::format("?{}&signature={}"sv, timestamp, signature);
}

std::string Crypto::create_query_2(std::string_view const &body) {
  auto now = clock::get_realtime<std::chrono::milliseconds>();
  auto tmp = fmt::format("{}&timestamp={}"sv, body, now.count());
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature;
  utils::codec::Hex::encode(signature, digest);
  return fmt::format("?{}&signature={}"sv, tmp, signature);
}

std::string_view Crypto::create_ws_api_signature(std::span<std::byte> const &buffer, std::string_view const &body) {
  mac_.clear();
  mac_.update(body);
  auto digest = mac_.final(digest_);
  utils::text::Writer writer{buffer};
  writer.write(utils::codec::Hex{digest});
  return writer.finish();
}

}  // namespace tools
}  // namespace binance_futures
}  // namespace roq
