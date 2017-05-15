project( Simple )
cmake_minimum_required(VERSION 2.8.10)

add_definitions(-std=c++1z)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories(vendor)
include_directories(include)

# add a target to generate API documentation with Doxygen
#find_package(Doxygen)
#if(DOXYGEN_FOUND)
    #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    #add_custom_target(doc
        #${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        #COMMENT "Generating API documentation with Doxygen" VERBATIM
        #)
#endif(DOXYGEN_FOUND)


# find_package(Boost COMPONENTS system REQUIRED)
# include_directories(${Boost_INCLUDE_DIR})
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)

add_subdirectory(tests)
