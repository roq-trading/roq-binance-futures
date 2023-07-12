/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/settings.hpp"

#include "roq/logging.hpp"

#include "roq/binance_futures/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, flags::Flags::api()},
      exchange{flags::Flags::exchange()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace binance_futures
}  // namespace roq
