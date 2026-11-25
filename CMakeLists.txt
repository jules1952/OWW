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

add_library(pmedian_s0 STATIC src/seq/s0.cpp)
target_link_libraries(pmedian_s0 PUBLIC pmedian_common)

add_executable(pm_s0 src/seq/s0_main.cpp)
target_link_libraries(pm_s0 PRIVATE pmedian_s0)

add_executable(pm_s1 src/seq/s1.cpp src/seq/s1_main.cpp)
target_link_libraries(pm_s1 PRIVATE pmedian_s0)

option(PMEDIAN_PROFILE_SEQ "Measure sequential solver phases" OFF)
if(PMEDIAN_PROFILE_SEQ)
    target_compile_definitions(pmedian_s0 PRIVATE PMEDIAN_PROFILE_SEQ)
    target_compile_definitions(pm_s1 PRIVATE PMEDIAN_PROFILE_SEQ)
endif()

if(MSVC)
    foreach(target pmedian_s0 pm_s0 pm_s1)
        target_compile_options(${target} PRIVATE /utf-8)
    endforeach()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(pmedian_input_tests tests/input_tests.cpp)
    target_link_libraries(pmedian_input_tests PRIVATE pmedian_common)
    add_test(NAME pmedian_input
        COMMAND pmedian_input_tests "${CMAKE_CURRENT_BINARY_DIR}/input-test-data")
endif()
