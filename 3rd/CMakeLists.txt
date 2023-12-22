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
        https://jihulab.com/infracreate-public/sfs-benchmark/-/archive/v1.7.0/sfs-benchmark-v1.7.0.zip
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
