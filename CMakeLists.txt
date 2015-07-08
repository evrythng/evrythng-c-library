cmake_minimum_required(VERSION 2.8)
project(evrythng-c-library)

set (MAJOR_VERSION 1)
set (MINOR_VERSION 0)

set(COMMON_PREFIX CMakeTimestamps)

add_definitions(-DOPENSSL)
add_definitions(-DNO_PERSISTENCE)
add_definitions(-DNOSTACKTRACE)
add_definitions(-DCONFIG_IPV6)

include_directories(${PROJECT_SOURCE_DIR}/evrythng/include)
include_directories(${PROJECT_SOURCE_DIR}/mqtt/src)
set(SRC ${PROJECT_SOURCE_DIR}/evrythng/src/evrythng.c)
include(cmake/mqtt.cmake)
include(cmake/toolchainfile.cmake)

add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
add_subdirectory(${PROJECT_SOURCE_DIR}/demo)

add_library(evrythng SHARED ${SRC})
set_target_properties(evrythng PROPERTIES VERSION ${MINOR_VERSION} SOVERSION ${MAJOR_VERSION})

target_link_libraries(evrythng pthread)
target_link_libraries(evrythng ssl)
