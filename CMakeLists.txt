project( Simple )
cmake_minimum_required(VERSION 2.8.10)

IF( NOT WIN32 )
add_definitions(-std=c++1z)
ENDIF()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories(vendor)
include_directories(include)

add_subdirectory(tests)
