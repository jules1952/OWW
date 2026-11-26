cmake_minimum_required(VERSION 3.20)
project(pmedian LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pmedian_common STATIC
    src/common/input.cpp
    src/common/solution.cpp
    src/common/nearest_search.cpp
    src/common/solver_cli.cpp)
target_include_directories(pmedian_common PUBLIC src/common)
if(MSVC)
    target_compile_options(pmedian_common PRIVATE /utf-8)
endif()

add_library(pmedian_s0 STATIC src/seq/s0.cpp)
target_link_libraries(pmedian_s0 PUBLIC pmedian_common)

add_executable(pm_s0 src/seq/s0_main.cpp)
target_link_libraries(pm_s0 PRIVATE pmedian_s0)

add_library(pmedian_s1 STATIC src/seq/s1.cpp)
target_link_libraries(pmedian_s1 PUBLIC pmedian_common)

add_executable(pm_s1 src/seq/s1_main.cpp)
target_link_libraries(pm_s1 PRIVATE pmedian_s1)

option(PMEDIAN_PROFILE_SEQ "Measure sequential solver phases" OFF)
if(PMEDIAN_PROFILE_SEQ)
    target_compile_definitions(pmedian_s0 PRIVATE PMEDIAN_PROFILE_SEQ)
    target_compile_definitions(pmedian_s1 PRIVATE PMEDIAN_PROFILE_SEQ)
endif()

function(pmedian_build_info target profile)
    target_compile_definitions(${target} PRIVATE
        PMEDIAN_BUILD_COMPILER="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
        PMEDIAN_BUILD_CONFIG="$<CONFIG>"
        PMEDIAN_BUILD_PROFILE=$<BOOL:${profile}>)
endfunction()

pmedian_build_info(pm_s0 "${PMEDIAN_PROFILE_SEQ}")
pmedian_build_info(pm_s1 "${PMEDIAN_PROFILE_SEQ}")

option(PMEDIAN_ENABLE_OPENMP "Build the P1 OpenMP solver" OFF)
if(PMEDIAN_ENABLE_OPENMP)
    find_package(OpenMP REQUIRED COMPONENTS CXX)
    add_library(pmedian_p1 STATIC src/omp/p1.cpp)
    target_link_libraries(pmedian_p1 PUBLIC pmedian_common PRIVATE OpenMP::OpenMP_CXX)
    add_executable(pm_p1 src/omp/p1_main.cpp)
    target_link_libraries(pm_p1 PRIVATE pmedian_p1)
    pmedian_build_info(pm_p1 OFF)
    target_compile_definitions(pm_p1 PRIVATE PMEDIAN_BUILD_OPENMP="${OpenMP_CXX_VERSION}")
    if(MSVC)
        target_compile_options(pmedian_p1 PRIVATE /utf-8)
    endif()
endif()

option(PMEDIAN_ENABLE_CUDA "Build the G1 CUDA solver" OFF)
option(PMEDIAN_PROFILE_CUDA "Measure detailed G1 solver phases" OFF)
if(PMEDIAN_ENABLE_CUDA)
    enable_language(CUDA)
    set(PMEDIAN_CUDA_BLOCK_SIZE "128" CACHE STRING "Threads per G1 CUDA block")
    set_property(CACHE PMEDIAN_CUDA_BLOCK_SIZE PROPERTY STRINGS 64 128)
    if(NOT PMEDIAN_CUDA_BLOCK_SIZE MATCHES "^(64|128)$")
        message(FATAL_ERROR "PMEDIAN_CUDA_BLOCK_SIZE must be 64 or 128")
    endif()
    add_library(pmedian_g1 STATIC src/cuda/g1.cu)
    set_target_properties(pmedian_g1 PROPERTIES
        CUDA_STANDARD 17 CUDA_STANDARD_REQUIRED ON CUDA_EXTENSIONS OFF)
    target_include_directories(pmedian_g1 PUBLIC src/cuda)
    target_link_libraries(pmedian_g1 PUBLIC pmedian_common)
    target_compile_definitions(pmedian_g1 PRIVATE PMEDIAN_CUDA_BLOCK_SIZE=${PMEDIAN_CUDA_BLOCK_SIZE})
    if(PMEDIAN_PROFILE_CUDA)
        target_compile_definitions(pmedian_g1 PRIVATE PMEDIAN_PROFILE_CUDA)
    endif()
    if(MSVC AND CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
        target_compile_options(pmedian_g1 PRIVATE -Xcompiler=/utf-8)
    endif()
    add_executable(pm_g1 src/cuda/g1_main.cpp)
    target_link_libraries(pm_g1 PRIVATE pmedian_g1)
    pmedian_build_info(pm_g1 "${PMEDIAN_PROFILE_CUDA}")
    string(REPLACE ";" "," pmedian_cuda_architectures "${CMAKE_CUDA_ARCHITECTURES}")
    target_compile_definitions(pm_g1 PRIVATE
        PMEDIAN_BUILD_CUDA="${CMAKE_CUDA_COMPILER_ID} ${CMAKE_CUDA_COMPILER_VERSION}"
        PMEDIAN_BUILD_CUDA_ARCHITECTURES="${pmedian_cuda_architectures}"
        PMEDIAN_BUILD_CUDA_BLOCK_SIZE=${PMEDIAN_CUDA_BLOCK_SIZE})
endif()

if(MSVC)
    foreach(target pmedian_s0 pmedian_s1 pm_s0 pm_s1)
        target_compile_options(${target} PRIVATE /utf-8)
    endforeach()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(pmedian_input_tests tests/input_tests.cpp)
    target_link_libraries(pmedian_input_tests PRIVATE pmedian_common)
    add_test(NAME pmedian_input
        COMMAND pmedian_input_tests "${CMAKE_CURRENT_BINARY_DIR}/input-test-data")

    add_executable(pmedian_seq_tests tests/seq_tests.cpp)
    target_link_libraries(pmedian_seq_tests PRIVATE pmedian_s0 pmedian_s1)
    if(PMEDIAN_ENABLE_OPENMP)
        target_link_libraries(pmedian_seq_tests PRIVATE pmedian_p1)
        target_compile_definitions(pmedian_seq_tests PRIVATE PMEDIAN_TEST_OPENMP)
    endif()
    if(PMEDIAN_ENABLE_CUDA)
        target_link_libraries(pmedian_seq_tests PRIVATE pmedian_g1)
        target_compile_definitions(pmedian_seq_tests PRIVATE PMEDIAN_TEST_CUDA)
    endif()
    if(MSVC)
        target_compile_options(pmedian_seq_tests PRIVATE /utf-8)
    endif()
    add_test(NAME pmedian_seq COMMAND pmedian_seq_tests)
endif()
