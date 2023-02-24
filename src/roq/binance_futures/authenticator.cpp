/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/authenticator.hpp"

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

Authenticator::Authenticator(Config const &config, std::string_view const &account)
    : account_{account}, crypto_{config.get_api_key(account_), config.get_secret(account_)} {
}

}  // namespace binance_futures
}  // namespace roq
