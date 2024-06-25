cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif(POLICY CMP0079)

project(config-db)

if (WIN32)
  if (POLICY CMP0111)
    cmake_policy(SET CMP0111 NEW)
  endif (POLICY CMP0111)
  if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW) 
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/EHsc /await:strict /std:c++20 /utf-8")
    add_compile_definitions(_WIN32_WINNT=0x0A000007)
  endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

  set(OPENSSL_ROOT "/opt/src/vcpkg/packages/openssl_arm64-windows" CACHE STRING "Where openssl is installed by vcpkg")
  set(READLINE_ROOT "/opt/src/vcpkg/packages/readline-win32_arm64-windows" CACHE STRING "Where readline is installed by vcpkg")
  set(ROCKSDB_ROOT "/opt/src/vcpkg/packages/rocksdb_arm64-windows" CACHE STRING "Where rocksdb is installed by vcpkg")
endif(WIN32)

# Enable C++23 features
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

option(HTTP_SERVER "enable HTTP server" ON)
if (HTTP_SERVER)
  message(STATUS "HTTP server enabled.  Requires nghttp2-asio")
  add_definitions(-DCONFIG_DB_HTTP_SERVER)
endif(HTTP_SERVER)

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
    set(OPENSSL_ROOT_DIR "${BREW_OPENSSL_PREFIX}/")
    set(OPENSSL_INCLUDE_DIR "${BREW_OPENSSL_PREFIX}/include")
    set(OPENSSL_LIBRARIES "${BREW_OPENSSL_PREFIX}/lib")
    set(OPENSSL_CRYPTO_LIBRARY "${BREW_OPENSSL_PREFIX}/lib/libcrypto.dylib")
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
  endif()
endif()

if (APPLE)
  find_package(OpenSSL REQUIRED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts")
endif(APPLE)

if (WIN32)
  find_package(OpenSSL REQUIRED)
  find_package(Snappy REQUIRED)
endif (WIN32)

find_package(Boost REQUIRED COMPONENTS container json)
find_package(RocksDB REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

if (UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
endif(UNIX AND NOT APPLE)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
  include_directories(
    /usr/local/flatbuffers/include
    /usr/local/rocksdb/include
  )

  if (HTTP_SERVER)
    include_directories(/usr/local/nghttp2/include)
    link_directories(/usr/local/nghttp2/lib)
  endif (HTTP_SERVER)

  set(CMAKE_MACOSX_RPATH 1)
elseif (UNIX)
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
endif()

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /opt/spt)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

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
endif()
