cmake_minimum_required(VERSION 3.0)

project(ExMM-Demo)

option(TARGET_CPU "arch" "x86_64")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DEBUG_INFO ON)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/CMakeOut/bin/ CACHE PATH "Path for output binary")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#set(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")

if(TARGET_CPU STREQUAL "x86")
    set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
endif()


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_subdirectory(googletest)

add_subdirectory(src)

file(GLOB_RECURSE EXAMPLES ./examples/*.cpp)
add_executable("${PROJECT_NAME}" ${EXAMPLES})
set_property(TARGET "${PROJECT_NAME}"  PROPERTY CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}" ExMM gtest gtest_main)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("${PROJECT_NAME}" Threads::Threads)


IF(MSVC)
  add_custom_target(examples COMMAND bin/${CMAKE_BUILD_TYPE}/${PROJECT_NAME})
else()
  add_custom_target(examples COMMAND bin/${PROJECT_NAME})
endif()


set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(google-benchmark)


file(GLOB_RECURSE BENCHMARKS_SRC ./benchmarks/*.cpp)
add_executable(ExMM-benchmark ${BENCHMARKS_SRC})
set_property(TARGET ExMM-benchmark   PROPERTY CXX_STANDARD 17)
target_link_libraries(ExMM-benchmark  ExMM benchmark benchmark_main)


IF(MSVC)
    add_custom_target(benchmarks COMMAND bin/${CMAKE_BUILD_TYPE}/ExMM-benchmark)
else()
    add_custom_target(benchmarks COMMAND bin/ExMM-benchmark)
endif()



#include(CodeCoverage.cmake)

#SETUP_TARGET_FOR_COVERAGE(
#        coverage_${PROJECT_NAME}-Demo  # Name for custom target.
#        "${PROJECT_NAME}-Demo"       # Name of the test driver executable that runs the tests.
#        # NOTE! This should always have a ZERO as exit code
#        # otherwise the coverage generation will not complete.
#        coverage_${PROJECT_NAME}-Demo_dir            # Name of output directory.
#)
