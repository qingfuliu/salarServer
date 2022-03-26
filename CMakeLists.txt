cmake_minimum_required(VERSION 3.0)

project(sylar)
include (/usr/local/cmake-3.6.2/util.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "-std=c++11")

set(
    LAB_SRC
    sylar/thread.cpp
    sylar/log.cpp
    sylar/util.cpp 
    sylar/config.cpp
    sylar/scheduler.cpp
    sylar/fiber.cpp
    sylar/IOManager.cpp
    sylar/timer.cpp
)

include_directories(
    sylar
    sylar/src
)

add_definitions("-Wall -g")
add_definitions(-Wno-builtin-macro-redefined)

add_library(sylar SHARED ${LAB_SRC})
redefine_file_macro(sylar)

add_executable(tests tests/test.cpp)
add_executable(tests_config tests/test_config.cpp)
add_executable(tests_thread tests/test_thread.cpp)
add_executable(tests_util tests/test_util.cpp)
add_executable(tests_fiber tests/test_Fiber.cpp)
add_executable(tests_IOManager tests/test_IOManager.cpp)
add_executable(tests_Timer tests/test_Timer.cpp)
redefine_file_macro(tests)
redefine_file_macro(tests_config)
redefine_file_macro(tests_thread)
redefine_file_macro(tests_util)
redefine_file_macro(tests_fiber)
redefine_file_macro(tests_IOManager)
redefine_file_macro(tests_Timer)

add_dependencies(tests sylar)
add_dependencies(tests_config sylar)                    
add_dependencies(tests_thread sylar)         
add_dependencies(tests_util sylar)       
add_dependencies(tests_fiber sylar)       
add_dependencies(tests_IOManager sylar)       
add_dependencies(tests_Timer sylar)       

target_link_libraries(tests sylar pthread yaml-cpp)
target_link_libraries(tests_config sylar pthread yaml-cpp)
target_link_libraries(tests_thread sylar pthread yaml-cpp)
target_link_libraries(tests_util sylar pthread yaml-cpp)
target_link_libraries(tests_fiber sylar pthread yaml-cpp)
target_link_libraries(tests_IOManager sylar pthread yaml-cpp)
target_link_libraries(tests_Timer sylar pthread yaml-cpp)

set(EXECUTABLE_OUTPUT_PATH ../bin)

set(LIBRARY_OUTPUT_PATH ../lib)