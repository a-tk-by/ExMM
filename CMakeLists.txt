cmake_minimum_required(VERSION 3.0)

project(ExMM)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DEBUG_INFO ON)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/CMakeOut/bin/ CACHE PATH "Path for output binary") 

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.14/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)


conan_cmake_run(REQUIRES gtest/1.8.1@bincrafters/stable
        BASIC_SETUP
        BUILD missing)


include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()




file(GLOB_RECURSE SOURCE ./src/*.cpp)
file(GLOB_RECURSE HEADERS ./src/*.hpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCE})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)



file(GLOB_RECURSE EXAMPLES ./examples/*.cpp)
message(STATUS  ${CONAN_LIBS_GTEST})
add_executable("${PROJECT_NAME}-Demo" ${EXAMPLES})
set_property(TARGET "${PROJECT_NAME}-Demo"  PROPERTY CXX_STANDARD 17)

target_link_libraries("${PROJECT_NAME}-Demo" ${PROJECT_NAME} ${CONAN_LIBS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("${PROJECT_NAME}-Demo" Threads::Threads ${CONAN_LIBS})


add_custom_target(test COMMAND bin/${PROJECT_NAME}-Demo)