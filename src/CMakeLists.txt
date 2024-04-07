#  std::filesystem is a part of the C++17 standard.
#  For GNU:
#    GNU supports std::filesystem (removing it from experimental) starting from 8.0,
#  but requires additional link -lstdc++fs when using it. Starting from 9.0, using
#  the std::filesystem no longer requires additional link -lstdc++fs, which has become a part of libstdc++.
#  Details for: https://gcc.gnu.org/gcc-8/changes.html and https://gcc.gnu.org/gcc-9/changes.html
#
#  For LLVM:
#    LLVM supports std::filesystem starting from 7.0 and requires additional link -lc++fs when using it.
#  Starting from 9.0, using the std::filesystem no longer requires additional link -lc++fs.
#  Details for: https://releases.llvm.org/7.0.0/projects/libcxx/docs/UsingLibcxx.html#using-filesystem-and-libc-fs
#  and https://releases.llvm.org/9.0.0/projects/libcxx/docs/UsingLibcxx.html#using-filesystem
#
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if(CMAKE_CXX_COMPILER_VERSION LESS 8)
    message(FATAL_ERROR "Compiler version too low. Require GNU version >= 8")
  elseif(CMAKE_CXX_COMPILER_VERSION LESS 9)
    set(NEED_LINK_FS ON)
    set(FS_LIB_NAME stdc++fs)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION LESS 7)
    message(FATAL_ERROR "Compiler version too low. Require Clang version >= 7")
  elseif(CMAKE_CXX_COMPILER_VERSION LESS 9)
    set(NEED_LINK_FS ON)
    set(FS_LIB_NAME c++fs)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  if(CMAKE_CXX_COMPILE_VERSION LESS 11)
    message(FATAL_ERROR "Compiler version too low. Require Apple Clang version >= 11")
  endif()
endif()

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
    "lib/local.h"
    "lib/objstore.cc"
    "lib/s3.cc"
    "lib/s3.h"

    # Only CMake 3.3+ supports PUBLIC sources in targets exported by "install".
    $<$<VERSION_GREATER:CMAKE_VERSION,3.2>:PUBLIC>
    "include/objstore.h"
    "include/s3.h"
)

target_include_directories(s3file SYSTEM PRIVATE "${INCLUDE_DIRS}")

target_link_libraries(s3file
    PUBLIC
    ${OBJSTORE_LIBRARIES}
    ${OBJSTORE_PLATFORM_DEPS})

if(NEED_LINK_FS)
  target_link_libraries(s3file PRIVATE ${FS_LIB_NAME})
endif()

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
      ${INCLUDE_DIRS})
  endforeach(sourcefile ${BENCHMARKS})

endif(WITH_BENCHMARK)
