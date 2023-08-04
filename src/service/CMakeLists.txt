file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(configdb ${configdb_SRCS} )

if (HTTP_SERVER)
  if (UNIX)
    if (APPLE)
      target_link_libraries(configdb
          PRIVATE nanolog configdb-common configdb-lib Boost::boost nghttp2_asio RocksDB::rocksdb flatbuffers::flatbuffers ${OPENSSL_LIBRARIES} ${JEMALLOC_LIBRARY} ZLIB::ZLIB
      )
    else()
      target_link_libraries(configdb
          PRIVATE nanolog configdb-common configdb-lib Boost::boost nghttp2_asio nghttp2 RocksDB::rocksdb flatbuffers::flatbuffers crypto ssl snappy pthread jemalloc ZLIB::ZLIB
      )
    endif()
  else()
    target_link_libraries(configdb
      PRIVATE nanolog configdb-common configdb-lib Boost::boost Boost::container Boost::json nghttp2_asio nghttp2 RocksDB::rocksdb-shared flatbuffers::flatbuffers OpenSSL::SSL OpenSSL::Crypto Snappy::snappy ZLIB::ZLIB
    )
  endif (UNIX)
else()
  if (UNIX)
    if (APPLE)
      target_link_libraries(configdb
          PRIVATE nanolog configdb-common configdb-lib Boost::boost RocksDB::rocksdb flatbuffers::flatbuffers ${OPENSSL_LIBRARIES} ${JEMALLOC_LIBRARY} ZLIB::ZLIB
      )
    else()
      target_link_libraries(configdb
          PRIVATE nanolog configdb-common configdb-lib Boost::boost RocksDB::rocksdb flatbuffers::flatbuffers crypto ssl snappy pthread jemalloc ZLIB::ZLIB
      )
    endif()
  else()
    target_link_libraries(configdb
      PRIVATE nanolog configdb-common configdb-lib Boost::boost Boost::container Boost::json RocksDB::rocksdb-shared flatbuffers::flatbuffers OpenSSL::SSL OpenSSL::Crypto Snappy::snappy ZLIB::ZLIB
    )
  endif (UNIX)
endif (HTTP_SERVER)

install(TARGETS configdb DESTINATION bin)
