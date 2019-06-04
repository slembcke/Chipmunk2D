cmake_policy(SET CMP0015 NEW) # Convert relative paths

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

set(chipmunk_demos_include_dirs
	${chipmunk_SOURCE_DIR}/include
	${OPENGL_INCLUDE_DIR}
)

set(chipmunk_demos_libraries
	chipmunk_static
	${OPENGL_LIBRARIES}
)

file(GLOB chipmunk_demos_source_files "*.c")

if(APPLE)
	FIND_LIBRARY(APPKIT AppKit)
	FIND_LIBRARY(IOKIT IOKit)
	
	list(APPEND chipmunk_demos_libraries ${APPKIT} ${IOKIT})
	list(APPEND chipmunk_demos_source_files "sokol/sokol.m")
	set_property(SOURCE "sokol/sokol.m" APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
else()
	list(APPEND chipmunk_demos_source_files "sokol/sokol.c")
endif(APPLE)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	list(APPEND chipmunk_demos_libraries dl X11)
endif()

if(NOT MSVC)
	list(APPEND chipmunk_demos_libraries m pthread)
endif(NOT MSVC)

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	list(APPEND chipmunk_demos_libraries BlocksRuntime)
endif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

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
