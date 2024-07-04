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
  target_link_libraries(integration PRIVATE lib api Catch2::Catch2 cpr::cpr)
else()
  target_link_libraries(integration PRIVATE lib api Catch2::Catch2 cpr::cpr)
endif()

