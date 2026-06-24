/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <string>

namespace roq {
namespace binance_futures {
namespace bridge {

struct User final {
  uint16_t user_id = {};
  std::string component;
  std::string username;
  std::string password;
};

}  // namespace bridge
}  // namespace binance_futures
}  // namespace roq

// user

template <>
struct fmt::formatter<roq::binance_futures::bridge::User> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::binance_futures::bridge::User const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(user_id={}, )"
        R"(component="{}", )"
        R"(username="{}", )"
        R"(password=***)"
        R"(}})"sv,
        value.user_id,
        value.component,
        value.username);
  }
};
