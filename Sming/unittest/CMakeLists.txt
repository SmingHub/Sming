cmake_minimum_required(VERSION 2.8.4)
project(gtest)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_bin test.cpp)
target_link_libraries(test_bin ${GTEST_BOTH_LIBRARIES})
target_link_libraries(test_bin ${CMAKE_THREAD_LIBS_INIT})

enable_testing()
add_test(A test_bin)
