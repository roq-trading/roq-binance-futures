/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/fix_bridge/tools/fix_log.hpp"

namespace roq {
namespace binance_futures {
namespace fix_bridge {
namespace tools {

// === CONSTANTS ===

namespace {
auto const FLAGS = Mask{
    io::fs::Flags::WRITE_ONLY,
    io::fs::Flags::CREATE,
    io::fs::Flags::APPEND,
};
auto const MODE = Mask{
    io::fs::Mode::USER_READ_WRITE,
    io::fs::Mode::GROUP_READ_WRITE,
    io::fs::Mode::OTHERS_READ_WRITE,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_file(auto &path) -> std::unique_ptr<io::fs::File> {
  if (std::empty(path)) {
    return {};
  }
  return std::make_unique<io::fs::File>(path, FLAGS, MODE);
}
}  // namespace

// === IMPLEMENTATION ===

FIXLog::FIXLog(Settings const &settings) : settings_{settings}, raw_file_{create_file(settings.fix_bridge.log_path)} {
  if (static_cast<bool>(raw_file_)) {
    auto fd = (*raw_file_).fd();
    file_ = ::fdopen(fd, "w");
  }
}

FIXLog::~FIXLog() {
  try {
    if (file_ != nullptr) {
      ::fflush(file_);
      ::fclose(file_);
    }
  } catch (...) {
  }
}

}  // namespace tools
}  // namespace fix_bridge
}  // namespace binance_futures
}  // namespace roq
