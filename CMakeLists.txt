cmake_minimum_required(VERSION 3.27)
project(coactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(asio REQUIRED)
find_package(concurrencpp REQUIRED)
find_package(fmt REQUIRED)

add_executable(reactive-co src/reactive-co.cpp)
target_link_libraries(reactive-co asio::asio fmt::fmt)

add_executable(actor src/actor.cpp src/stage.cpp)
target_link_libraries(actor concurrencpp::concurrencpp fmt::fmt)

add_executable(ct src/conctest.cpp)
target_link_libraries(ct concurrencpp::concurrencpp)
