file(GLOB_RECURSE configdb_SRCS *.cpp *.hpp)
add_executable(configdb-shell ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(configdb-shell PRIVATE lib api readline)
elseif(WIN32)
  target_link_libraries(configdb-shell PRIVATE lib api readline)
else()
  target_link_libraries(configdb-shell PRIVATE lib api readline pthread)
endif()

set_target_properties(configdb-shell PROPERTIES OUTPUT_NAME configsh)
install(TARGETS configdb-shell DESTINATION bin)
