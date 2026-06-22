/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/binance_futures/flags/settings.hpp"

#include "roq/binance_futures/bridge/flags/fix.hpp"
#include "roq/binance_futures/bridge/flags/messaging.hpp"
#include "roq/binance_futures/bridge/flags/oms.hpp"
#include "roq/binance_futures/bridge/flags/test.hpp"

namespace roq {
namespace binance_futures {
namespace bridge {
namespace flags {

struct Settings final : public binance_futures::flags::Settings {
  explicit Settings(args::Parser const &);

  FIX fix;
  Messaging messaging;
  OMS oms;
  Test test;
};

}  // namespace flags
}  // namespace bridge
}  // namespace binance_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::binance_futures::bridge::flags::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::binance_futures::bridge::flags::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(fix={}, )"
        R"(messaging={}, )"
        R"(oms={}, )"
        R"(test={}, )"
        R"(gateway={})"
        R"(}})"sv,
        value.fix,
        value.messaging,
        value.oms,
        value.test,
        static_cast<roq::binance_futures::flags::Settings const &>(value));
  }
};
