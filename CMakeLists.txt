cmake_minimum_required(VERSION 3.28...4.0)
project(coactor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src)
add_subdirectory(tests)
