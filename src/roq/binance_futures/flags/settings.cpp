/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/flags/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace flags {

Settings::Settings(args::Parser const &args) : Settings{args, flags::Flags::create()} {
}

Settings::Settings(args::Parser const &args, flags::Flags const &flags)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, flags.api}, flags::Flags{flags}, misc{flags::Misc::create()},
      rest{flags::REST::create()}, ws{flags::WS::create()}, mbp{flags::MBP::create()}, request{flags::Request::create()}, ws_api_2{flags::WS_API::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace flags
}  // namespace binance_futures
}  // namespace roq
