find_package(Ruby REQUIRED)

file(GLOB ruby_ext_source_files "ext/*.c")

set(ruby_ext_include_dirs
  ${chipmunk_SOURCE_DIR}/include/chipmunk
  ${RUBY_INCLUDE_PATH}
)

set(ruby_ext_libraries
  chipmunk_static
  ${RUBY_LIBRARY}
)

include_directories(${ruby_ext_include_dirs})
add_library(ruby_ext MODULE ${ruby_ext_source_files})
target_link_libraries(ruby_ext ${ruby_ext_libraries})

set_target_properties(ruby_ext PROPERTIES OUTPUT_NAME chipmunk PREFIX "") # else the name would be libruby_ext
if(APPLE)
  set_target_properties(ruby_ext PROPERTIES SUFFIX ".bundle") # else it get's .so, even if the type ls module!
endif(APPLE)

install(TARGETS ruby_ext DESTINATION ${RUBY_SITEARCH_DIR})
install(FILES "chipmunk_object.rb" DESTINATION ${RUBY_SITELIB_DIR})

