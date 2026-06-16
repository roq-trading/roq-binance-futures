/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include <fmt/format.h>

namespace roq {
namespace binance_futures {
namespace gateway {

struct Utils final {
  inline static std::string_view create_external_order_id(std::string &buffer, std::string_view const &symbol, auto &order_id) {
    using namespace std::literals;
    buffer.clear();
    fmt::format_to(std::back_inserter(buffer), "{}#{}"sv, symbol, order_id);
    return buffer;
  }
};

}  // namespace gateway
}  // namespace binance_futures
}  // namespace roq
