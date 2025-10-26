set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES
    encoder.cpp
    map.cpp
    market_stream_parser.cpp
    user_stream_parser.cpp
    utils.cpp
    wsapi_parser_2.cpp
    wsapi_request.cpp)

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
    balance_update.json
    book_ticker.json
    cancel_all_open_orders.json
    cancel_order.json
    contract_status.json
    contract_type.json
    depth_item.json
    depth.json
    depth_update.json
    error.json
    event_reason.json
    event_type.json
    exchange_info_asset.json
    exchange_info.json
    execution_report_2.json
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
    kline_ack_item.json
    kline_ack.json
    kline_data.json
    kline.json
    liability_change.json
    listen_key.json
    margin_call_item.json
    margin_call.json
    margin_type.json
    mark_price_update.json
    mini_ticker.json
    modify_order.json
    new_order.json
    open_orders.json
    order.json
    order_status.json
    order_trade_update.json
    order_type.json
    outbound_account_position_balances_item.json
    outbound_account_position.json
    position.json
    position_list.json
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
    trade_lite.json
    trades.json
    working_type.json
    wsapi_account_balance.json
    wsapi_account_position.json
    wsapi_account_status.json
    wsapi_cancel_open_orders.json
    wsapi_cancel_order.json
    wsapi_listen_key.json
    wsapi_modify_order.json
    wsapi_open_orders.json
    wsapi_order_place.json
    wsapi_rate_limits_item.json
    wsapi_session_logon.json
    wsapi_session_logon_result.json
    wsapi_trades.json
    wsapi_type.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} autogen-json-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-logging::roq-logging fmt::fmt)
