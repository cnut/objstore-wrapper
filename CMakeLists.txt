cmake_minimum_required(VERSION 3.13)

project(s3wrapper VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# generate compile commands, easy for vscode with clangd language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Benchmark
option(WITH_BENCHMARK "Build with benchmarks" ON)

# CMake Macro
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
INCLUDE(aws-sdk-cpp)

CHECK_PREPARE_OBJSTORE()

add_subdirectory(3rd)
add_subdirectory(src)
