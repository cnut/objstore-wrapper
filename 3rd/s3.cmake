# Copyright (c) 2024, ApeCloud Inc Holding Limited.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

# cmake -DWITH_OBJSTORE=system|bundled
# bundled is the default


SET(OBJSTORE_INCLUDE_DIR "/usr/local/include")
SET(OBJSTORE_LIBRARY_PATH "/usr/local/lib64")
SET(OBJSTORE_LIBRARY "aws-cpp-sdk-s3;aws-cpp-sdk-core")
SET(AWSSDK_PLATFORM_DEPS "pthread;curl")

MACRO(SHOW_OBJSTORE_INFO)
  MESSAGE(STATUS "OBJSTORE_INCLUDE_DIR ${OBJSTORE_INCLUDE_DIR}")
  MESSAGE(STATUS "OBJSTORE_LIBRARY_PATH ${OBJSTORE_LIBRARY_PATH}")
  MESSAGE(STATUS "OBJSTORE_LIBRARY ${OBJSTORE_LIBRARY}")
  MESSAGE(STATUS "AWSSDK_PLATFORM_DEPS ${AWSSDK_PLATFORM_DEPS}")
ENDMACRO()

MACRO(MYSQL_USE_BUNDLED_OJBSTORE)
  SET(OBJSTORE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/aws-sdk-cpp")

  include(ExternalProject)
  ExternalProject_Add(
    objstore
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/3rd/aws-sdk-cpp"
    #GIT_REPOSITORY "https://github.com/aws/aws-sdk-cpp.git"
    #GIT_TAG "1.11.283"
    #UPDATE_COMMAND "" # "git submodule update --init --recursive"
    #PATCH_COMMAND ""
    #CONFIGURE_COMMAND ""
    BUILD_COMMAND cmake -S ${PROJECT_SOURCE_DIR}/3rd/aws-sdk-cpp -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY=s3 -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${OBJSTORE_INSTALL_PREFIX} && make -j
    TEST_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND cmake --install ${PROJECT_SOURCE_DIR}/3rd/aws-sdk-cpp
    #cmake -E copy ${PROJECT_SOURCE_DIR}/3rd/leveldb/libleveldb.a ${CMAKE_BINARY_DIR}/lib
  )

  SET(OBJSTORE_INCLUDE_DIR "${OBJSTORE_INSTALL_PREFIX}/include")
  SET(OBJSTORE_LIBRARY_PATH "${OBJSTORE_INSTALL_PREFIX}/lib64")
  SET(OBJSTORE_LIBRARY "aws-cpp-sdk-s3;aws-cpp-sdk-core")
  SET(AWSSDK_PLATFORM_DEPS "pthread;curl")

  INCLUDE_DIRECTORIES(${OBJSTORE_INCLUDE_DIR})
  LINK_DIRECTORIES(${OBJSTORE_LIBRARY_PATH})
  MESSAGE(STATUS "add OBJSTORE_INCLUDE_DIR ${OBJSTORE_INCLUDE_DIR}")
  MESSAGE(STATUS "add OBJSTORE_LIBRARY_PATH ${OBJSTORE_LIBRARY_PATH}")
  MESSAGE(STATUS "add OBJSTORE_LIBRARY ${OBJSTORE_LIBRARY}")
  MESSAGE(STATUS "add AWSSDK_PLATFORM_DEPS ${AWSSDK_PLATFORM_DEPS}")
ENDMACRO()

MACRO(FIND_SYSTEM_OBJSTORE)
  FIND_PACKAGE(AWSSDK REQUIRED COMPONENTS ${SERVICE_COMPONENTS})
  #FIND_PATH(OJBSTORE_INCLUDE_DIR
  #  NAMES lz4frame.h)
  #FIND_LIBRARY(OBJSTORE_SYSTEM_LIBRARY
  #  NAMES lz4)
  #IF (OBJSTORE_INCLUDE_DIR AND OBJSTORE_SYSTEM_LIBRARY)
  #  SET(SYSTEM_OBJSTORE_FOUND 1)
  #  SET(OBJSTORE_LIBRARY ${OBJSTORE_SYSTEM_LIBRARY})
  #ENDIF()
ENDMACRO()

MACRO (MYSQL_CHECK_OBJSTORE)
  IF(NOT WITH_OBJSTOR)
    SET(WITH_OBJSTOR "bundled" CACHE STRING "By default use bundled ObjStore library")
  ENDIF()

  IF(WITH_OBJSTOR STREQUAL "bundled")
    MYSQL_USE_BUNDLED_OJBSTORE()
  ELSEIF(WITH_OBJSTOR STREQUAL "system")
    FIND_SYSTEM_OBJSTORE()
    IF (NOT SYSTEM_OBJSTORE_FOUND)
      MESSAGE(FATAL_ERROR "Cannot find system objstore libraries.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "WITH_OBJSTOR must be bundled or system")
  ENDIF()
  SHOW_OBJSTORE_INFO()
ENDMACRO()
