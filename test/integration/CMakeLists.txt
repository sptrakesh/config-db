Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
)

FetchContent_MakeAvailable(Catch2)
find_package(cpr REQUIRED)

file( GLOB_RECURSE test_SRCS *.cpp *.h )
add_executable(integration ${test_SRCS} )

if (APPLE)
  target_link_libraries(integration
    PRIVATE nanolog configdb-common configdb-lib configdb-api Catch2::Catch2 Boost::boost Boost::json cpr::cpr flatbuffers::flatbuffers ${OPENSSL_LIBRARIES} ZLIB::ZLIB
  )
else()
  target_link_libraries(integration
    PRIVATE nanolog configdb-common configdb-lib configdb-api Catch2::Catch2 Boost::boost Boost::json cpr::cpr flatbuffers::flatbuffers crypto ssl ZLIB::ZLIB
  )
endif()

