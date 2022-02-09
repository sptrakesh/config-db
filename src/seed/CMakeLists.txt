file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(seed-configdb ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(seed-configdb
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost rocksdb flatbuffers ${OPENSSL_LIBRARIES} z
  )
else()
  target_link_libraries(seed-configdb
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost rocksdb flatbuffers crypto ssl pthread z
  )
endif()

install(TARGETS seed-configdb DESTINATION bin)
