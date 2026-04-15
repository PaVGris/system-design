cmake_minimum_required(VERSION 3.22)
project(item-service)

find_package(userver REQUIRED COMPONENTS core chaotic postgresql)

add_executable(item-service
    src/main.cpp
    src/handlers/items.cpp
    src/storage/items_storage.cpp
)

file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.yaml)

userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    GENERATE_SERIALIZERS
    LAYOUT "/components/schemas/([^/]*)/=item_service::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(item-service
    userver::core
    userver::chaotic
    userver::postgresql
    ${PROJECT_NAME}-chgen
)

target_include_directories(item-service PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

userver_testsuite_add(
    SERVICE_TARGET item-service
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    PYTEST_ARGS
        --service-config=${CMAKE_CURRENT_SOURCE_DIR}/configs/config.yaml
        --service-binary=${CMAKE_CURRENT_BINARY_DIR}/item-service
)