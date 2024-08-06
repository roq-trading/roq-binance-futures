/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/binance_futures/json/map.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace binance_futures {
namespace json {

// === HELPERS ===

namespace {
// note! constexpr helper for static testing
template <typename... Args>
struct Helper final {
  explicit constexpr Helper(std::tuple<Args...> const &args) : args_{args} {}
  explicit constexpr Helper(Args &&...args_) : args_{std::forward<Args>(args_)...} {}

  template <typename R>
  constexpr operator R();

 private:
  std::tuple<Args...> const args_;
};

// ==> roq

// OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<OrderStatus>::operator roq::OrderStatus() {
  switch (std::get<0>(args_)) {
    using enum json::OrderStatus::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case NEW:
      return roq::OrderStatus::WORKING;
    case PARTIALLY_FILLED:
      return roq::OrderStatus::WORKING;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
    case EXPIRED:
      return roq::OrderStatus::EXPIRED;
    case NEW_INSURANCE:
      return {};  // note!
    case NEW_ADL:
      return {};  // note!
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::UNDEFINED__}}) == roq::OrderStatus::UNDEFINED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::NEW}}) == roq::OrderStatus::WORKING);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::PARTIALLY_FILLED}}) == roq::OrderStatus::WORKING);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::FILLED}}) == roq::OrderStatus::COMPLETED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::CANCELED}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::EXPIRED}}) == roq::OrderStatus::EXPIRED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::NEW_INSURANCE}}) == roq::OrderStatus::UNDEFINED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::NEW_ADL}}) == roq::OrderStatus::UNDEFINED);

// OrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<OrderType>::operator roq::OrderType() {
  switch (std::get<0>(args_)) {
    using enum json::OrderType::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case MARKET:
      return roq::OrderType::MARKET;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case STOP:
      return {};  // note!
    case TAKE_PROFIT:
      return {};  // note!
    case LIQUIDATION:
      return {};  // note!
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::UNDEFINED__}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::LIMIT}}) == roq::OrderType::LIMIT);
static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::MARKET}}) == roq::OrderType::MARKET);
static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::STOP}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::TAKE_PROFIT}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::LIQUIDATION}}) == roq::OrderType::UNDEFINED);

// Side ==> roq::Side

template <>
template <>
constexpr Helper<Side>::operator roq::Side() {
  switch (std::get<0>(args_)) {
    using enum json::Side::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::Side>(Helper{Side{Side::UNDEFINED__}}) == roq::Side::UNDEFINED);
static_assert(static_cast<roq::Side>(Helper{Side{Side::BUY}}) == roq::Side::BUY);
static_assert(static_cast<roq::Side>(Helper{Side{Side::SELL}}) == roq::Side::SELL);

// ContractType ==> roq::SecurityType

template <>
template <>
constexpr Helper<ContractType>::operator roq::SecurityType() {
  switch (std::get<0>(args_)) {
    using enum json::ContractType::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case PERPETUAL:
      return roq::SecurityType::SWAP;
    case CURRENT_QUARTER:
      return roq::SecurityType::FUTURES;
    case NEXT_QUARTER:
      return roq::SecurityType::FUTURES;
    case CURRENT_QUARTER_DELIVERING:
      return roq::SecurityType::FUTURES;
    case PERPETUAL_DELIVERING:
      return roq::SecurityType::FUTURES;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::SecurityType>(Helper{ContractType{ContractType::UNDEFINED__}}) == roq::SecurityType::UNDEFINED);
static_assert(static_cast<roq::SecurityType>(Helper{ContractType{ContractType::PERPETUAL}}) == roq::SecurityType::SWAP);
static_assert(static_cast<roq::SecurityType>(Helper{ContractType{ContractType::CURRENT_QUARTER}}) == roq::SecurityType::FUTURES);
static_assert(static_cast<roq::SecurityType>(Helper{ContractType{ContractType::NEXT_QUARTER}}) == roq::SecurityType::FUTURES);
static_assert(static_cast<roq::SecurityType>(Helper{ContractType{ContractType::CURRENT_QUARTER_DELIVERING}}) == roq::SecurityType::FUTURES);
static_assert(static_cast<roq::SecurityType>(Helper{ContractType{ContractType::PERPETUAL_DELIVERING}}) == roq::SecurityType::FUTURES);

// SymbolStatus ==> roq::Side

template <>
template <>
constexpr Helper<SymbolStatus>::operator roq::TradingStatus() {
  switch (std::get<0>(args_)) {
    using enum json::SymbolStatus::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case TRADING:
      return roq::TradingStatus::OPEN;
    case HALT:
      return roq::TradingStatus::HALT;
    case BREAK:
      return roq::TradingStatus::CLOSE;
    case END_OF_DAY:
      return roq::TradingStatus::END_OF_DAY;
      // note! following probably not used (not sure if also applies to futures)
      // - https://dev.binance.vision/t/explanation-on-symbol-status/118
    case PRE_TRADING:
      return roq::TradingStatus::PRE_OPEN;
    case AUCTION_MATCH:
      return roq::TradingStatus::PRE_OPEN;
    case POST_TRADING:
      return roq::TradingStatus::CLOSE;
      // note! have found no documentation
    case SETTLING:                           // note! no idea what this is for
      return roq::TradingStatus::PRE_CLOSE;  // XXX REVIEW
    case PENDING_TRADING:                    // note! no idea what this is for
      return roq::TradingStatus::PRE_OPEN;   // XXX REVIEW
    case DELIVERING:
      return roq::TradingStatus::UNDEFINED;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::UNDEFINED__}}) == roq::TradingStatus::UNDEFINED);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::TRADING}}) == roq::TradingStatus::OPEN);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::HALT}}) == roq::TradingStatus::HALT);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::BREAK}}) == roq::TradingStatus::CLOSE);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::END_OF_DAY}}) == roq::TradingStatus::END_OF_DAY);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::PRE_TRADING}}) == roq::TradingStatus::PRE_OPEN);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::AUCTION_MATCH}}) == roq::TradingStatus::PRE_OPEN);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::POST_TRADING}}) == roq::TradingStatus::CLOSE);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::SETTLING}}) == roq::TradingStatus::PRE_CLOSE);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::PENDING_TRADING}}) == roq::TradingStatus::PRE_OPEN);
static_assert(static_cast<roq::TradingStatus>(Helper{SymbolStatus{SymbolStatus::DELIVERING}}) == roq::TradingStatus::UNDEFINED);

// TimeInForce ==> roq::TimeInForce

template <>
template <>
constexpr Helper<TimeInForce>::operator roq::TimeInForce() {
  switch (std::get<0>(args_)) {
    using enum json::TimeInForce::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case GTC:
      return roq::TimeInForce::GTC;
    case IOC:
      return roq::TimeInForce::IOC;
    case FOK:
      return roq::TimeInForce::FOK;
    case GTX:
      return roq::TimeInForce::GTX;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::UNDEFINED__}}) == roq::TimeInForce::UNDEFINED);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::GTC}}) == roq::TimeInForce::GTC);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::IOC}}) == roq::TimeInForce::IOC);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::FOK}}) == roq::TimeInForce::FOK);
static_assert(static_cast<roq::TimeInForce>(Helper{TimeInForce{TimeInForce::GTX}}) == roq::TimeInForce::GTX);

// roq ==>

// roq::Side ==> Side

template <>
template <>
constexpr Helper<roq::Side>::operator Side() {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return {};
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<Side>(Helper{roq::Side::UNDEFINED}) == Side::UNDEFINED__);
static_assert(static_cast<Side>(Helper{roq::Side::BUY}) == Side::BUY);
static_assert(static_cast<Side>(Helper{roq::Side::SELL}) == Side::SELL);

// roq::OrderStatus ==> OrderStatus

template <>
template <>
constexpr Helper<roq::OrderStatus>::operator OrderStatus() {
  switch (std::get<0>(args_)) {
    using enum roq::OrderStatus;
    case UNDEFINED:
      return {};
    case SENT:
      return {};  // note!
    case ACCEPTED:
      return {};  // note!
    case SUSPENDED:
      return {};  // note!
    case WORKING:
      return json::OrderStatus::NEW;
      // return json::OrderStatus::PARTIALLY_FILLED;
    case STOPPED:
      return {};  // note!
    case COMPLETED:
      return json::OrderStatus::FILLED;
    case EXPIRED:
      return json::OrderStatus::EXPIRED;
    case CANCELED:
      return json::OrderStatus::CANCELED;
    case REJECTED:
      return {};  // note!
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::UNDEFINED}) == OrderStatus::UNDEFINED__);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::SENT}) == OrderStatus::UNDEFINED__);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::ACCEPTED}) == OrderStatus::UNDEFINED__);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::SUSPENDED}) == OrderStatus::UNDEFINED__);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::WORKING}) == OrderStatus::NEW);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::STOPPED}) == OrderStatus::UNDEFINED__);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::COMPLETED}) == OrderStatus::FILLED);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::EXPIRED}) == OrderStatus::EXPIRED);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::CANCELED}) == OrderStatus::CANCELED);
static_assert(static_cast<OrderStatus>(Helper{roq::OrderStatus::REJECTED}) == OrderStatus::UNDEFINED__);

// roq::OrderType ==> OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator OrderType() {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return {};
    case MARKET:
      return json::OrderType::MARKET;
    case LIMIT:
      return json::OrderType::LIMIT;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<OrderType>(Helper{roq::OrderType::UNDEFINED}) == OrderType::UNDEFINED__);
static_assert(static_cast<OrderType>(Helper{roq::OrderType::MARKET}) == OrderType::MARKET);
static_assert(static_cast<OrderType>(Helper{roq::OrderType::LIMIT}) == OrderType::LIMIT);

// roq::TimeInForce ==> TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator TimeInForce() {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return {};
    case GFD:
      return {};  // note!
    case GTC:
      return json::TimeInForce::GTC;
    case OPG:
      return {};  // note!
    case IOC:
      return json::TimeInForce::IOC;
    case FOK:
      return json::TimeInForce::FOK;
    case GTX:
      return json::TimeInForce::GTX;
    case GTD:
      return {};  // note!
    case AT_THE_CLOSE:
      return {};  // note!
    case GOOD_THROUGH_CROSSING:
      return {};  // note!
    case AT_CROSSING:
      return {};  // note!
    case GOOD_FOR_TIME:
      return {};  // note!
    case GFA:
      return {};  // note!
    case GFM:
      return {};  // note!
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::UNDEFINED}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GFD}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GTC}) == TimeInForce::GTC);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::OPG}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::IOC}) == TimeInForce::IOC);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::FOK}) == TimeInForce::FOK);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GTX}) == TimeInForce::GTX);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GTD}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::AT_THE_CLOSE}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::AT_CROSSING}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GOOD_FOR_TIME}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GFA}) == TimeInForce::UNDEFINED__);
static_assert(static_cast<TimeInForce>(Helper{roq::TimeInForce::GFM}) == TimeInForce::UNDEFINED__);
}  // namespace

// === IMPLEMENTATION ===

// ==> roq

template <>
template <>
Map<ContractType>::operator roq::SecurityType() {
  return Helper{args_};
}

template <>
template <>
Map<OrderStatus>::operator roq::OrderStatus() {
  return Helper{args_};
}

template <>
template <>
Map<OrderType>::operator roq::OrderType() {
  return Helper{args_};
}

template <>
template <>
Map<Side>::operator roq::Side() {
  return Helper{args_};
}

template <>
template <>
Map<SymbolStatus>::operator roq::TradingStatus() {
  return Helper{args_};
}

template <>
template <>
Map<TimeInForce>::operator roq::TimeInForce() {
  return Helper{args_};
}

// roq ==>

template <>
template <>
Map<roq::OrderStatus>::operator OrderStatus() {
  return Helper{args_};
}

template <>
template <>
Map<roq::OrderType>::operator OrderType() {
  return Helper{args_};
}

template <>
template <>
Map<roq::Side>::operator Side() {
  return Helper{args_};
}

template <>
template <>
Map<roq::TimeInForce>::operator TimeInForce() {
  return Helper{args_};
}

}  // namespace json
}  // namespace binance_futures
}  // namespace roq
