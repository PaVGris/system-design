cmake_minimum_required(VERSION 3.22)
project(api-gateway)

find_package(userver REQUIRED COMPONENTS core)

add_executable(api-gateway
    src/main.cpp
    src/handlers/gateway.cpp
)

target_link_libraries(api-gateway userver::core)

target_include_directories(api-gateway PRIVATE src)

userver_testsuite_add(
    SERVICE_TARGET api-gateway
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    PYTEST_ARGS
        --service-config=${CMAKE_CURRENT_SOURCE_DIR}/configs/config.yaml
        --service-binary=${CMAKE_CURRENT_BINARY_DIR}/api-gateway
)