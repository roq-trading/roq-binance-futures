/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/protocol/json/utils.hpp"

#include "roq/logging.hpp"

#include "roq/binance_futures/protocol/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace protocol {
namespace json {

// map

TradingStatus trading_status_helper(SymbolStatus symbol_status, ContractStatus contract_status) {
  if (symbol_status != SymbolStatus::UNDEFINED_INTERNAL) {
    return map(symbol_status);
  }
  return map(contract_status);
}

std::string_view get_order_id(std::string_view const &external_order_id) {
  auto pos = external_order_id.find('#');
  if (pos == std::string_view::npos) {
    return external_order_id;
  }
  return external_order_id.substr(pos + 1);
}

}  // namespace json
}  // namespace protocol
}  // namespace binance_futures
}  // namespace roq
