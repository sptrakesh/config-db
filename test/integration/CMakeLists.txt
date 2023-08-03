Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.7
)

FetchContent_MakeAvailable(Catch2)

file( GLOB_RECURSE test_SRCS *.cpp *.h )
add_executable(integration ${test_SRCS} )

if (APPLE)
  target_link_libraries(integration
    PRIVATE nanolog configdb-common configdb-lib configdb-api Catch2::Catch2 Boost::boost flatbuffers::flatbuffers ${OPENSSL_LIBRARIES} ZLIB::ZLIB
  )
else()
  target_link_libraries(integration
    PRIVATE nanolog configdb-common configdb-lib configdb-api Catch2::Catch2 Boost::boost flatbuffers::flatbuffers crypto ssl ZLIB::ZLIB
  )
endif()

