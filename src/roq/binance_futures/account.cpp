/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/account.hpp"

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name)
    : name_{name}, crypto_{config.get_api_key(name_), config.get_secret(name_)} {
}

}  // namespace binance_futures
}  // namespace roq
