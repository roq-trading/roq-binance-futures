/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/binance_futures/drop_copy.h"

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/binance_futures/flags.h"

#include "roq/binance_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace binance_futures {

namespace {
static const auto NAME = "ex"_sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,  // XXX HANS ???
    SupportType::FUNDS,
};

static auto create_uri(const std::string_view &listen_key) {
  assert(!listen_key.empty());
  // XXX HANS make it easier to append to path
  auto &uri = Flags::ws_uri();
  auto result = fmt::format("{}://{}{}/{}"_sv, uri.scheme, uri.host, uri.path, listen_key);
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
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
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
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"_sv),
          .order_trade_update = create_metrics(name_, "order_trade_update"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
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
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(const core::web::Socket::Connected &) {
}

void DropCopy::operator()(const core::web::Socket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void DropCopy::operator()(const core::web::Socket::Ready &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopy::operator()(const core::web::Socket::Close &) {
}

void DropCopy::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
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
      log::debug(R"(message="{}")"_sv, message);
      server::TraceInfo trace_info;
      core::json::Buffer buffer(decode_buffer_);
      json::UserStreamParser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"_sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(const server::Trace<json::OrderTradeUpdate> &event) {
  profile_.order_trade_update([&]() {
    auto &[trace_info, order_trade_update] = event;
    log::debug("order_trade_update={}"_sv, order_trade_update);
    log::info<3>("order_trade_update={}"_sv, order_trade_update);
    auto &execution_report = order_trade_update.execution_report;
    auto side = json::map(execution_report.side);
    auto external_order_id = fmt::format("{}"_sv, execution_report.order_id);  // XXX HANS
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
        .update_time_utc = order_trade_update.event_time,
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
            []([[maybe_unused]] auto &order) {
              // XXX IMPLEMENT
            })) {
    } else {
      log::warn("*** EXTERNAL ORDER ***"_sv);
      log::warn("execution_report={}"_sv, execution_report);
    }
  });
}

}  // namespace binance_futures
}  // namespace roq
