file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(configdb-cli ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(configdb-cli
    PRIVATE configdb-common configdb-lib configdb-api Boost::boost flatbuffers ${OPENSSL_LIBRARIES} z
  )
else()
  target_link_libraries(configdb-cli
    PRIVATE configdb-common configdb-lib configdb-api Boost::boost flatbuffers crypto ssl z
  )
endif()

set_target_properties(configdb-cli PROPERTIES OUTPUT_NAME configctl)
install(TARGETS configdb-cli DESTINATION bin)
