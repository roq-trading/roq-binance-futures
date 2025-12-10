set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES
    encoder.cpp
    map.cpp
    market_stream_parser.cpp
    user_stream_parser.cpp
    utils.cpp
    wsapi_parser.cpp
    wsapi_request.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_balance_ack_item.json
    account_balance_ack.json
    account_config_update_data1.json
    account_config_update_data2.json
    account_config_update.json
    account_status_ack.json
    account_update_data_balance.json
    account_update_data.json
    account_update_data_position.json
    account_update.json
    agg_trade.json
    asset.json
    balances_item.json
    balance_update.json
    book_ticker.json
    contract_status.json
    contract_type.json
    countdown_cancel_all_ack.json
    depth_ack_bids_asks_item.json
    depth_ack.json
    depth_update_bids_asks_item.json
    depth_update.json
    error.json
    event_reason.json
    event_type.json
    exchange_info_ack_assets_item.json
    exchange_info_ack.json
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
    kline_ack_data_item.json
    kline_ack.json
    kline_data.json
    kline.json
    liability_change.json
    listen_key_ack.json
    margin_call_item.json
    margin_call.json
    margin_type.json
    mark_price_update.json
    mini_ticker.json
    open_orders_ack.json
    open_orders_cancel_all_ack.json
    order_cancel_ack.json
    order.json
    order_modify_ack.json
    order_place_ack.json
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
    trades_ack.json
    working_type.json
    wsapi_account_balance.json
    wsapi_account_position.json
    wsapi_account_position_result_item.json
    wsapi_account_status.json
    wsapi_error.json
    wsapi_listen_key.json
    wsapi_open_orders_cancel_all.json
    wsapi_open_orders.json
    wsapi_order_cancel.json
    wsapi_order_modify.json
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
