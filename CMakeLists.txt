cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif(POLICY CMP0079)

project(config-db)

# Enable C++20 features (coroutine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
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
    message(STATUS "Found OpenSSL keg installed by Homebrew at ${BREW_JEMALLOC_PREFIX}")
    set(JEMALLOC_ROOT_DIR "${BREW_JEMALLOC_PREFIX}/")
    set(JEMALLOC_LIBRARIES "${BREW_JEMALLOC_PREFIX}/lib")
    set(JEMALLOC_LIBRARY "${BREW_JEMALLOC_PREFIX}/lib/libjemalloc.dylib")
  endif()
endif()

if (APPLE)
  find_package(OpenSSL REQUIRED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts")
endif(APPLE)

find_package(Boost)
find_package(RocksDB)
find_package(flatbuffers)
find_package(Threads)

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
    /usr/local/nghttp2/include
    /usr/local/rocksdb/include
  )
  LINK_DIRECTORIES(
    /usr/local/flatbuffers/lib
    /usr/local/nghttp2/lib
    /usr/local/rocksdb/lib
  )
  set(CMAKE_MACOSX_RPATH 1)
elseif (UNIX)
  include_directories(
    /opt/local/include
  )
  link_directories(
    /opt/local/lib
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
