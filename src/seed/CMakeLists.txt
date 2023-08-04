file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(seed-configdb ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(seed-configdb
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost RocksDB::rocksdb flatbuffers::flatbuffers ${OPENSSL_LIBRARIES} ZLIB::ZLIB
  )
elseif (WIN32)
  target_link_libraries(seed-configdb
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost Boost::container Boost::json RocksDB::rocksdb-shared flatbuffers::flatbuffers OpenSSL::Crypto OpenSSL::SSL ZLIB::ZLIB
  )
else()
  target_link_libraries(seed-configdb
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost RocksDB::rocksdb flatbuffers::flatbuffers crypto ssl pthread ZLIB::ZLIB
  )
endif()

install(TARGETS seed-configdb DESTINATION bin)
