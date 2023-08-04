file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(configdb-shell ${configdb_SRCS} )

if (APPLE)
  target_link_libraries(configdb-shell
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost flatbuffers::flatbuffers ${OPENSSL_LIBRARIES} readline ZLIB::ZLIB
  )
elseif(WIN32)
  target_link_libraries(configdb-shell
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost Boost::container flatbuffers::flatbuffers OpenSSL::Crypto OpenSSL::SSL readline ZLIB::ZLIB
  )
else()
  target_link_libraries(configdb-shell
    PRIVATE nanolog configdb-common configdb-lib configdb-api Boost::boost flatbuffers::flatbuffers crypto ssl readline pthread ZLIB::ZLIB
  )
endif()

set_target_properties(configdb-shell PROPERTIES OUTPUT_NAME configsh)
install(TARGETS configdb-shell DESTINATION bin)
