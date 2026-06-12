/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/application.hpp"

#include "roq/binance_futures/gateway/config.hpp"
#include "roq/binance_futures/gateway/controller.hpp"

#include "roq/binance_futures/flags/settings.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<gateway::Controller>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace binance_futures
}  // namespace roq
