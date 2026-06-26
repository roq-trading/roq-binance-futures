set(TARGET_NAME ${PROJECT_NAME}-fix_bridge-tools)

set(SOURCES crypto.cpp fix_log.cpp mapping.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
