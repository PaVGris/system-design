cmake_minimum_required(VERSION 3.22)
project(user-service)

find_package(userver REQUIRED COMPONENTS core chaotic postgresql)

add_executable(user-service
    src/main.cpp
    src/handlers/auth.cpp
    src/pg/users_pg.cpp
)

file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.yaml)

userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    GENERATE_SERIALIZERS
    LAYOUT "/components/schemas/([^/]*)/=user_service::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(user-service
    userver::core
    userver::chaotic
    userver::postgresql
    ${PROJECT_NAME}-chgen
)

target_include_directories(user-service PRIVATE
    src
    src/pg
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

userver_testsuite_add(
    SERVICE_TARGET user-service
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    PYTEST_ARGS
        --service-config=${CMAKE_CURRENT_SOURCE_DIR}/configs/config.yaml
        --service-binary=${CMAKE_CURRENT_BINARY_DIR}/user-service
)