include(RoqAutogen)

set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_account_balance_ack.cpp
    json_account_config_update.cpp
    json_account_status_ack.cpp
    json_account_update.cpp
    json_account_update_papi.cpp
    json_balance_update.cpp
    json_book_ticker.cpp
    json_countdown_cancel_all_ack.cpp
    json_depth_ack.cpp
    json_depth_update.cpp
    json_encoder.cpp
    json_error.cpp
    json_exchange_info_ack.cpp
    json_filters.cpp
    json_grid_update.cpp
    json_liability_change.cpp
    json_listen_key_ack.cpp
    json_margin_call.cpp
    json_mini_ticker.cpp
    json_open_orders_ack.cpp
    json_order_cancel_ack.cpp
    json_order_place_ack.cpp
    json_order_trade_update.cpp
    json_order_trade_update_papi.cpp
    json_outbound_account_position.cpp
    json_session_logon.cpp
    json_strategy_update.cpp
    json_trades_ack.cpp
    json_wsapi_account_balance.cpp
    json_wsapi_account_position.cpp
    json_wsapi_account_status.cpp
    json_wsapi_error.cpp
    json_wsapi_order_cancel.cpp
    json_wsapi_order_modify.cpp
    json_wsapi_order_place.cpp
    json_zzz_position_papi.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-json ${PROJECT_NAME}-flags Catch2::Catch2)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
