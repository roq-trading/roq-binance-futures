/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/gateway/account.hpp"

#include "roq/logging.hpp"

#include "roq/utils/safe_cast.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace gateway {

// === HELPERS ===

namespace {
template <typename R>
auto create_crypto(auto &config, auto &name, auto margin_mode) {
  using result_type = std::remove_cvref_t<R>;
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
  auto key_2 = config.get_api_key_2(name);
  auto secret_2 = config.get_secret_2(name);
  return result_type{key, secret, margin_mode, key_2, secret_2};
}
}  // namespace

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name, MarginMode margin_mode)
    : name{name}, margin_mode{margin_mode}, crypto_{create_crypto<decltype(crypto_)>(config, name, margin_mode)},
      query_encode_buffer_(tools::Crypto::QUERY_BUFFER_LENGTH) {
}

std::string Account::create_rest_signature(TraceInfo const &trace_info) {
  return crypto_.create_rest_signature(utils::safe_cast(trace_info.origin_create_time_utc));
}

std::string Account::create_rest_signature_body(TraceInfo const &trace_info, std::string_view const &body) {
  return crypto_.create_rest_signature_body(utils::safe_cast(trace_info.origin_create_time_utc), body);
}

std::string Account::create_rest_signature_query(TraceInfo const &trace_info, std::string_view const &query) {
  return crypto_.create_rest_signature_query(utils::safe_cast(trace_info.origin_create_time_utc), query);
}

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
