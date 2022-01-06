file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(configdb-shell ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(configdb-shell
    PRIVATE configdb-common configdb-lib configdb-api Boost::boost flatbuffers ${OPENSSL_LIBRARIES} readline z
  )
else()
  target_link_libraries(configdb-shell
    PRIVATE configdb-common configdb-lib configdb-api Boost::boost flatbuffers crypto ssl readline z
  )
endif()

set_target_properties(configdb-shell PROPERTIES OUTPUT_NAME configsh)
install(TARGETS configdb-shell DESTINATION bin)
