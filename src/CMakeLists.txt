set(chipmunk_public_header
  chipmunk.h
  chipmunk_unsafe.h
  cpArbiter.h
  cpArray.h
  cpBB.h
  cpBody.h
  cpCollision.h
  cpHashSet.h
  cpPolyShape.h
  cpShape.h
  cpSpace.h
  cpSpaceHash.h
  cpVect.h
)

set(chipmunk_source_files
  chipmunk.c
  cpArbiter.c
  cpArray.c
  cpBB.c
  cpBody.c
  cpCollision.c
  cpHashSet.c
  cpPolyShape.c
  cpShape.c
  cpSpace.c
  cpSpaceHash.c
  cpVect.c
  constraints/cpBreakableJoint.c
  constraints/cpConstraint.c
  constraints/cpDampedRotarySpring.c
  constraints/cpDampedSpring.c
  constraints/cpGearJoint.c
  constraints/cpGrooveJoint.c
  constraints/cpPinJoint.c
  constraints/cpPivotJoint.c
  constraints/cpRotaryLimitJoint.c
  constraints/cpSimpleMotor.c
  constraints/cpSlideJoint.c
)

if(BUILD_SHARED)
  add_library(chipmunk SHARED
    ${chipmunk_source_files}
  )
  # set the lib's version number
  set_target_properties(chipmunk PROPERTIES VERSION 4)
  install(TARGETS chipmunk LIBRARY DESTINATION lib)
endif(BUILD_SHARED)

if(BUILD_STATIC)
  add_library(chipmunk_static STATIC
    ${chipmunk_source_files}
  )
  # Sets chipmunk_static to output "libchipmunk.a" not "libchipmunk_static.a"
  set_target_properties(chipmunk_static PROPERTIES OUTPUT_NAME chipmunk)
  if(INSTALL_STATIC)
    install(TARGETS chipmunk_static ARCHIVE DESTINATION lib)
  endif(INSTALL_STATIC)
endif(BUILD_STATIC)

if(BUILD_SHARED OR INSTALL_STATIC)
  # FIXME: change to PUBLIC_HEADER to allow building frameworks
  install(FILES ${chipmunk_public_header} DESTINATION include/chipmunk)
endif(BUILD_SHARED OR INSTALL_STATIC)
