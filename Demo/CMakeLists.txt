find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(chipmunk_demos_include_dirs
  ${chipmunk_SOURCE_DIR}/src
  ${GLUT_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
)

set(chipmunk_demos_libraries
  chipmunk_static
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

set(chipmunk_demos_source_files
  ChipmunkDemo.c
  LogoSmash.c
  PyramidStack.c
  Plink.c
  Tumble.c
  PyramidTopple.c
  Bounce.c
  Planet.c
  Springies.c
  Pump.c
  WalkBot.c
  TheoJansen.c
  MagnetsElectric.c
  drawSpace.c
  UnsafeOps.c
  Query.c
	OneWay.c,
	Player.c,
	Sensors.c,
)

include_directories(${chipmunk_demos_include_dirs})
add_executable(chipmunk_demos ${chipmunk_demos_source_files})
target_link_libraries(chipmunk_demos ${chipmunk_demos_libraries})

if(INSTALL_DEMOS)
  install(TARGETS chipmunk_demos RUNTIME DESTINATION bin)
endif(INSTALL_DEMOS)
