/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/binance_futures/drop_copy.h"

#include <algorithm>

#include "roq/utils/mask.h"
#include "roq/utils/safe_cast.h"
#include "roq/utils/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/binance_futures/flags.h"

#include "roq/binance_futures/json/utils.h"

using namespace std::literals;

namespace roq {
namespace binance_futures {

namespace {
const auto NAME = "ex"sv;
const auto SUPPORTS = utils::Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::FUNDS,
    SupportType::POSITION,
};

auto create_uri(const std::string_view &listen_key) {
  assert(!std::empty(listen_key));
  // XXX HANS make it easier to append to path
  auto &uri = Flags::ws_uri();
  auto result = fmt::format("{}://{}{}/{}"sv, uri.scheme, uri.host, uri.path, listen_key);
  return core::URI{result};
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

DropCopy::DropCopy(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared,
    const std::string_view &listen_key)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          create_uri(listen_key),
          {},
          Flags::ws_ping_freq(),
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          []() { return std::string(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .order_trade_update = create_metrics(name_, "order_trade_update"sv),
          .account_update = create_metrics(name_, "account_update"sv),
          .margin_call = create_metrics(name_, "margin_call"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      security_(security), shared_(shared),
      download_({}, [this](auto state) { return download(state); }) {
}

bool DropCopy::ready() const {
  return connection_.ready();
}

void DropCopy::operator()(const Event<Start> &) {
  connection_.start();
}

void DropCopy::operator()(const Event<Stop> &) {
  connection_.stop();
}

void DropCopy::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.order_trade_update, metrics::PROFILE)
      .write(profile_.account_update, metrics::PROFILE)
      .write(profile_.margin_call, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(const core::web::ClientSocket::Connected &) {
}

void DropCopy::operator()(const core::web::ClientSocket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void DropCopy::operator()(const core::web::ClientSocket::Ready &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopy::operator()(const core::web::ClientSocket::Close &) {
}

void DropCopy::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    case DropCopyState::UNDEFINED:
      assert(false);
      break;
    case DropCopyState::DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void DropCopy::parse(const std::string_view &message) {
  profile_.parse([&]() {
    try {
      log::debug(R"(message="{}")"sv, message);
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::UserStreamParser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(const server::Trace<json::OrderTradeUpdate> &event) {
  profile_.order_trade_update([&]() {
    // auto &[trace_info, order_trade_update] = event;
    auto &trace_info = event.trace_info;
    auto &order_trade_update = event.value;
    log::debug("order_trade_update={}"sv, order_trade_update);
    log::info<3>("order_trade_update={}"sv, order_trade_update);
    auto &execution_report = order_trade_update.execution_report;
    auto side = json::map(execution_report.side);
    auto external_order_id = fmt::format("{}"sv, execution_report.order_id);  // XXX HANS
    auto order_status = json::map(execution_report.order_status);
    auto order_type = json::map(execution_report.order_type);
    auto time_in_force = json::map(execution_report.time_in_force);
    auto liquidity = execution_report.is_trade_maker ? Liquidity::MAKER : Liquidity::TAKER;
    oms::OrderUpdate order_update{
        .account = security_.get_account(),
        .exchange = Flags::exchange(),
        .symbol = execution_report.symbol,
        .side = side,
        .position_effect = {},
        .max_show_quantity = NaN,
        .order_type = order_type,
        .time_in_force = time_in_force,
        .execution_instruction = {},
        .order_template = {},
        .create_time_utc = {},
        .update_time_utc = utils::safe_cast(order_trade_update.event_time),
        .external_account = {},
        .external_order_id = external_order_id,
        .status = order_status,
        .quantity = execution_report.original_quantity,
        .price = execution_report.original_price,
        .stop_price = execution_report.stop_price,
        .remaining_quantity = NaN,
        .traded_quantity = execution_report.order_filled_accumulated_quantity,
        .average_traded_price = execution_report.average_price,
        .last_traded_quantity = execution_report.last_filled_quantity,
        .last_traded_price = execution_report.last_filled_price,
        .last_liquidity = liquidity,
    };
    if (shared_.update_order(
            execution_report.client_order_id,
            stream_id_,
            trace_info,
            order_update,
            [&](auto &order) {
              if (execution_report.execution_type == json::ExecutionType::TRADE) {
                auto external_trade_id =
                    fmt::format("{}"sv, execution_report.trade_id);  // XXX HANS
                Fill fill{
                    .external_trade_id = {},
                    .quantity = execution_report.last_filled_quantity,
                    .price = execution_report.last_filled_price,
                };
                TradeUpdate trade_update{
                    .stream_id = stream_id_,
                    .account = order.account,
                    .order_id = order.order_id,
                    .exchange = order.exchange,
                    .symbol = order.symbol,
                    .side = order.side,
                    .position_effect = order.position_effect,
                    .create_time_utc = utils::safe_cast(execution_report.order_trade_time),
                    .update_time_utc = utils::safe_cast(execution_report.order_trade_time),
                    .external_account = order.external_account,
                    .external_order_id = order.external_order_id,
                    .fills = {&fill, 1},
                    .routing_id = order.routing_id,
                };
                server::create_trace_and_dispatch(
                    handler_, trace_info, trade_update, true, order.user_id);
              }
            })) {
    } else {
      log::warn("*** EXTERNAL ORDER ***"sv);
      log::warn("execution_report={}"sv, execution_report);
    }
  });
}

void DropCopy::operator()(const server::Trace<json::AccountUpdate> &event) {
  profile_.account_update([&]() {
    auto &[trace_info, account_update] = event;
    log::info<2>("account_update={}"sv, account_update);
    for (auto &item : account_update.data.balances) {
      log::debug("item={}"sv, item);
      FundsUpdate funds_update{
          .stream_id = stream_id_,
          .account = security_.get_account(),
          .currency = item.asset,
          .balance = item.wallet_balance,
          .hold = NaN,  // note! we don't see this
          .external_account = {},
      };
      create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    }
    for (auto &item : account_update.data.positions) {
      if (shared_.discard_symbol(item.symbol))
        continue;
      log::debug("item={}"sv, item);
      PositionUpdate position_update{
          .stream_id = stream_id_,
          .account = security_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.symbol,
          .external_account{},
          .long_quantity = std::max(0.0, item.position_amount),
          .short_quantity = std::max(0.0, -item.position_amount),
          .long_quantity_begin = NaN,
          .short_quantity_begin = NaN,
      };
      create_trace_and_dispatch(handler_, trace_info, position_update, true);
    }
  });
}

void DropCopy::operator()(const server::Trace<json::MarginCall> &event) {
  profile_.margin_call([&]() {
    auto &[trace_info, margin_call] = event;
    log::debug("margin_call={}"sv, margin_call);
  });
}

}  // namespace binance_futures
}  // namespace roq
