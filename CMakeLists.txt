cmake_minimum_required(VERSION 3.20)
project(Lumina-HFT
  VERSION 1.0.0
  LANGUAGES CXX C
  DESCRIPTION "Multi-Threaded Liquidity Provision Framework"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build Google Benchmark" ON)
option(BUILD_PYBIND11 "Build Python bindings" ON)
option(USE_AVX512 "Enable AVX-512 SIMD" ON)

# Compiler flags for low latency
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
  add_compile_options($<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>)
  if(USE_AVX512)
    add_compile_options($<$<CONFIG:Release>:-mavx512f -mavx512dq>)
  endif()
elseif(MSVC)
  add_compile_options(/W4 /permissive-)
  add_compile_options($<$<CONFIG:Release>:/O2 /DNDEBUG>)
endif()

# Dependencies: FetchContent for GTest, Benchmark, fmt
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.0
)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

FetchContent_MakeAvailable(fmt)

# Core library (no Boost dependency for minimal build; optional Asio later)
add_library(lumina_core STATIC
  src/memory_pool.cpp
  src/ring_buffer.cpp
  src/order_book.cpp
  src/thread_utils.cpp
  src/market_data_handler.cpp
  src/strategy_engine.cpp
  src/avellaneda_stoikov.cpp
  src/order_book_imbalance.cpp
  src/risk_checks.cpp
  src/fix_engine.cpp
  src/simd_indicators.cpp
  src/kdb_mock.cpp
)
target_include_directories(lumina_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lumina_core PUBLIC fmt::fmt)
if(USE_AVX512 AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
  target_compile_options(lumina_core PRIVATE -mavx512f -mavx512dq)
endif()

# Main executable (simulator / runner)
add_executable(lumina_hft_main src/main.cpp)
target_link_libraries(lumina_hft_main PRIVATE lumina_core)

# Unit tests
if(BUILD_TESTS)
  add_executable(lumina_tests
    tests/test_memory_pool.cpp
    tests/test_ring_buffer.cpp
    tests/test_order_book.cpp
    tests/test_avellaneda_stoikov.cpp
    tests/test_risk_checks.cpp
    tests/test_fix_engine.cpp
  )
  target_link_libraries(lumina_tests PRIVATE lumina_core GTest::gtest GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(lumina_tests)
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
  add_executable(lumina_bench
    benchmarks/bench_order_book.cpp
    benchmarks/bench_ring_buffer.cpp
    benchmarks/bench_simd.cpp
  )
  target_link_libraries(lumina_bench PRIVATE lumina_core benchmark::benchmark)
endif()

# Python bindings (optional)
if(BUILD_PYBIND11)
  find_package(pybind11 CONFIG QUIET)
  if(pybind11_FOUND)
    pybind11_add_module(lumina_py bindings/python_bindings.cpp)
    target_link_libraries(lumina_py PRIVATE lumina_core)
  endif()
endif()

# Install
install(TARGETS lumina_core lumina_hft_main
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)
