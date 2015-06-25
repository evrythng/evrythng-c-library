set(SRC
    ${PROJECT_SOURCE_DIR}/demo/evrythng-cli.c
)

include_directories(${PROJECT_SOURCE_DIR}/evrythng/include)

add_executable(evrythng-cli ${SRC})
target_link_libraries(evrythng-cli evrythng)
