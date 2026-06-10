/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/binance_futures/protocol/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// binance_futures => roq

// binance_futures::protocol::json::ContractStatus ==> roq::TradingStatus

template <>
template <>
constexpr Helper<binance_futures::protocol::json::ContractStatus>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::ContractStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
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
  return {};
}

static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::UNDEFINED_INTERNAL}} ==
    roq::TradingStatus::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::TRADING}} == roq::TradingStatus::OPEN);
static_assert(Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::HALT}} == roq::TradingStatus::HALT);
static_assert(Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::BREAK}} == roq::TradingStatus::CLOSE);
static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::END_OF_DAY}} == roq::TradingStatus::END_OF_DAY);
static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::PRE_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::AUCTION_MATCH}} == roq::TradingStatus::PRE_OPEN);
static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::POST_TRADING}} == roq::TradingStatus::CLOSE);
static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::SETTLING}} == roq::TradingStatus::PRE_CLOSE);
static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::PENDING_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(
    Helper{binance_futures::protocol::json::ContractStatus{binance_futures::protocol::json::ContractStatus::DELIVERING}} == roq::TradingStatus::UNDEFINED);

template <>
template <>
std::optional<roq::TradingStatus> Map<binance_futures::protocol::json::ContractStatus>::helper() const {
  return Helper{args_};
}

// binance_futures::protocol::json::ContractType ==> roq::SecurityType

template <>
template <>
constexpr Helper<binance_futures::protocol::json::ContractType>::operator std::optional<roq::SecurityType>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::ContractType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::SecurityType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::SecurityType::UNDEFINED;
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
    case TRADIFI_PERPETUAL:
      return roq::SecurityType::SWAP;
  }
  return {};
}

static_assert(
    Helper{binance_futures::protocol::json::ContractType{binance_futures::protocol::json::ContractType::UNDEFINED_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::ContractType{binance_futures::protocol::json::ContractType::PERPETUAL}} == roq::SecurityType::SWAP);
static_assert(
    Helper{binance_futures::protocol::json::ContractType{binance_futures::protocol::json::ContractType::CURRENT_QUARTER}} == roq::SecurityType::FUTURES);
static_assert(Helper{binance_futures::protocol::json::ContractType{binance_futures::protocol::json::ContractType::NEXT_QUARTER}} == roq::SecurityType::FUTURES);
static_assert(
    Helper{binance_futures::protocol::json::ContractType{binance_futures::protocol::json::ContractType::CURRENT_QUARTER_DELIVERING}} ==
    roq::SecurityType::FUTURES);
static_assert(
    Helper{binance_futures::protocol::json::ContractType{binance_futures::protocol::json::ContractType::PERPETUAL_DELIVERING}} == roq::SecurityType::FUTURES);
static_assert(
    Helper{binance_futures::protocol::json::ContractType{binance_futures::protocol::json::ContractType::TRADIFI_PERPETUAL}} == roq::SecurityType::SWAP);

template <>
template <>
std::optional<roq::SecurityType> Map<binance_futures::protocol::json::ContractType>::helper() const {
  return Helper{args_};
}

// binance_futures::protocol::json::OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<binance_futures::protocol::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
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
      return roq::OrderStatus::UNDEFINED;
    case NEW_ADL:
      return roq::OrderStatus::UNDEFINED;
    case EXPIRED_IN_MATCH:
      return roq::OrderStatus::EXPIRED;
  }
  return {};
}

static_assert(
    Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::NEW}} == roq::OrderStatus::WORKING);
static_assert(
    Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::EXPIRED}} == roq::OrderStatus::EXPIRED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::NEW_INSURANCE}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::NEW_ADL}} == roq::OrderStatus::UNDEFINED);
static_assert(
    Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::EXPIRED_IN_MATCH}} == roq::OrderStatus::EXPIRED);

template <>
template <>
std::optional<roq::OrderStatus> Map<binance_futures::protocol::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// binance_futures::protocol::json::OrderStatus ==> roq::RequestStatus

template <>
template <>
constexpr Helper<binance_futures::protocol::json::OrderStatus>::operator std::optional<RequestStatus>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return RequestStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return RequestStatus::UNDEFINED;
    case NEW:
      return RequestStatus::ACCEPTED;
    case PARTIALLY_FILLED:
      return RequestStatus::ACCEPTED;
    case FILLED:
      return RequestStatus::REJECTED;
    case CANCELED:
      return RequestStatus::REJECTED;
    case EXPIRED:
      return RequestStatus::REJECTED;
    case NEW_INSURANCE:
      return RequestStatus::REJECTED;
    case NEW_ADL:
      return RequestStatus::REJECTED;
    case EXPIRED_IN_MATCH:
      return RequestStatus::REJECTED;
  }
  return {};
}

static_assert(
    Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL}} == RequestStatus::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::NEW}} == RequestStatus::ACCEPTED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::PARTIALLY_FILLED}} == RequestStatus::ACCEPTED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::FILLED}} == RequestStatus::REJECTED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::CANCELED}} == RequestStatus::REJECTED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::EXPIRED}} == RequestStatus::REJECTED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::NEW_INSURANCE}} == RequestStatus::REJECTED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::NEW_ADL}} == RequestStatus::REJECTED);
static_assert(Helper{binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::EXPIRED_IN_MATCH}} == RequestStatus::REJECTED);

template <>
template <>
std::optional<RequestStatus> Map<binance_futures::protocol::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// binance_futures::protocol::json::OrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<binance_futures::protocol::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case MARKET:
      return roq::OrderType::MARKET;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case STOP:
      return roq::OrderType::UNDEFINED;
    case TAKE_PROFIT:
      return roq::OrderType::UNDEFINED;
    case LIQUIDATION:
      return roq::OrderType::UNDEFINED;
  }
  return {};
}

static_assert(Helper{binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::STOP}} == roq::OrderType::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::TAKE_PROFIT}} == roq::OrderType::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::LIQUIDATION}} == roq::OrderType::UNDEFINED);

template <>
template <>
std::optional<roq::OrderType> Map<binance_futures::protocol::json::OrderType>::helper() const {
  return Helper{args_};
}

// binance_futures::protocol::json::Side ==> roq::Side

template <>
template <>
constexpr Helper<binance_futures::protocol::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::Side::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{binance_futures::protocol::json::Side{binance_futures::protocol::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::Side{binance_futures::protocol::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{binance_futures::protocol::json::Side{binance_futures::protocol::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<binance_futures::protocol::json::Side>::helper() const {
  return Helper{args_};
}

// binance_futures::protocol::json::SymbolStatus ==> roq::TradingStatus

template <>
template <>
constexpr Helper<binance_futures::protocol::json::SymbolStatus>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::SymbolStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
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
    case PRE_SETTLE:
      return roq::TradingStatus::OPEN;  // XXX REVIEW
    case TRADING_HALT:
      return roq::TradingStatus::HALT;
    case TRADING_CANCEL_ONLY:
      return roq::TradingStatus::HALT;
  }
  return {};
}

static_assert(
    Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::TRADING}} == roq::TradingStatus::OPEN);
static_assert(Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::HALT}} == roq::TradingStatus::HALT);
static_assert(Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::BREAK}} == roq::TradingStatus::CLOSE);
static_assert(
    Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::END_OF_DAY}} == roq::TradingStatus::END_OF_DAY);
static_assert(
    Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::PRE_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(
    Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::AUCTION_MATCH}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::POST_TRADING}} == roq::TradingStatus::CLOSE);
static_assert(Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::SETTLING}} == roq::TradingStatus::PRE_CLOSE);
static_assert(
    Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::PENDING_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(
    Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::DELIVERING}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::PRE_SETTLE}} == roq::TradingStatus::OPEN);
static_assert(Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::TRADING_HALT}} == roq::TradingStatus::HALT);
static_assert(
    Helper{binance_futures::protocol::json::SymbolStatus{binance_futures::protocol::json::SymbolStatus::TRADING_CANCEL_ONLY}} == roq::TradingStatus::HALT);

template <>
template <>
std::optional<roq::TradingStatus> Map<binance_futures::protocol::json::SymbolStatus>::helper() const {
  return Helper{args_};
}

// binance_futures::protocol::json::TimeInForce ==> roq::TimeInForce

template <>
template <>
constexpr Helper<binance_futures::protocol::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::protocol::json::TimeInForce::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case GTC:
      return roq::TimeInForce::GTC;
    case IOC:
      return roq::TimeInForce::IOC;
    case FOK:
      return roq::TimeInForce::FOK;
    case GTX:
      return roq::TimeInForce::GTX;
    case OTC:
      return roq::TimeInForce::UNDEFINED;
  }
  return {};
}

static_assert(
    Helper{binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::IOC}} == roq::TimeInForce::IOC);
static_assert(Helper{binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::GTX}} == roq::TimeInForce::GTX);
static_assert(Helper{binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::OTC}} == roq::TimeInForce::UNDEFINED);

template <>
template <>
std::optional<roq::TimeInForce> Map<binance_futures::protocol::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// roq ==> binance_futures

// roq::OrderStatus ==> binance_futures::protocol::json::OrderStatus

template <>
template <>
constexpr Helper<roq::OrderStatus>::operator std::optional<binance_futures::protocol::json::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderStatus;
    case UNDEFINED:
      return binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL;
    case SENT:
      return binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL;
    case ACCEPTED:
      return binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL;
    case SUSPENDED:
      return binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL;
    case WORKING:
      return binance_futures::protocol::json::OrderStatus::NEW;
      // return json::OrderStatus::PARTIALLY_FILLED;
    case STOPPED:
      return binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL;
    case COMPLETED:
      return binance_futures::protocol::json::OrderStatus::FILLED;
    case EXPIRED:
      return binance_futures::protocol::json::OrderStatus::EXPIRED;
    case CANCELED:
      return binance_futures::protocol::json::OrderStatus::CANCELED;
    case REJECTED:
      return binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(
    Helper{roq::OrderStatus::UNDEFINED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::SENT} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::OrderStatus::ACCEPTED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::OrderStatus::SUSPENDED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::WORKING} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::NEW});
static_assert(
    Helper{roq::OrderStatus::STOPPED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::COMPLETED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::FILLED});
static_assert(Helper{roq::OrderStatus::EXPIRED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::EXPIRED});
static_assert(Helper{roq::OrderStatus::CANCELED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::CANCELED});
static_assert(
    Helper{roq::OrderStatus::REJECTED} == binance_futures::protocol::json::OrderStatus{binance_futures::protocol::json::OrderStatus::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<binance_futures::protocol::json::OrderStatus> Map<roq::OrderStatus>::helper() const {
  return Helper{args_};
}

// roq::OrderType ==> binance_futures::protocol::json::OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<binance_futures::protocol::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return binance_futures::protocol::json::OrderType::UNDEFINED_INTERNAL;
    case MARKET:
      return binance_futures::protocol::json::OrderType::MARKET;
    case LIMIT:
      return binance_futures::protocol::json::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == binance_futures::protocol::json::OrderType{binance_futures::protocol::json::OrderType::LIMIT});

template <>
template <>
std::optional<binance_futures::protocol::json::OrderType> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

// roq::Side ==> binance_futures::protocol::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<binance_futures::protocol::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return binance_futures::protocol::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return binance_futures::protocol::json::Side::BUY;
    case SELL:
      return binance_futures::protocol::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == binance_futures::protocol::json::Side{binance_futures::protocol::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == binance_futures::protocol::json::Side{binance_futures::protocol::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == binance_futures::protocol::json::Side{binance_futures::protocol::json::Side::SELL});

template <>
template <>
std::optional<binance_futures::protocol::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// {roq::TimeInForce, Mask<roq::ExecutionInstruction>} ==> binance_futures::protocol::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce, Mask<ExecutionInstruction>>::operator std::optional<binance_futures::protocol::json::TimeInForce>() const {
  auto [time_in_force, execution_instructions] = args_;
  switch (time_in_force) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTC:
      if (execution_instructions.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE)) {
        return binance_futures::protocol::json::TimeInForce::GTX;
      }
      return binance_futures::protocol::json::TimeInForce::GTC;
    case OPG:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return binance_futures::protocol::json::TimeInForce::IOC;
    case FOK:
      return binance_futures::protocol::json::TimeInForce::FOK;
    case GTX:
      return binance_futures::protocol::json::TimeInForce::GTX;
    case GTD:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(
    Helper{roq::TimeInForce::UNDEFINED, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GFD, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GTC, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::GTC});
static_assert(
    Helper{roq::TimeInForce::OPG, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::IOC, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::IOC});
static_assert(
    Helper{roq::TimeInForce::FOK, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::FOK});
static_assert(
    Helper{roq::TimeInForce::GTX, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::GTX});
static_assert(
    Helper{roq::TimeInForce::GTD, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::AT_THE_CLOSE, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::AT_CROSSING, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GOOD_FOR_TIME, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GFA, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(
    Helper{roq::TimeInForce::GFM, Mask<ExecutionInstruction>{}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::UNDEFINED_INTERNAL});

// special

static_assert(
    Helper{roq::TimeInForce::GTC, Mask{ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE}} ==
    binance_futures::protocol::json::TimeInForce{binance_futures::protocol::json::TimeInForce::GTX});

template <>
template <>
std::optional<binance_futures::protocol::json::TimeInForce> Map<roq::TimeInForce, Mask<ExecutionInstruction>>::helper() const {
  return Helper{args_};
}

}  // namespace roq
