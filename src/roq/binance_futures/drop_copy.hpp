/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/metrics/writer.hpp"

namespace roq {
namespace binance_futures {

struct DropCopy {
  virtual ~DropCopy() {}

  virtual void operator()(Event<Start> const &) = 0;
  virtual void operator()(Event<Stop> const &) = 0;
  virtual void operator()(Event<Timer> const &) = 0;

  virtual void operator()(metrics::Writer &) = 0;
};

}  // namespace binance_futures
}  // namespace roq
