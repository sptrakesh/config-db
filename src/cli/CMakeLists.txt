file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(configdb-cli ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(configdb-cli
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost flatbuffers::flatbuffers ${OPENSSL_LIBRARIES} ZLIB::ZLIB
  )
elseif(WIN32)
  target_link_libraries(configdb-cli
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost flatbuffers::flatbuffers OpenSSL::Crypto OpenSSL::SSL ZLIB::ZLIB
  )
else()
  target_link_libraries(configdb-cli
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost flatbuffers::flatbuffers crypto ssl pthread ZLIB::ZLIB
  )
endif()

set_target_properties(configdb-cli PROPERTIES OUTPUT_NAME configctl)
install(TARGETS configdb-cli DESTINATION bin)
