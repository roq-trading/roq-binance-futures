/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/binance_futures/security.h"

namespace roq {
namespace binance_futures {

Security::Security(const Config &config, const std::string_view &account)
    : account_(account), hasher_(config.get_api_key(account_), config.get_secret(account_)) {
}

std::string Security::create_headers() {
  return hasher_.create_headers();
}

std::string Security::create_query() {
  return hasher_.create_query();
}
/*

std::string Security::create_signature() {
  return hasher_.create_signature();
}

std::pair<std::string, std::string> Security::create_signature(std::chrono::nanoseconds now) {
  return hasher_.create_signature(now);
}
*/

}  // namespace binance_futures
}  // namespace roq
