INCLUDE_DIRECTORIES(${CHIPMUNK_SOURCE_DIR}/src)

SET(chipmunk_includes
	chipmunk.h
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

set(chipmunk_constraint_source_files
    cpBreakableJoint.c
    cpConstraint.c
    cpDampedRotarySpring.c
    cpDampedSpring.c
    cpGearJoint.c
    cpGrooveJoint.c
    cpPinJoint.c
    cpPivotJoint.c
    cpRotaryLimitJoint.c
    cpSimpleMotor.c
    cpSlideJoint.c
    )
FOREACH(f ${chipmunk_constraint_source_files})
    LIST(APPEND chipmunk_source_files constraints/${f})
ENDFOREACH(f)

list(APPEND chipmunk_source_files
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
    )

ADD_LIBRARY(chipmunk SHARED 
    ${chipmunk_source_files}
)

ADD_LIBRARY(chipmunk_static STATIC 
    ${chipmunk_source_files}
)

INSTALL(FILES ${chipmunk_includes} DESTINATION include/chipmunk)
SET_TARGET_PROPERTIES(chipmunk_static PROPERTIES OUTPUT_NAME chipmunk) #Sets chipmunk_static to output "libchipmunk.a" not "libchipmunk_static.a"
INSTALL(TARGETS chipmunk chipmunk_static
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
SET_TARGET_PROPERTIES(chipmunk PROPERTIES VERSION 4)

