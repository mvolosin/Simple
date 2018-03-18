project( Simple )
cmake_minimum_required(VERSION 2.8.10)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")

find_package(Boost 1.66 COMPONENTS system REQUIRED)
add_library(Simple INTERFACE)
target_include_directories(Simple INTERFACE "include/")
target_link_libraries(Simple INTERFACE ${Boost_LIBRARIES})

# add test target
add_subdirectory(tests)
