/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/binance_futures/json/utils.hpp"

namespace roq {
namespace binance_futures {
namespace json {

Error guess_error(int32_t code) {
  return Error::UNKNOWN;
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
