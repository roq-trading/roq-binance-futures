/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/metrics/writer.hpp"

namespace roq {
namespace binance_futures {

struct DropCopy {
  DropCopy() = default;

  DropCopy(DropCopy const &) = delete;

  virtual ~DropCopy() = default;

  virtual void operator()(Event<Start> const &) = 0;
  virtual void operator()(Event<Stop> const &) = 0;
  virtual void operator()(Event<Timer> const &) = 0;

  virtual void operator()(metrics::Writer &) const = 0;
};

}  // namespace binance_futures
}  // namespace roq
