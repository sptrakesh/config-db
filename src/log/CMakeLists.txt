file(GLOB_RECURSE log_SRCS *.cpp *.h)
add_library(nanolog STATIC ${log_SRCS})
install(TARGETS nanolog DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/log
  DESTINATION include
  FILES_MATCHING PATTERN "*.h*")
