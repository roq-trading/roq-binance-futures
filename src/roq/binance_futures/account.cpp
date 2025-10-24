/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/account.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === HELPERS ===

namespace {
auto create_crypto(auto &config, auto &name) -> tools::Crypto {
  auto ready = true;
  auto key = config.get_api_key(name);
  if (std::empty(key)) {
    ready = false;
    log::warn(R"(Unexpected: missing key for name="{}")"sv, name);
  }
  auto secret = config.get_secret(name);
  if (std::empty(secret)) {
    ready = false;
    log::warn(R"(Unexpected: missing secret for name="{}")"sv, name);
  }
  if (!ready) {
    log::fatal("Invalid config"sv);
  }
  return {key, secret};
}
}  // namespace

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name, MarginMode margin_mode)
    : name{name}, margin_mode{margin_mode}, crypto_{create_crypto(config, name)}, query_encode_buffer_(tools::Crypto::QUERY_BUFFER_LENGTH) {
}

std::string Account::create_rest_signature() {
  auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
  return crypto_.create_rest_signature(now_utc);
}

std::string Account::create_rest_signature_body(std::string_view const &body) {
  auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
  return crypto_.create_rest_signature_body(now_utc, body);
}

std::string Account::create_rest_signature_query(std::string_view const &query) {
  auto now_utc = clock::get_realtime<std::chrono::milliseconds>();
  return crypto_.create_rest_signature_query(now_utc, query);
}

}  // namespace binance_futures
}  // namespace roq
