file(GLOB chipmunk_source_files "*.c" "constraints/*.c")
file(GLOB chipmunk_public_header "${chipmunk_SOURCE_DIR}/include/chipmunk/*.h")
file(GLOB chipmunk_constraint_header "${chipmunk_SOURCE_DIR}/include/chipmunk/constraints/*.h")

set(CHIPMUNK_VERSION_MAJOR 6)
set(CHIPMUNK_VERSION_MINOR 2)
set(CHIPMUNK_VERSION_PATCH 1)
set(CHIPMUNK_VERSION "${CHIPMUNK_VERSION_MAJOR}.${CHIPMUNK_VERSION_MINOR}.${CHIPMUNK_VERSION_PATCH}")
message("Configuring chipmunk version ${CHIPMUNK_VERSION}")


include_directories(${chipmunk_SOURCE_DIR}/include/chipmunk)

if(BUILD_SHARED)
  add_library(chipmunk SHARED
    ${chipmunk_source_files}
  )
  # Tell MSVC to compile the code as C++.
  if(MSVC)
    set_source_files_properties(${chipmunk_source_files} PROPERTIES LANGUAGE CXX)
    set_target_properties(chipmunk PROPERTIES LINKER_LANGUAGE CXX)
  endif(MSVC)
  # set the lib's version number
  # But avoid on Android because symlinks to version numbered .so's don't work with Android's Java-side loadLibrary.
  if(NOT ANDROID)
    set_target_properties(chipmunk PROPERTIES
      SOVERSION ${CHIPMUNK_VERSION_MAJOR}
      VERSION ${CHIPMUNK_VERSION})
  endif(NOT ANDROID)
  if(ANDROID)
	  # need to explicitly link to the math library because the CMake/Android toolchains may not do it automatically
	  target_link_libraries(chipmunk m)
  endif(ANDROID)
  install(TARGETS chipmunk RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
endif(BUILD_SHARED)

if(BUILD_STATIC)
  add_library(chipmunk_static STATIC
    ${chipmunk_source_files}
  )
  # Tell MSVC to compile the code as C++.
  if(MSVC)
    set_source_files_properties(${chipmunk_source_files} PROPERTIES LANGUAGE CXX)
    set_target_properties(chipmunk_static PROPERTIES LINKER_LANGUAGE CXX)
  endif(MSVC)
  # Sets chipmunk_static to output "libchipmunk.a" not "libchipmunk_static.a"
  set_target_properties(chipmunk_static PROPERTIES OUTPUT_NAME chipmunk)
  if(INSTALL_STATIC)
    install(TARGETS chipmunk_static ARCHIVE DESTINATION lib)
  endif(INSTALL_STATIC)
endif(BUILD_STATIC)

if(BUILD_SHARED OR INSTALL_STATIC)
  # FIXME: change to PUBLIC_HEADER to allow building frameworks
  install(FILES ${chipmunk_public_header} DESTINATION include/chipmunk)
  install(FILES ${chipmunk_constraint_header} DESTINATION include/chipmunk/constraints)
endif(BUILD_SHARED OR INSTALL_STATIC)
