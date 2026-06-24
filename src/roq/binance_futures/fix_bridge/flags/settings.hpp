/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/binance_futures/flags/settings.hpp"

#include "roq/binance_futures/fix_bridge/flags/fix_bridge.hpp"

namespace roq {
namespace binance_futures {
namespace fix_bridge {
namespace flags {

struct Settings final : public binance_futures::flags::Settings {
  explicit Settings(args::Parser const &);

  FIXBridge fix_bridge;
};

}  // namespace flags
}  // namespace fix_bridge
}  // namespace binance_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::binance_futures::fix_bridge::flags::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::binance_futures::fix_bridge::flags::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(fix_bridge={}, )"
        R"(gateway={})"
        R"(}})"sv,
        value.fix_bridge,
        static_cast<roq::binance_futures::flags::Settings const &>(value));
  }
};
