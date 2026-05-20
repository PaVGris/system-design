cmake_minimum_required(VERSION 3.22)
project(kafka-service)

find_package(userver REQUIRED COMPONENTS core kafka)

add_executable(kafka-service
    src/main.cpp
    src/handlers/consumer_handler.cpp
)

target_link_libraries(kafka-service
    userver::core
    userver::kafka
)

target_include_directories(kafka-service PRIVATE
    src
)