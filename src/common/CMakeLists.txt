set(Target_Name "common")
file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_library(${Target_Name} STATIC ${configdb_SRCS})
add_library(configdb::common ALIAS ${Target_Name})
target_compile_features(${Target_Name} INTERFACE cxx_std_23)
set_target_properties(${Target_Name} PROPERTIES OUTPUT_NAME configcommon)
target_include_directories(${Target_Name}
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  ${Boost_INCLUDE_DIR}
)
target_link_libraries(${Target_Name} PRIVATE flatbuffers::flatbuffers)
target_link_libraries(${Target_Name} INTERFACE nanolog Boost::boost Boost::json)
install(TARGETS ${Target_Name} EXPORT ConfigDbCommonTargets DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/common
  DESTINATION include/configdb
  FILES_MATCHING PATTERN "*.h*")
install(EXPORT ConfigDbCommonTargets
  FILE ConfigDbCommonTargets.cmake
  NAMESPACE configdb::
  DESTINATION lib/cmake/ConfigDb
)