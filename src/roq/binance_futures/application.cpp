/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/binance_futures/application.hpp"

#include "roq/binance_futures/config.hpp"
#include "roq/binance_futures/flags.hpp"
#include "roq/binance_futures/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {

// === HELPERS ===

namespace {
auto get_settings = [](auto &api) {
  return server::Settings{
      .package_name = ROQ_PACKAGE_NAME,
      .build_number = ROQ_BUILD_NUMBER,
      .api = api,
      .type = server::Type::ORDER_MANAGEMENT,
  };
};
}

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  Flags2 flags;
  Config config{flags};
  auto context = server::create_io_context();
  auto settings = get_settings(flags.api);
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace binance_futures
}  // namespace roq
