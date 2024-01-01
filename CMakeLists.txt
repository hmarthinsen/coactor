cmake_minimum_required(VERSION 3.22...3.27)
project(coactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(asio REQUIRED)
find_package(concurrencpp REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(src)
