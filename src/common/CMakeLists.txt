file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_library(configdb-common STATIC ${configdb_SRCS})
set_target_properties(configdb-common PROPERTIES OUTPUT_NAME configcommon)
install(TARGETS configdb-common DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/common
  DESTINATION include/configdb
  FILES_MATCHING PATTERN "*.h*")
