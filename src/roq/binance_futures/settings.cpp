/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

Settings::Settings(args::Parser const &args) : Settings{args, flags::Flags::create()} {
}

Settings::Settings(args::Parser const &args, flags::Flags const &flags)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, flags.api}, flags::Flags{flags}, misc{flags::Misc::create()},
      rest{flags::REST::create()}, ws{flags::WS::create()}, mbp{flags::MBP::create()}, request{flags::Request::create()}, ws_api_2{flags::WS_API::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace binance_futures
}  // namespace roq
