set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES market_stream_parser.cpp user_stream_parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_config_update_data1.json
    account_config_update_data2.json
    account_config_update.json
    account.json
    account_update_data_balance.json
    account_update_data.json
    account_update_data_position.json
    account_update.json
    agg_trade.json
    asset.json
    auto_cancel_all_open_orders.json
    balance_item.json
    balance.json
    balances_item.json
    book_ticker.json
    cancel_all_open_orders.json
    cancel_order.json
    contract_type.json
    depth_item.json
    depth.json
    depth_update.json
    error.json
    event_reason.json
    event_type.json
    exchange_info_asset.json
    exchange_info.json
    execution_report.json
    execution_type.json
    field.json
    fill.json
    filter.json
    filters.json
    filter_type.json
    grid_update_data.json
    grid_update.json
    interval.json
    listen_key.json
    margin_call_item.json
    margin_call.json
    mark_price_update.json
    mini_ticker.json
    new_order.json
    open_orders.json
    order.json
    order_status.json
    order_trade_update.json
    order_type.json
    position.json
    position_side.json
    rate_limit.json
    rate_limit_type.json
    result.json
    side.json
    strategy_update_data.json
    strategy_update.json
    stream.json
    symbol.json
    symbol_status.json
    ticker.json
    time_in_force.json
    trade.json
    trades.json
    working_type.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/binance_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/binance_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
