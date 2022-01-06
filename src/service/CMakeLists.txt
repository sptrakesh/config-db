file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(configdb ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(configdb
    PRIVATE configdb-common configdb-lib Boost::boost nghttp2_asio rocksdb flatbuffers ${OPENSSL_LIBRARIES} z
  )
else()
  target_link_libraries(configdb
    PRIVATE configdb-common configdb-lib Boost::boost nghttp2_asio nghttp2 rocksdb flatbuffers crypto ssl z
  )
endif()

install(TARGETS configdb DESTINATION bin)
