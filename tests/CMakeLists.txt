include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(SIMPLE_INCLUDE_DIR "../include")

add_executable(signal_test "signal_test.cpp")
target_include_directories(signal_test PRIVATE ${SIMPLE_INCLUDE_DIR})

find_package(Threads REQUIRED)
add_executable(thread_pool_test "thread_pool_test.cpp")
target_include_directories(thread_pool_test PRIVATE ${SIMPLE_INCLUDE_DIR})
IF( NOT WIN32 )
target_link_libraries(thread_pool_test Threads::Threads "-lstdc++fs")
ENDIF()

add_executable(logger_test "logger_test.cpp")
target_include_directories(logger_test PRIVATE ${SIMPLE_INCLUDE_DIR})
IF( NOT WIN32 )
target_link_libraries(logger_test "-lstdc++fs")
ENDIF()

add_executable(condition_buffer_test "condition_buffer_test.cpp")
target_include_directories(condition_buffer_test PRIVATE ${SIMPLE_INCLUDE_DIR})
IF( NOT WIN32 )
target_link_libraries(condition_buffer_test Threads::Threads "-lstdc++fs")
ENDIF()

add_executable(zipper_test "zipper_test.cpp")
target_include_directories(zipper_test PRIVATE ${SIMPLE_INCLUDE_DIR})
