include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_executable(signal_test "signal_test.cpp")


find_package(Threads REQUIRED)
add_executable(thread_pool_test "thread_pool_test.cpp")
target_link_libraries(thread_pool_test Threads::Threads "-lstdc++fs")
