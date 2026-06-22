/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/bridge/application.hpp"

#include "roq/logging.hpp"

#include "roq/binance_futures/bridge/config.hpp"
#include "roq/binance_futures/bridge/controller.hpp"
#include "roq/binance_futures/bridge/settings.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace bridge {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  log::warn("config={}"sv, config);
  auto context = server::create_io_context(settings);
  Controller{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bridge
}  // namespace binance_futures
}  // namespace roq
