#
# Copyright (c) 2023 ApeCloud, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(SERVICE_COMPONENTS iam s3 sts)
# Find the AWS SDK for C++ package.
find_package(AWSSDK REQUIRED COMPONENTS ${SERVICE_COMPONENTS})

set(INCLUDE_DIRS
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/include"
)

add_library(s3file STATIC "")
target_sources(s3file
  PRIVATE
    "lib/s3.cc"

  # Only CMake 3.3+ supports PUBLIC sources in targets exported by "install".
    $<$<VERSION_GREATER:CMAKE_VERSION,3.2>:PUBLIC>
    "include/s3.h"
)

# Only sfs needs leveldb and protobuf's header
target_include_directories(s3file SYSTEM PRIVATE "${INCLUDE_DIRS}")

target_link_libraries(s3file
    PRIVATE
    ${AWSSDK_LINK_LIBRARIES}
    ${AWSSDK_PLATFORM_DEPS})

target_link_directories(s3file
    SYSTEM PRIVATE
    ${CURL_LIB}
    ${INCLUDE_DIRS})
