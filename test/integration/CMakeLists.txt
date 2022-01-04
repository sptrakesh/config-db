Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.7
)

FetchContent_MakeAvailable(Catch2)

file( GLOB_RECURSE test_SRCS *.cpp *.h )

set( test_SRCS ${test_SRCS} )

# Create the application
add_executable(integration ${test_SRCS} )

target_link_libraries(integration
  PRIVATE configdb-common configdb-lib configdb-api Catch2::Catch2 Boost::boost flatbuffers z
)

