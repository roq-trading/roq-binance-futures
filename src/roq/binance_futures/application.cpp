/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/application.hpp"

#include "roq/binance_futures/config.hpp"
#include "roq/binance_futures/gateway.hpp"
#include "roq/binance_futures/settings.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === CONSTANTS ===

namespace {
uint8_t const FAPI = 0x0;
uint8_t const DAPI = 0x1;
}  // namespace

// === HELPERS ===

namespace {

auto parse_api(auto &settings) {
  if (settings.api == "fapi"sv) {
    return FAPI;
  }
  if (settings.api == "dapi"sv) {
    return DAPI;
  }
  log::fatal(R"(Unexpected: api="{}")"sv, settings.api);
}
}  // namespace

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  auto api = parse_api(settings);
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context, api}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace binance_futures
}  // namespace roq
