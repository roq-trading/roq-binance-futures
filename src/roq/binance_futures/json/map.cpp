/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/binance_futures/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// binance_futures => roq

// binance_futures::json::ContractStatus ==> roq::TradingStatus

template <>
template <>
constexpr Helper<binance_futures::json::ContractStatus>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::json::ContractStatus::type_t;
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

static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::TRADING}} == roq::TradingStatus::OPEN);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::HALT}} == roq::TradingStatus::HALT);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::BREAK}} == roq::TradingStatus::CLOSE);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::END_OF_DAY}} == roq::TradingStatus::END_OF_DAY);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::PRE_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::AUCTION_MATCH}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::POST_TRADING}} == roq::TradingStatus::CLOSE);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::SETTLING}} == roq::TradingStatus::PRE_CLOSE);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::PENDING_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{binance_futures::json::ContractStatus{binance_futures::json::ContractStatus::DELIVERING}} == roq::TradingStatus::UNDEFINED);

template <>
template <>
std::optional<roq::TradingStatus> Map<binance_futures::json::ContractStatus>::helper() const {
  return Helper{args_};
}

// binance_futures::json::ContractType ==> roq::SecurityType

template <>
template <>
constexpr Helper<binance_futures::json::ContractType>::operator std::optional<roq::SecurityType>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::json::ContractType::type_t;
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
  }
  return {};
}

static_assert(Helper{binance_futures::json::ContractType{binance_futures::json::ContractType::UNDEFINED_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(Helper{binance_futures::json::ContractType{binance_futures::json::ContractType::PERPETUAL}} == roq::SecurityType::SWAP);
static_assert(Helper{binance_futures::json::ContractType{binance_futures::json::ContractType::CURRENT_QUARTER}} == roq::SecurityType::FUTURES);
static_assert(Helper{binance_futures::json::ContractType{binance_futures::json::ContractType::NEXT_QUARTER}} == roq::SecurityType::FUTURES);
static_assert(Helper{binance_futures::json::ContractType{binance_futures::json::ContractType::CURRENT_QUARTER_DELIVERING}} == roq::SecurityType::FUTURES);
static_assert(Helper{binance_futures::json::ContractType{binance_futures::json::ContractType::PERPETUAL_DELIVERING}} == roq::SecurityType::FUTURES);

template <>
template <>
std::optional<roq::SecurityType> Map<binance_futures::json::ContractType>::helper() const {
  return Helper{args_};
}

// binance_futures::json::OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<binance_futures::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::json::OrderStatus::type_t;
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
  }
  return {};
}

static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::NEW}} == roq::OrderStatus::WORKING);
static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::EXPIRED}} == roq::OrderStatus::EXPIRED);
static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::NEW_INSURANCE}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::NEW_ADL}} == roq::OrderStatus::UNDEFINED);

template <>
template <>
std::optional<roq::OrderStatus> Map<binance_futures::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// binance_futures::json::OrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<binance_futures::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::json::OrderType::type_t;
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

static_assert(Helper{binance_futures::json::OrderType{binance_futures::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{binance_futures::json::OrderType{binance_futures::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{binance_futures::json::OrderType{binance_futures::json::OrderType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{binance_futures::json::OrderType{binance_futures::json::OrderType::STOP}} == roq::OrderType::UNDEFINED);
static_assert(Helper{binance_futures::json::OrderType{binance_futures::json::OrderType::TAKE_PROFIT}} == roq::OrderType::UNDEFINED);
static_assert(Helper{binance_futures::json::OrderType{binance_futures::json::OrderType::LIQUIDATION}} == roq::OrderType::UNDEFINED);

template <>
template <>
std::optional<roq::OrderType> Map<binance_futures::json::OrderType>::helper() const {
  return Helper{args_};
}

// binance_futures::json::Side ==> roq::Side

template <>
template <>
constexpr Helper<binance_futures::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::json::Side::type_t;
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

static_assert(Helper{binance_futures::json::Side{binance_futures::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{binance_futures::json::Side{binance_futures::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{binance_futures::json::Side{binance_futures::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<binance_futures::json::Side>::helper() const {
  return Helper{args_};
}

// binance_futures::json::SymbolStatus ==> roq::TradingStatus

template <>
template <>
constexpr Helper<binance_futures::json::SymbolStatus>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::json::SymbolStatus::type_t;
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
  }
  return {};
}

static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::TRADING}} == roq::TradingStatus::OPEN);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::HALT}} == roq::TradingStatus::HALT);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::BREAK}} == roq::TradingStatus::CLOSE);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::END_OF_DAY}} == roq::TradingStatus::END_OF_DAY);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::PRE_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::AUCTION_MATCH}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::POST_TRADING}} == roq::TradingStatus::CLOSE);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::SETTLING}} == roq::TradingStatus::PRE_CLOSE);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::PENDING_TRADING}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::DELIVERING}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{binance_futures::json::SymbolStatus{binance_futures::json::SymbolStatus::PRE_SETTLE}} == roq::TradingStatus::OPEN);

template <>
template <>
std::optional<roq::TradingStatus> Map<binance_futures::json::SymbolStatus>::helper() const {
  return Helper{args_};
}

// binance_futures::json::TimeInForce ==> roq::TimeInForce

template <>
template <>
constexpr Helper<binance_futures::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum binance_futures::json::TimeInForce::type_t;
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
  }
  return {};
}

static_assert(Helper{binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::IOC}} == roq::TimeInForce::IOC);
static_assert(Helper{binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::GTX}} == roq::TimeInForce::GTX);

template <>
template <>
std::optional<roq::TimeInForce> Map<binance_futures::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// roq ==> binance_futures

// roq::OrderStatus ==> binance_futures::json::OrderStatus

template <>
template <>
constexpr Helper<roq::OrderStatus>::operator std::optional<binance_futures::json::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderStatus;
    case UNDEFINED:
      return binance_futures::json::OrderStatus::UNDEFINED_INTERNAL;
    case SENT:
      return binance_futures::json::OrderStatus::UNDEFINED_INTERNAL;
    case ACCEPTED:
      return binance_futures::json::OrderStatus::UNDEFINED_INTERNAL;
    case SUSPENDED:
      return binance_futures::json::OrderStatus::UNDEFINED_INTERNAL;
    case WORKING:
      return binance_futures::json::OrderStatus::NEW;
      // return json::OrderStatus::PARTIALLY_FILLED;
    case STOPPED:
      return binance_futures::json::OrderStatus::UNDEFINED_INTERNAL;
    case COMPLETED:
      return binance_futures::json::OrderStatus::FILLED;
    case EXPIRED:
      return binance_futures::json::OrderStatus::EXPIRED;
    case CANCELED:
      return binance_futures::json::OrderStatus::CANCELED;
    case REJECTED:
      return binance_futures::json::OrderStatus::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::OrderStatus::UNDEFINED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::SENT} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::ACCEPTED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::SUSPENDED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::WORKING} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::NEW});
static_assert(Helper{roq::OrderStatus::STOPPED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderStatus::COMPLETED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::FILLED});
static_assert(Helper{roq::OrderStatus::EXPIRED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::EXPIRED});
static_assert(Helper{roq::OrderStatus::CANCELED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::CANCELED});
static_assert(Helper{roq::OrderStatus::REJECTED} == binance_futures::json::OrderStatus{binance_futures::json::OrderStatus::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<binance_futures::json::OrderStatus> Map<roq::OrderStatus>::helper() const {
  return Helper{args_};
}

// roq::OrderType ==> binance_futures::json::OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<binance_futures::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return binance_futures::json::OrderType::UNDEFINED_INTERNAL;
    case MARKET:
      return binance_futures::json::OrderType::MARKET;
    case LIMIT:
      return binance_futures::json::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == binance_futures::json::OrderType{binance_futures::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == binance_futures::json::OrderType{binance_futures::json::OrderType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == binance_futures::json::OrderType{binance_futures::json::OrderType::LIMIT});

template <>
template <>
std::optional<binance_futures::json::OrderType> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

// roq::Side ==> binance_futures::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<binance_futures::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return binance_futures::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return binance_futures::json::Side::BUY;
    case SELL:
      return binance_futures::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == binance_futures::json::Side{binance_futures::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == binance_futures::json::Side{binance_futures::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == binance_futures::json::Side{binance_futures::json::Side::SELL});

template <>
template <>
std::optional<binance_futures::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce ==> binance_futures::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<binance_futures::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTC:
      return binance_futures::json::TimeInForce::GTC;
    case OPG:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return binance_futures::json::TimeInForce::IOC;
    case FOK:
      return binance_futures::json::TimeInForce::FOK;
    case GTX:
      return binance_futures::json::TimeInForce::GTX;
    case GTD:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return binance_futures::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTC} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::GTC});
static_assert(Helper{roq::TimeInForce::OPG} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::FOK});
static_assert(Helper{roq::TimeInForce::GTX} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::GTX});
static_assert(Helper{roq::TimeInForce::GTD} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == binance_futures::json::TimeInForce{binance_futures::json::TimeInForce::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<binance_futures::json::TimeInForce> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
