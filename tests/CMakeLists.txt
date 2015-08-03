set(SRC
    ${PROJECT_SOURCE_DIR}/tests/CuTest.c 
    ${PROJECT_SOURCE_DIR}/tests/tests.c
    ${PROJECT_SOURCE_DIR}/tests/main.c
    ${PROJECT_SOURCE_DIR}/platforms/POSIX/posix.c
)

add_definitions(-DPOSIX_PLATFORM)

include_directories(${PROJECT_SOURCE_DIR}/evrythng/include)
include_directories(${PROJECT_SOURCE_DIR}/platforms)
include_directories(${PROJECT_SOURCE_DIR}/tests)

add_executable(test ${SRC})
target_link_libraries(test evrythng)
