cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# For INTERPROCEDURAL_OPTIMIZATION
if (POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif()

if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif (POLICY CMP0079)

if (POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

file(STRINGS "docker/env.sh" ver REGEX "VERSION=([0-9]*)")
string(REGEX REPLACE "VERSION='" "" project_version ${ver})
string(REGEX REPLACE "'" "" project_version ${project_version})
message("Project version: ${project_version}")

project(config-db LANGUAGES CXX VERSION ${project_version})

if (WIN32)
  if (POLICY CMP0111)
    cmake_policy(SET CMP0111 NEW)
  endif (POLICY CMP0111)
  if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW) 
  endif (POLICY CMP0091)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/EHsc /await:strict /std:c++20 /utf-8")
    add_compile_definitions(_WIN32_WINNT=0x0A000007)
  endif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  set(OPENSSL_ROOT "/opt/src/vcpkg/packages/openssl_arm64-windows" CACHE STRING "Where openssl is installed by vcpkg")
  set(READLINE_ROOT "/opt/src/vcpkg/packages/readline-win32_arm64-windows" CACHE STRING "Where readline is installed by vcpkg")
  set(ROCKSDB_ROOT "/opt/src/vcpkg/packages/rocksdb_arm64-windows" CACHE STRING "Where rocksdb is installed by vcpkg")
endif (WIN32)

# Enable C++23 features
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

if (WIN32)
  option(HTTP_SERVER "enable HTTP server" OFF)
else (WIN32)
  option(HTTP_SERVER "enable HTTP server" ON)
endif (WIN32)

if (HTTP_SERVER)
  message(STATUS "HTTP server enabled.  Requires nghttp2-asio")
  add_definitions(-DCONFIG_DB_HTTP_SERVER)
endif (HTTP_SERVER)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

if (UNIX)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif (UNIX)
set(Boost_USE_MULTITHREADED ON)
set(OPENSSL_USE_STATIC_LIBS TRUE)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  execute_process(
    COMMAND brew --prefix OpenSSL
    RESULT_VARIABLE BREW_OPENSSL
    OUTPUT_VARIABLE BREW_OPENSSL_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (BREW_OPENSSL EQUAL 0 AND EXISTS "${BREW_OPENSSL_PREFIX}")
    message(STATUS "Found OpenSSL keg installed by Homebrew at ${BREW_OPENSSL_PREFIX}")
    list(APPEND CMAKE_PREFIX_PATH "${BREW_OPENSSL_PREFIX}/")
  endif (BREW_OPENSSL EQUAL 0 AND EXISTS "${BREW_OPENSSL_PREFIX}")

  execute_process(
      COMMAND brew --prefix libnghttp2
      RESULT_VARIABLE BREW_NGHTTP2
      OUTPUT_VARIABLE BREW_NGHTTP2_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (BREW_NGHTTP2 EQUAL 0 AND EXISTS "${BREW_NGHTTP2_PREFIX}")
    message(STATUS "Found libnghttp2 keg installed by Homebrew at ${BREW_NGHTTP2_PREFIX}")
    list(APPEND CMAKE_PREFIX_PATH "${BREW_NGHTTP2_PREFIX}/")
  endif()

  execute_process(
    COMMAND brew --prefix jemalloc
    RESULT_VARIABLE BREW_JEMALLOC
    OUTPUT_VARIABLE BREW_JEMALLOC_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (BREW_JEMALLOC EQUAL 0 AND EXISTS "${BREW_JEMALLOC_PREFIX}")
    message(STATUS "Found Jemalloc keg installed by Homebrew at ${BREW_JEMALLOC_PREFIX}")
    set(JEMALLOC_ROOT_DIR "${BREW_JEMALLOC_PREFIX}/")
    set(JEMALLOC_LIBRARIES "${BREW_JEMALLOC_PREFIX}/lib")
    set(JEMALLOC_LIBRARY "${BREW_JEMALLOC_PREFIX}/lib/libjemalloc.dylib")
  endif (BREW_JEMALLOC EQUAL 0 AND EXISTS "${BREW_JEMALLOC_PREFIX}")
endif (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")

find_package(Boost REQUIRED COMPONENTS container json url)
find_package(RocksDB REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

if (WIN32)
  find_package(Snappy REQUIRED)
endif (WIN32)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

if (UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")

  if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")
    message(STATUS "Enabling mold linker")
  elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")
    message(STATUS "Enabling mold linker")
  endif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64" )
endif (UNIX AND NOT APPLE)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
  include_directories(
    /usr/local/boost/include
    /usr/local/flatbuffers/include
    /usr/local/rocksdb/include
  )

  if (HTTP_SERVER)
    include_directories(
      /usr/local/nghttp2/include
      ${BREW_NGHTTP2_PREFIX}/include
    )
    link_directories(
      /usr/local/nghttp2/lib
      ${BREW_NGHTTP2_PREFIX}/lib
    )
  endif (HTTP_SERVER)

  set(CMAKE_MACOSX_RPATH 1)
elseif (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-tautological-compare")
  include_directories(/opt/local/include)
  link_directories(/opt/local/lib)
elseif (WIN32)
  include_directories(
    /opt/local/include
    ${OPENSSL_ROOT}/include
    ${READLINE_ROOT}/include
    ${ROCKSDB_ROOT}/include
  )
  link_directories(
    ${OPENSSL_ROOT}/lib
    ${READLINE_ROOT}/lib
  )
endif (APPLE)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /opt/spt)
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

add_subdirectory(src/log)
add_subdirectory(src/common)
add_subdirectory(src/lib)
add_subdirectory(src/service)
add_subdirectory(src/api)
add_subdirectory(src/seed)
add_subdirectory(src/shell)
add_subdirectory(src/cli)

if (BUILD_TESTING)
  enable_testing()
  add_subdirectory(test/unit)
  add_subdirectory(test/integration)
endif (BUILD_TESTING)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "ConfigDbConfigVersion.cmake"
  VERSION ${configdb_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES "ConfigDbConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/ConfigDbConfigVersion.cmake" DESTINATION lib/cmake/ConfigDb)
