link_directories(${CMAKE_BINARY_DIR}/3rd/lib)

set(INCLUDE_DIRS
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/include"
)

add_library(s3file STATIC "")
add_dependencies(s3file objstore-ext-proj benchmark-lib)
target_sources(s3file
  PRIVATE
    "lib/local.cc"
    "lib/obj_store.cc"
    "lib/s3.cc"

  # Only CMake 3.3+ supports PUBLIC sources in targets exported by "install".
    $<$<VERSION_GREATER:CMAKE_VERSION,3.2>:PUBLIC>
    "include/local.h"
    "include/obj_store.h"
    "include/s3.h"
)

target_include_directories(s3file SYSTEM PRIVATE "${INCLUDE_DIRS}")

target_link_libraries(s3file
    PUBLIC
    ${OBJSTORE_LIBRARIES}
    ${OBJSTORE_PLATFORM_DEPS})

target_link_directories(s3file
    SYSTEM PRIVATE
    ${CURL_LIB}
    ${INCLUDE_DIRS})


if(WITH_BENCHMARK)
  set(BENCHMARK_FILE
    bench/put_get.cc)

  foreach(sourcefile ${BENCHMARK_FILE})
    get_filename_component(filename ${sourcefile} NAME_WE)
    set(exename run_${filename})
    add_executable(${exename} ${sourcefile})
    add_dependencies(${exename} s3file benchmark-lib)
    target_link_libraries(${exename} PRIVATE s3file benchmark benchmark_main)
    target_include_directories(${exename} SYSTEM PRIVATE
      "${CMAKE_BINARY_DIR}/3rd/include"
      ${CURL_LIB}
      ${INCLUDE_DIRS})
  endforeach(sourcefile ${BENCHMARKS})

endif(WITH_BENCHMARK)
