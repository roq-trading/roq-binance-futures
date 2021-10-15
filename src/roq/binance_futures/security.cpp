/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/binance_futures/security.h"

namespace roq {
namespace binance_futures {

Security::Security(const Config &config, const std::string_view &account)
    : account_(account), hasher_(config.get_api_key(account_), config.get_secret(account_)) {
}

}  // namespace binance_futures
}  // namespace roq
