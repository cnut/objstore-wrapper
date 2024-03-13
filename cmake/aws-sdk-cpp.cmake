# cmake -DWITH_OBJSTORE=system|bundled
# bundled is the default

SET(OBJSTORE_INCLUDE_DIR "system default header")
SET(OBJSTORE_LIBRARY_PATH "system default lib path")
SET(OBJSTORE_LIBRARIES "aws-cpp-sdk-s3;aws-cpp-sdk-core")
SET(OBJSTORE_PLATFORM_DEPS "pthread;curl")

MACRO(SHOW_OBJSTORE_INFO)
  MESSAGE(STATUS "OBJSTORE_INCLUDE_DIR: ${OBJSTORE_INCLUDE_DIR}")
  MESSAGE(STATUS "OBJSTORE_LIBRARY_PATH: ${OBJSTORE_LIBRARY_PATH}")
  MESSAGE(STATUS "OBJSTORE_LIBRARIES: ${OBJSTORE_LIBRARIES}")
  MESSAGE(STATUS "OBJSTORE_PLATFORM_DEPS: ${OBJSTORE_PLATFORM_DEPS}")
ENDMACRO()

MACRO(PREPARE_BUNDLED_OJBSTORE)
  SET(OBJSTORE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/aws-sdk-cpp")

  INCLUDE(ExternalProject)
  ExternalProject_Add(
    objstore-ext-proj
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/3rd/aws-sdk-cpp"
    #GIT_REPOSITORY "https://github.com/aws/aws-sdk-cpp.git"
    #GIT_TAG "1.11.283"
    #UPDATE_COMMAND "" # "git submodule update --init --recursive"
    BUILD_COMMAND cmake -S ${PROJECT_SOURCE_DIR}/3rd/aws-sdk-cpp -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY=s3 -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${OBJSTORE_INSTALL_PREFIX} && make -j
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND cmake --install ${PROJECT_SOURCE_DIR}/3rd/aws-sdk-cpp
  )
ENDMACRO()

MACRO(FIND_SYSTEM_OBJSTORE)
  FIND_PACKAGE(AWSSDK REQUIRED COMPONENTS "s3")
ENDMACRO()

MACRO (CHECK_PREPARE_OBJSTORE)
  IF(NOT WITH_OBJSTOR)
    SET(WITH_OBJSTOR "bundled" CACHE STRING "By default use bundled ObjStore library")
  ENDIF()

  IF(WITH_OBJSTOR STREQUAL "bundled")
    MESSAGE(STATUS "WITH_OBJSTOR is bundled, download aws-sdk-cpp and compile it")
    PREPARE_BUNDLED_OJBSTORE()
    MESSAGE(STATUS "aws-sdk-cpp will be installed to ${OBJSTORE_INSTALL_PREFIX} in the compile phase")

    # Set the variables for the project
    SET(OBJSTORE_INCLUDE_DIR "${OBJSTORE_INSTALL_PREFIX}/include")
    SET(OBJSTORE_LIBRARY_PATH "${OBJSTORE_INSTALL_PREFIX}/lib64")
    SET(OBJSTORE_LIBRARY "aws-cpp-sdk-s3;aws-cpp-sdk-core")
    SET(OBJSTORE_PLATFORM_DEPS "pthread;curl")
    # Prepare include and ld path
    INCLUDE_DIRECTORIES(${OBJSTORE_INCLUDE_DIR})
    LINK_DIRECTORIES(${OBJSTORE_LIBRARY_PATH}) 
  ELSEIF(WITH_OBJSTOR STREQUAL "system")
    MESSAGE(STATUS "WITH_OBJSTOR is system, use system aws s3 lib")
    FIND_SYSTEM_OBJSTORE()
    # avoid error when add_dependencies(objstore-ext-proj) in the main project
    ADD_CUSTOM_TARGET(objstore-ext-proj COMMAND "")
    SET(OBJSTORE_LIBRARY ${AWSSDK_LINK_LIBRARIES})
    SET(OBJSTORE_PLATFORM_DEPS ${OBJSTORE_PLATFORM_DEPS})
  ELSE()
    MESSAGE(FATAL_ERROR "WITH_OBJSTOR must be bundled or system")
  ENDIF()

  SHOW_OBJSTORE_INFO()
ENDMACRO()
