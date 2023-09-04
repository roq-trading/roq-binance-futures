set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_account_config_update.cpp
    json_account.cpp
    json_account_update.cpp
    json_auto_cancel_all_open_orders.cpp
    json_balance.cpp
    json_book_ticker.cpp
    json_cancel_order.cpp
    json_depth.cpp
    json_depth_update.cpp
    json_error.cpp
    json_exchange_info.cpp
    json_filters.cpp
    json_grid_update.cpp
    json_listen_key.cpp
    json_margin_call.cpp
    json_mini_ticker.cpp
    json_new_order.cpp
    json_open_orders.cpp
    json_order_trade_update.cpp
    json_strategy_update.cpp
    json_trades.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  ${PROJECT_NAME}-json
  ${PROJECT_NAME}-flags
  roq-server::roq-server
  roq-core::roq-core
  roq-io::roq-io
  roq-logging::roq-logging
  roq-flags::roq-flags
  aklomp::base64
  Catch2::Catch2
  ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
