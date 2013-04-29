cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0001 NEW) # don't use MAKE_BACKWARDS_COMPATIBILITY but policies instead

project(chipmunk)

# to change the prefix, run cmake with the parameter:
#   -D CMAKE_INSTALL_PREFIX=/my/prefix

# to change the build type, run cmake with the parameter:
#   -D CMAKE_BUILD_TYPE=<build-type>
# run "cmake --help-variable CMAKE_BUILD_TYPE" for details
if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

# other options for the build, you can i.e. activate the shared library by passing
#   -D BUILD_SHARED=ON
# to cmake. Other options analog
option(BUILD_DEMOS "Build the demo applications" ON)
option(INSTALL_DEMOS "Install the demo applications" OFF)
option(BUILD_SHARED "Build and install the shared library" ON)
option(BUILD_STATIC "Build as static library" ON)
option(INSTALL_STATIC "Install the static library" ON)

# sanity checks...
if(INSTALL_DEMOS)
  set(BUILD_DEMOS ON FORCE)
endif()

# these need the static lib too
if(BUILD_DEMOS OR INSTALL_STATIC)
  set(BUILD_STATIC ON FORCE)
endif()

if(NOT MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99") # always use gnu99
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffast-math") # extend release-profile with fast-math
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall") # extend debug-profile with -Wall
endif()

add_subdirectory(src)

if(BUILD_DEMOS)
  add_subdirectory(Demo)
endif()
