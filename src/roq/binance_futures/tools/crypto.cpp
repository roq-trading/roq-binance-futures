/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/tools/crypto.hpp"

#include <fmt/format.h>

#include <array>
#include <cassert>

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/utils/codec/base64.hpp"
#include "roq/utils/codec/hex.hpp"

#include "roq/utils/text/writer.hpp"

#include "roq/clock.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace tools {

// === HELPERS ===

namespace {
auto create_headers(auto &key) {
  return fmt::format("X-MBX-APIKEY: {}\r\n"sv, key);
}

template <typename R>
auto create_ed25519(auto &secret, auto margin_mode) {
  using result_type = std::remove_cvref_t<R>;
  if (margin_mode != MarginMode::PORTFOLIO) {
    return result_type::create(secret, false);
  }
  return result_type{};
}

template <typename R>
auto create_mac(auto &secret, auto margin_mode, auto &secret_2) {
  using result_type = std::remove_cvref_t<R>;
  if (margin_mode == MarginMode::PORTFOLIO) {
    if (!std::empty(secret_2)) {
      log::fatal("Unexpected: secret_2 is now allowed in PORTFOLIO mode"sv);
    }
    return result_type{secret};
  }
  return result_type::create(std::empty(secret_2) ? secret : secret_2, false);
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(
    std::string_view const &key, std::string_view const &secret, MarginMode margin_mode, std::string_view const &key_2, std::string_view const &secret_2)
    : key_{key}, key_2_{key_2}, headers_{create_headers(std::empty(key_2) ? key : key_2)}, pkey_{create_ed25519<decltype(pkey_)>(secret, margin_mode)},
      mac_{create_mac<decltype(mac_)>(secret, margin_mode, secret_2)} {
  if (std::empty(pkey_) && std::empty(mac_)) {
    log::fatal("The secret must be valid ED25519 or HMAC_SHA256"sv);
  }
}

std::string Crypto::create_rest_signature(std::chrono::milliseconds now_utc) {
  assert(!std::empty(mac_));
  auto timestamp = fmt::format("timestamp={}"sv, now_utc.count());
  mac_.clear();
  mac_.update(timestamp);
  auto digest = mac_.final(digest_2_);
  std::string signature;
  utils::codec::Hex::encode(signature, digest);
  return fmt::format("?{}&signature={}"sv, timestamp, signature);
}

std::string Crypto::create_rest_signature_body(std::chrono::milliseconds now_utc, std::string_view const &body) {
  assert(!std::empty(mac_));
  auto timestamp = fmt::format("timestamp={}"sv, now_utc.count());
  mac_.clear();
  mac_.update(timestamp);
  assert(!std::empty(body));
  mac_.update(body);
  auto digest = mac_.final(digest_2_);
  std::string signature;
  utils::codec::Hex::encode(signature, digest);
  return fmt::format("?{}&signature={}"sv, timestamp, signature);
}

std::string Crypto::create_rest_signature_query(std::chrono::milliseconds now_utc, std::string_view const &query) {
  assert(!std::empty(mac_));
  auto tmp = fmt::format("{}&timestamp={}"sv, query, now_utc.count());
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_2_);
  std::string signature;
  utils::codec::Hex::encode(signature, digest);
  return fmt::format("?{}&signature={}"sv, tmp, signature);
}

std::string_view Crypto::create_session_logon_signature(std::string &buffer, std::chrono::milliseconds now_utc) {
  assert(!std::empty(pkey_));
  auto payload = fmt::format("apiKey={}&timestamp={}"sv, key_, now_utc.count());
  digest_.clear();
  context_.reset();
  pkey_.sign(digest_, payload, context_);
  utils::codec::Base64::encode(buffer, digest_, false, false);
  return buffer;
}

}  // namespace tools
}  // namespace binance_futures
}  // namespace roq
