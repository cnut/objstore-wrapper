include(ExternalProject)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP on CMake 3.24+:
# CMake 3.24 and above prefers to set the timestamps of all extracted contents to the time of the extraction.
# This ensures that anything that depends on the extracted contents will be rebuilt whenever the URL changes.
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.24)
  cmake_policy(SET CMP0135 NEW)
endif()

set(3RD_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/3rd)

if(WITH_BENCHMARK)
  find_library(BENCHMARK_LIBRARY benchmark HINTS ${3RD_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)

  if (NOT BENCHMARK_LIBRARY)
    ExternalProject_Add(
      benchmark-lib
      SOURCE_DIR "${PROJECT_SOURCE_DIR}/3rd/benchmark"
      URL
        https://github.com/google/benchmark/archive/refs/tags/v1.7.0.zip
      TIMEOUT 120
      CMAKE_ARGS
        -DCMAKE_CXX_STANDARD=17
        -DCMAKE_INSTALL_PREFIX=${3RD_INSTALL_PREFIX}
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_BUILD_TYPE=Release
        -DBENCHMARK_ENABLE_TESTING=OFF
      BUILD_COMMAND cmake --build .
    )
  else()
    add_custom_target(benchmark-lib COMMAND "")
  endif()
endif()

if(WITH_TESTS)
  find_library(GTEST_LIBRARY gtest HINTS ${3RD_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)

  if (NOT GTEST_LIBRARY)
    ExternalProject_Add(
      gtest-lib
      SOURCE_DIR "${PROJECT_SOURCE_DIR}/3rd/googletest"
      URL
        https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
      TIMEOUT 120
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_GMOCK=OFF
        -DCMAKE_INSTALL_PREFIX=${3RD_INSTALL_PREFIX}
        -DCMAKE_INSTALL_LIBDIR=lib
      BUILD_COMMAND cmake --build .
    )
    MESSAGE(STATUS "GTest library not found")
  else()
    MESSAGE(STATUS "GTest library found at ${GTEST_LIBRARY}")
    add_custom_target(gtest-lib COMMAND "")
  endif()
endif()

if (NOT GFLAGS_LIBRARY)
  ExternalProject_Add(
    gflags-lib
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/3rd/gflags"
    URL
      https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.zip
    TIMEOUT 120
    CMAKE_ARGS
      -DCMAKE_CXX_STANDARD=17
      -DCMAKE_INSTALL_PREFIX=${3RD_INSTALL_PREFIX}
      -DCMAKE_INSTALL_LIBDIR=lib
    BUILD_COMMAND cmake --build .
  )
else()
  add_custom_target(gflags-lib COMMAND "")
endif()
