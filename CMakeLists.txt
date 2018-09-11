project( Simple )
cmake_minimum_required(VERSION 2.8.10)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")

add_library(Simple INTERFACE)
target_include_directories(Simple INTERFACE "include/")

find_package(Boost 1.66.0 COMPONENTS system filesystem REQUIRED)
if(Boost_FOUND)
find_package(Threads REQUIRED)

add_library(SimpleHttpServer SHARED "src/Server.cpp")
target_link_libraries(SimpleHttpServer ${Boost_LIBRARIES} Threads::Threads Simple)

add_executable(WebServerTest "tests/http_server_test.cpp")
target_link_libraries(WebServerTest SimpleHttpServer)
endif()

# add test target
add_subdirectory(tests)
