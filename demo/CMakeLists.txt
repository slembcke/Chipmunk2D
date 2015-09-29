cmake_policy(SET CMP0015 NEW) # Convert relative paths

find_package(OpenGL REQUIRED)

if(APPLE)
	set(GLEW_INCLUDE_DIR ../xcode/libGLEW/include)
	set(GLFW_INCLUDE_DIR ../xcode/libglfw/include)
	set(chipmunk_demos_library_dirs ../xcode/libglfw/lib ../xcode/libGLEW/lib)
	set(GLEW_LIBRARIES GLEW)
	set(GLFW_LIBRARIES glfw)
else(APPLE)
	FIND_PATH(GLFW_INCLUDE_DIR GL/glfw.h /usr/include/GL)
	FIND_PATH(GLEW_INCLUDE_DIR GL/glew.h /usr/include/GL)
	FIND_LIBRARY(GLFW_LIBRARIES glfw /usr/lib)
	FIND_LIBRARY(GLEW_LIBRARIES GLEW /usr/lib)
endif(APPLE)

set(chipmunk_demos_include_dirs
  ${chipmunk_SOURCE_DIR}/include
  ${GLFW_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
)

set(chipmunk_demos_libraries
  chipmunk_static
  ${GLFW_LIBRARIES}
  ${GLEW_LIBRARIES}
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
