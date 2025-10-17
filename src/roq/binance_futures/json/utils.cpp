/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/utils.hpp"

#include "roq/logging.hpp"

#include "roq/binance_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// NOLINTBEGIN(readability-magic-numbers)

Error guess_error([[maybe_unused]] int32_t code) {
  switch (code) {
    case -2011:  // CANCEL_REJECTED
      return Error::UNKNOWN_EXTERNAL_ORDER_ID;
    case -2013:  // NO_SUCH_ORDER
      return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
    case -4197:  // SAME_ORDER
      return Error::MODIFY_HAS_NO_EFFECT;
  }
  log::warn("DEBUG unable to map error_code={}"sv, code);
  return Error::UNKNOWN;
}

// NOLINTEND(readability-magic-numbers)

// map

TradingStatus trading_status_helper(SymbolStatus symbol_status, ContractStatus contract_status) {
  if (symbol_status != SymbolStatus::UNDEFINED_INTERNAL) {
    return map(symbol_status);
  }
  return map(contract_status);
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
