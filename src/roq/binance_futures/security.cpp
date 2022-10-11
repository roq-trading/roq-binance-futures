/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/binance_futures/security.hpp"

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

Security::Security(Config const &config, std::string_view const &account)
    : account_(account), hasher_(config.get_api_key(account_), config.get_secret(account_)) {
}

}  // namespace binance_futures
}  // namespace roq
