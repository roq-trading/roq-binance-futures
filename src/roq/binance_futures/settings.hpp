/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/binance_futures/flags/flags.hpp"
#include "roq/binance_futures/flags/mbp.hpp"
#include "roq/binance_futures/flags/misc.hpp"
#include "roq/binance_futures/flags/request.hpp"
#include "roq/binance_futures/flags/rest.hpp"
#include "roq/binance_futures/flags/ws.hpp"
#include "roq/binance_futures/flags/ws_api.hpp"

namespace roq {
namespace binance_futures {

struct Settings final : public server::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::Misc misc;
  flags::REST rest;
  flags::WS ws;
  flags::MBP mbp;
  flags::Request request;
  flags::WS_API ws_api_2;  // note! overlapping with flags::Flags

 private:
  Settings(args::Parser const &, flags::Flags const &);
};

}  // namespace binance_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::binance_futures::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::binance_futures::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange="{}", )"
        R"(misc={}, )"
        R"(rest={}, )"
        R"(ws={}, )"
        R"(mbp={}, )"
        R"(request={}, )"
        R"(ws_api={}, )"
        R"(server={})"
        R"(}})"sv,
        value.exchange,
        value.misc,
        value.rest,
        value.ws,
        value.mbp,
        value.request,
        value.ws_api_2,
        static_cast<roq::server::Settings const &>(value));
  }
};
