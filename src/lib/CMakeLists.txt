set(Target_Name "lib")
file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_library(${Target_Name} STATIC ${configdb_SRCS})
add_library(configdb::lib ALIAS ${Target_Name})
target_compile_features(${Target_Name} INTERFACE cxx_std_23)
set_target_properties(${Target_Name} PROPERTIES OUTPUT_NAME configdb)
target_include_directories(${Target_Name}
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  ${Boost_INCLUDE_DIR}
)
if (UNIX)
  if (APPLE)
    target_link_libraries(${Target_Name} INTERFACE common RocksDB::rocksdb ${OPENSSL_LIBRARIES} ${JEMALLOC_LIBRARY} ZLIB::ZLIB)
  else (APPLE)
    target_link_libraries(${Target_Name} INTERFACE common RocksDB::rocksdb crypto ssl snappy pthread jemalloc ZLIB::ZLIB)
  endif (APPLE)
else (UNIX)
  target_link_libraries(${Target_Name} INTERFACE common Boost::container RocksDB::rocksdb-shared OpenSSL::SSL OpenSSL::Crypto Snappy::snappy ZLIB::ZLIB)
endif (UNIX)
install(TARGETS ${Target_Name} EXPORT ConfigDbLibTargets DESTINATION lib)
install(TARGETS ${Target_Name} DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/lib
  DESTINATION include/configdb
  FILES_MATCHING PATTERN "*.h*")
install(EXPORT ConfigDbLibTargets
    FILE ConfigDbLibTargets.cmake
    NAMESPACE configdb::
    DESTINATION lib/cmake/ConfigDb
)
