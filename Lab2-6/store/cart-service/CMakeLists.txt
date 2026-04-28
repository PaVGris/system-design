cmake_minimum_required(VERSION 3.22)
project(cart-service)

find_package(userver REQUIRED COMPONENTS core chaotic mongo)

add_executable(cart-service
    src/main.cpp
    src/handlers/cart.cpp
    src/storage/cart_storage.cpp
)

file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.yaml)

userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    GENERATE_SERIALIZERS
    LAYOUT "/components/schemas/([^/]*)/=cart_service::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cart-service
    userver::core
    userver::chaotic
    userver::mongo
    ${PROJECT_NAME}-chgen
)

target_include_directories(cart-service PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

userver_testsuite_add(
    SERVICE_TARGET cart-service
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    PYTEST_ARGS
        --service-config=${CMAKE_CURRENT_SOURCE_DIR}/configs/config.yaml
        --service-binary=${CMAKE_CURRENT_BINARY_DIR}/cart-service
)