file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_library(configdb-lib STATIC ${configdb_SRCS})
set_target_properties(configdb-lib PROPERTIES OUTPUT_NAME configdb)
install(TARGETS configdb-lib DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/lib
  DESTINATION include/configdb
  FILES_MATCHING PATTERN "*.h*")
