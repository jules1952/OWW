cmake_minimum_required(VERSION 3.20)
project(pmedian LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pmedian_common STATIC src/common/input.cpp)
target_include_directories(pmedian_common PUBLIC src/common)
if(MSVC)
    target_compile_options(pmedian_common PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(pmedian_input_tests tests/input_tests.cpp)
    target_link_libraries(pmedian_input_tests PRIVATE pmedian_common)
    add_test(NAME pmedian_input
        COMMAND pmedian_input_tests "${CMAKE_CURRENT_BINARY_DIR}/input-test-data")
endif()
