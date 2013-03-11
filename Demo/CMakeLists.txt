find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(chipmunk_demos_include_dirs
  ${chipmunk_SOURCE_DIR}/include/chipmunk
  ${GLUT_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
)

set(chipmunk_demos_libraries
  chipmunk_static
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

if(NOT MSVC)
  list(APPEND chipmunk_demos_libraries m)
endif(NOT MSVC)

file(GLOB chipmunk_demos_source_files "*.c")

include_directories(${chipmunk_demos_include_dirs})
add_executable(chipmunk_demos ${chipmunk_demos_source_files})
target_link_libraries(chipmunk_demos ${chipmunk_demos_libraries})

# Tell MSVC to compile the code as C++.
if(MSVC)
  set_source_files_properties(${chipmunk_demos_source_files} PROPERTIES LANGUAGE CXX)
  set_target_properties(chipmunk_demos PROPERTIES LINKER_LANGUAGE CXX)
endif(MSVC)

if(INSTALL_DEMOS)
  install(TARGETS chipmunk_demos RUNTIME DESTINATION bin)
endif(INSTALL_DEMOS)
