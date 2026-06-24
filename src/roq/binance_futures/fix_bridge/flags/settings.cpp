/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/fix_bridge/settings.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace fix_bridge {
namespace flags {

// === IMPLEMENTATION ===

Settings::Settings(args::Parser const &parser) : binance_futures::flags::Settings{parser}, fix_bridge{flags::FIXBridge::create()} {
}

}  // namespace flags
}  // namespace fix_bridge
}  // namespace binance_futures
}  // namespace roq
