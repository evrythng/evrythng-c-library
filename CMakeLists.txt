cmake_minimum_required(VERSION 2.8)
project(evrythng-c-library)

set (MAJOR_VERSION 1)
set (MINOR_VERSION 0)

set(COMMON_PREFIX CMakeTimestamps)

add_definitions(-DPOSIX_PLATFORM)

include_directories(${PROJECT_SOURCE_DIR}/evrythng/include)
include_directories(${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTClient-C/src)
include_directories(${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTClient-C/src/linux)
include_directories(${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src)
include_directories(${PROJECT_SOURCE_DIR}/platforms)

set(SRC ${PROJECT_SOURCE_DIR}/evrythng/src/evrythng.c)
include(cmake/mqtt.cmake)
include(cmake/toolchainfile.cmake)

add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
add_subdirectory(${PROJECT_SOURCE_DIR}/demo)

add_library(evrythng SHARED ${SRC})
set_target_properties(evrythng PROPERTIES VERSION ${MINOR_VERSION} SOVERSION ${MAJOR_VERSION})

target_link_libraries(evrythng pthread)
target_link_libraries(evrythng ssl)
