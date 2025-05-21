/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/api.hpp"

#include "roq/server/oms/order.hpp"

#include "roq/metrics/writer.hpp"

namespace roq {
namespace binance_futures {

struct OrderEntry {
  OrderEntry() = default;

  OrderEntry(OrderEntry const &) = delete;

  virtual ~OrderEntry() = default;

  virtual void operator()(Event<Start> const &) = 0;
  virtual void operator()(Event<Stop> const &) = 0;
  virtual void operator()(Event<Timer> const &) = 0;

  virtual void operator()(metrics::Writer &) const = 0;

  virtual uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id) = 0;
  virtual uint16_t operator()(
      Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id) = 0;
  virtual uint16_t operator()(
      Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id) = 0;

  virtual uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) = 0;
};

}  // namespace binance_futures
}  // namespace roq
