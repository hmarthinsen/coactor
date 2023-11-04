cmake_minimum_required(VERSION 3.27)
project(coactor LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(asio REQUIRED)
find_package(fmt REQUIRED)

add_executable(reactive-co src/reactive-co.cpp)
target_link_libraries(reactive-co asio::asio fmt::fmt)

add_executable(actor src/actor.cpp)
target_link_libraries(actor asio::asio fmt::fmt)
