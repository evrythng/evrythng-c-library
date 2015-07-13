set(SRC
    ${PROJECT_SOURCE_DIR}/tests/CuTest.c 
    ${PROJECT_SOURCE_DIR}/tests/tests.c
    ${PROJECT_SOURCE_DIR}/tests/main.c
)

include_directories(${PROJECT_SOURCE_DIR}/evrythng/include)
include_directories(${PROJECT_SOURCE_DIR}/tests)

add_executable(test ${SRC})
target_link_libraries(test evrythng)
