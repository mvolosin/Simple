project( Simple )
cmake_minimum_required(VERSION 2.8.10)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")

add_library(Simple INTERFACE)
target_include_directories(Simple INTERFACE "include/")

# add test target
add_subdirectory(tests)
