file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_library(configdb-api STATIC ${configdb_SRCS})
set_target_properties(configdb-api PROPERTIES OUTPUT_NAME configapi)
install(TARGETS configdb-api DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/api
  DESTINATION include/configdb
  FILES_MATCHING PATTERN "*.h*")
