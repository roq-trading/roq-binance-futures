set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES market_stream_parser.cpp user_stream_parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account.json
    agg_trade.json
    asset.json
    auto_cancel_all_open_orders.json
    balance.json
    balance_item.json
    balance_update.json
    balances_item.json
    book_ticker.json
    cancel_all_open_orders.json
    cancel_order.json
    contract_type.json
    depth.json
    depth_item.json
    depth_update.json
    error.json
    event_type.json
    exchange_info.json
    exchange_info_asset.json
    execution_report.json
    execution_type.json
    field.json
    fill.json
    filter.json
    filter_type.json
    filters.json
    interval.json
    listen_key.json
    mark_price_update.json
    mini_ticker.json
    new_order.json
    open_orders.json
    order.json
    order_status.json
    order_type.json
    outbound_account_info.json
    outbound_account_position.json
    position.json
    position_side.json
    rate_limit.json
    rate_limit_type.json
    result.json
    side.json
    stream.json
    symbol.json
    symbol_status.json
    ticker.json
    time_in_force.json
    working_type.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/binance_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "h"
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

set(GITIGNORE "${CMAKE_CURRENT_SOURCE_DIR}/.gitignore")

file(WRITE "${GITIGNORE}" "")

foreach(path ${AUTOGEN_SCHEMAS})
  get_filename_component(name ${path} NAME_WE)
  file(APPEND "${GITIGNORE}" "${name}.h\n${name}.cpp\n")
endforeach()

add_library("${TARGET_NAME}" STATIC ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
