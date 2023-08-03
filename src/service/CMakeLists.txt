file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(configdb ${configdb_SRCS} )

if (UNIX)
if (APPLE)
  target_link_libraries(configdb
    PRIVATE nanolog configdb-common configdb-lib Boost::boost nghttp2_asio rocksdb flatbuffers ${OPENSSL_LIBRARIES} ${JEMALLOC_LIBRARY} z
  )
else()
  target_link_libraries(configdb
    PRIVATE nanolog configdb-common configdb-lib Boost::boost nghttp2_asio nghttp2 rocksdb flatbuffers OpenSSL::SSL OpenSSL::Crypto snappy pthread jemalloc z
  )
endif(APPLE)
else()
  target_link_libraries(configdb
    PRIVATE nanolog configdb-common configdb-lib Boost::boost nghttp2_asio nghttp2 rocksdb flatbuffers OpenSSL::SSL OpenSSL::Crypto Snappy::snappy ZLIB::ZLIB
  )
endif(UNIX)

install(TARGETS configdb DESTINATION bin)
