file(GLOB_RECURSE configdb_SRCS *.cpp *.h)
add_executable(seed-configdb ${configdb_SRCS} )
target_link_libraries(seed-configdb PRIVATE lib api)
install(TARGETS seed-configdb DESTINATION bin)
