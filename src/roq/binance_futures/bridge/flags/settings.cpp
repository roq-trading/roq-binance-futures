/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/bridge/settings.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace bridge {
namespace flags {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &parser)
    : binance_futures::flags::Settings{parser}, fix{flags::FIX::create()}, messaging{flags::Messaging::create()}, oms{flags::OMS::create()},
      test{flags::Test::create()} {
}

}  // namespace flags
}  // namespace bridge
}  // namespace binance_futures
}  // namespace roq
