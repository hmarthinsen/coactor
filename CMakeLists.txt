cmake_minimum_required(VERSION 3.22...3.27)
project(coactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(asio REQUIRED)
find_package(concurrencpp REQUIRED)
find_package(fmt REQUIRED)

add_library(coactor src/actor.cpp src/stage.cpp)
target_link_libraries(coactor PUBLIC concurrencpp::concurrencpp)

add_executable(actortest src/actortest.cpp)
target_link_libraries(actortest PRIVATE coactor fmt::fmt)

add_executable(reactive-co src/reactive-co.cpp)
target_link_libraries(reactive-co PRIVATE asio::asio fmt::fmt)

add_executable(ct src/conctest.cpp)
target_link_libraries(ct PRIVATE concurrencpp::concurrencpp)
