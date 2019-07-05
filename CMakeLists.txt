cmake_minimum_required(VERSION 3.0)

project(ExMM)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DEBUG_INFO ON) 
set (CMAKE_CXX_STANDARD 11)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/CMakeOut/bin/ CACHE PATH "Path for output binary") 

file(GLOB_RECURSE SOURCE ./src/*.cpp)
file(GLOB_RECURSE HEADERS ./src/*.hpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCE})

file(GLOB_RECURSE UTILITIES_SOURCE ./utilities/*.cpp)
file(GLOB_RECURSE UTILITIES_HEADERS ./utilities/*.hpp)

add_library("${PROJECT_NAME}-Utilities" STATIC ${UTILITIES_HEADERS} ${UTILITIES_SOURCE})

file(GLOB_RECURSE EXAMPLES ./examples/*.cpp)

add_executable("${PROJECT_NAME}-Demo" ${EXAMPLES})
target_link_libraries("${PROJECT_NAME}-Demo" ${PROJECT_NAME} "${PROJECT_NAME}-Utilities")

