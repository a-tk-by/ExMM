cmake_minimum_required(VERSION 3.0)

project(ExMM)

option(TARGET_CPU "arch" "x86_64")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DEBUG_INFO ON)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/CMakeOut/bin/ CACHE PATH "Path for output binary")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#set(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")

if(TARGET_CPU STREQUAL "x86")
    set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
endif()

add_subdirectory(googletest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


file(GLOB_RECURSE SOURCE ./src/*.cpp)
file(GLOB_RECURSE HEADERS ./src/*.hpp)
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCE})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)


file(GLOB_RECURSE EXAMPLES ./examples/*.cpp)
add_executable("${PROJECT_NAME}-Demo" ${EXAMPLES})
set_property(TARGET "${PROJECT_NAME}-Demo"  PROPERTY CXX_STANDARD 17)
target_link_libraries("${PROJECT_NAME}-Demo" ${PROJECT_NAME} gtest gtest_main)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("${PROJECT_NAME}-Demo" Threads::Threads gtest gtest_main)


IF(MSVC)
  add_custom_target(examples COMMAND ${CMAKE_BUILD_TYPE}/${PROJECT_NAME}-Demo)
else()
  add_custom_target(examples COMMAND bin/${PROJECT_NAME}-Demo)
endif()
#include(CodeCoverage.cmake)
#SETUP_TARGET_FOR_COVERAGE(
#        coverage_${PROJECT_NAME}-Demo  # Name for custom target.
#        "${PROJECT_NAME}-Demo"       # Name of the test driver executable that runs the tests.
#        # NOTE! This should always have a ZERO as exit code
#        # otherwise the coverage generation will not complete.
#        coverage_${PROJECT_NAME}-Demo_dir            # Name of output directory.
#)
