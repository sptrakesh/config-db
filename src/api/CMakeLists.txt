set(Target_Name "api")
file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_library(${Target_Name} STATIC ${configdb_SRCS})
add_library(configdb::api ALIAS ${Target_Name})
target_compile_features(${Target_Name} INTERFACE cxx_std_23)
set_target_properties(${Target_Name} PROPERTIES OUTPUT_NAME configapi)
target_compile_definitions(${Target_Name} PRIVATE _FORTIFY_SOURCE=2)
target_include_directories(${Target_Name}
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
if (UNIX)
  target_link_libraries(${Target_Name} INTERFACE common ${OPENSSL_LIBRARIES} ${JEMALLOC_LIBRARY})
else (UNIX)
  target_link_libraries(${Target_Name} INTERFACE common OpenSSL::SSL OpenSSL::Crypto)
endif (UNIX)
install(TARGETS ${Target_Name} EXPORT ConfigDbApiTargets DESTINATION lib)
install(TARGETS ${Target_Name} DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/api
  DESTINATION include/configdb
  FILES_MATCHING PATTERN "*.h*")
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/certs
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*")
install(EXPORT ConfigDbApiTargets
    FILE ConfigDbApiTargets.cmake
    NAMESPACE configdb::
    DESTINATION lib/cmake/ConfigDb
)
