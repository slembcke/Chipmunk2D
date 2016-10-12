cmake_policy(SET CMP0015 NEW) # Convert relative paths

find_package(OpenGL REQUIRED)

if(APPLE)
	set(GLFW_INCLUDE_DIR ../xcode/libglfw/include)
	set(chipmunk_demos_library_dirs ../xcode/libglfw/lib)
	set(GLFW_LIBRARIES glfw)
else(APPLE)
	FIND_PATH(GLFW_INCLUDE_DIR GLFW/glfw3.h /usr/include/GL)
	FIND_LIBRARY(GLFW_LIBRARIES glfw3 /usr/lib)
endif(APPLE)

set(chipmunk_demos_include_dirs
  ${chipmunk_SOURCE_DIR}/include
  ${GLFW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
)

set(chipmunk_demos_libraries
  chipmunk_static
  ${GLFW_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

if(APPLE)
	FIND_LIBRARY(APPKIT AppKit)
	FIND_LIBRARY(IOKIT IOKit)
	
	list(APPEND chipmunk_demos_libraries ${APPKIT} ${IOKIT})
endif(APPLE)

if(NOT MSVC)
  list(APPEND chipmunk_demos_libraries m)
endif(NOT MSVC)

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  list(APPEND chipmunk_demos_libraries BlocksRuntime)
endif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

file(GLOB chipmunk_demos_source_files "*.c")

include_directories(${chipmunk_demos_include_dirs})
link_directories(${chipmunk_demos_library_dirs})
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
