/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/application.hpp"

#include "roq/binance_futures/config.hpp"
#include "roq/binance_futures/gateway.hpp"
#include "roq/binance_futures/settings.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace binance_futures
}  // namespace roq
