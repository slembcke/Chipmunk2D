/' Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 '/

#ifndef CHIPMUNK_SPACE
#define CHIPMUNK_SPACE	1

extern "c"

''/ @defgroup cpSpace cpSpace

''/ @{



type as cpContactBufferHeader cpContactBufferHeader

type as sub(byval arb as cpArbiter_ ptr) cpSpaceArbiterApplyImpulseFunc



''/ Basic Unit of Simulation in Chipmunk

type cpSpace

	''/ Number of iterations to use in the impulse solver to solve contacts.

	as integer iterations

	

	''/ Gravity to pass to rigid bodies when integrating velocity.

	as cpVect gravity

	

	''/ Damping rate expressed as the fraction of velocity bodies retain each second.

	''/ A value of 0.9 would mean that each body's velocity will drop 10% per second.

	''/ The default value is 1.0, meaning no damping is applied.

	''/ @note This damping value is different than those of cpDampedSpring and cpDampedRotarySpring.

	as cpFloat damping

	

	''/ Speed threshold for a body to be considered idle.

	''/ The default value of 0 means to let the space guess a good threshold based on gravity.

	as cpFloat idleSpeedThreshold

	

	''/ Time a group of bodies must remain idle in order to fall asleep.

	''/ Enabling sleeping also implicitly enables the the contact graph.

	''/ The default value of INFINITY disables the sleeping algorithm.

	as cpFloat sleepTimeThreshold

	

	''/ Amount of encouraged penetration between colliding shapes.

	''/ Used to reduce oscillating contacts and keep the collision cache warm.

	''/ Defaults to 0.1. If you have poor simulation quality,

	''/ increase this number as much as possible without allowing visible amounts of overlap.

	as cpFloat collisionSlop

	

	''/ Determines how fast overlapping shapes are pushed apart.

	''/ Expressed as a fraction of the error remaining after each second.

	''/ Defaults to pow(1.0 - 0.1, 60.0) meaning that Chipmunk fixes 10% of overlap each frame at 60Hz.

	as cpFloat collisionBias

	

	''/ Number of frames that contact information should persist.

	''/ Defaults to 3. There is probably never a reason to change this value.

	as cpTimestamp collisionPersistence

	

	''/ Rebuild the contact graph during each step. Must be enabled to use the cpBodyEachArbiter() function.

	''/ Disabled by default for a small performance boost. Enabled implicitly when the sleeping feature is enabled.

	as cpBool enableContactGraph

	

	''/ User definable data pointer.

	''/ Generally this points to your game's controller or game state

	''/ class so you can access it when given a cpSpace reference in a callback.

	as cpDataPointer data

	

	''/ The designated static body for this space.

	''/ You can modify this body, or replace it with your own static body.

	''/ By default it points to a statically allocated cpBody in the cpSpace struct.

	as cpBody_ ptr staticBody
	
	CP_PRIVATE(as cpTimestamp stamp)
	CP_PRIVATE(as cpFloat curr_dt)

	CP_PRIVATE(as cpArray_ ptr bodies)
	CP_PRIVATE(as cpArray_ ptr rousedBodies)
	CP_PRIVATE(as cpArray_ ptr sleepingComponents)
	

	CP_PRIVATE(as cpSpatialIndex ptr staticShapes)
	CP_PRIVATE(as cpSpatialIndex ptr activeShapes)

	CP_PRIVATE(as cpArray_ ptr arbiters)
	CP_PRIVATE(as cpContactBufferHeader ptr contactBuffersHead)
	CP_PRIVATE(as cpHashSet_ ptr cachedArbiters)
	CP_PRIVATE(as cpArray_ ptr pooledArbiters)
	CP_PRIVATE(as cpArray_ ptr constraints)

	CP_PRIVATE(as cpArray_ ptr allocatedBuffers)
	CP_PRIVATE(as integer locked)

	CP_PRIVATE(as cpHashSet_ ptr collisionHandlers)
	CP_PRIVATE(as cpCollisionHandler defaultHandler)

	CP_PRIVATE(as cpBool skipPostStep)
	CP_PRIVATE(as cpArray_ ptr postStepCallbacks)

	CP_PRIVATE(as cpBody _staticBody)
end type



''/ Allocate a cpSpace.

declare function cpSpaceAlloc() as cpSpace ptr

''/ Initialize a cpSpace.

declare function cpSpaceInit(byval s as cpSpace ptr) as cpSpace ptr

''/ Allocate and initialize a cpSpace.

declare function cpSpaceNew() as cpSpace ptr



''/ Destroy a cpSpace.

declare sub cpSpaceDestroy(byval s as cpSpace ptr)

''/ Destroy and free a cpSpace.

declare sub cpSpaceFree(byval s as cpSpace ptr)

#ifndef CP_DefineSpaceStructGetter
#macro CP_DefineSpaceStructGetter( _type, _member, _name )
function cpSpaceGet##_name( byval s as const cpSpace ptr ) as _type
	return s->_member
end function
#endmacro
#endif

#ifndef CP_DefineSpaceStructSetter
#macro CP_DefineSpaceStructSetter( _type, _member, _name )
sub cpSpaceSet##_name( byval s as cpSpace ptr, byval value as _type )
	s->_member = value
end sub
#endmacro
#endif

#ifndef CP_DefineSpaceStructProperty
#macro CP_DefineSpaceStructProperty( _type, _member, _name )
CP_DefineSpaceStructGetter( _type, _member, _name )
CP_DefineSpaceStructSetter( _type, _member, _name )
#endmacro
#endif

CP_DefineSpaceStructProperty(integer, iterations, Iterations)
CP_DefineSpaceStructProperty(cpVect, gravity, Gravity)
CP_DefineSpaceStructProperty(cpFloat, damping, Damping)
CP_DefineSpaceStructProperty(cpFloat, idleSpeedThreshold, IdleSpeedThreshold)
CP_DefineSpaceStructProperty(cpFloat, sleepTimeThreshold, SleepTimeThreshold)
CP_DefineSpaceStructProperty(cpFloat, collisionSlop, CollisionSlop)
CP_DefineSpaceStructProperty(cpFloat, collisionBias, CollisionBias)
CP_DefineSpaceStructProperty(cpTimestamp, collisionPersistence, CollisionPersistence)
CP_DefineSpaceStructProperty(cpBool, enableContactGraph, EnableContactGraph)
CP_DefineSpaceStructProperty(cpDataPointer, data, UserData_) '' FIXME: If I remove the trailing _ from "UserData_", fbc complains about a duplicate definition.  This seems to stem from the Getter macro on lines 194-200
CP_DefineSpaceStructGetter(cpBody_ ptr, staticBody, StaticBody)
CP_DefineSpaceStructGetter(cpFloat, CP_PRIVATE(curr_dt), CurrentTimeStep)



''/ returns true from inside a callback and objects cannot be added/removed.
#ifndef cpSpaceIsLocked
#define cpSpaceIsLocked( _space )	( _space->CP_PRIVATE(locked) )
#endif

''/ Set a default collision handler for this space.
''/ The default collision handler is invoked for each colliding pair of shapes
''/ that isn't explicitly handled by a specific collision handler.
''/ You can pass NULL for any function you don't want to implement.
declare sub cpSpaceSetDefaultCollisionHandler( _
	byval as cpSpace ptr, _
	byval as cpCollisionBeginFunc, _
	byval as cpCollisionPreSolveFunc, _
	byval as cpCollisionPostSolveFunc, _
	byval as cpCollisionSeparateFunc, _
	byval as any ptr _
)


''/ Set a collision handler to be used whenever the two shapes with the given collision types collide.
''/ You can pass NULL for any function you don't want to implement.
declare sub cpSpaceAddCollisionHandler( _
	byval space as cpSpace ptr, _
	byval a as cpCollisionType, byval b as cpCollisionType, _
	byval begin as cpCollisionBeginFunc, _
	byval preSolve as cpCollisionPreSolveFunc, _
	byval postSolve as cpCollisionPostSolveFunc, _
	byval separate as cpCollisionSeparateFunc, _
	byval data as any ptr _
)



''/ Unset a collision handler.

declare sub cpSpaceRemoveCollisionHandler(byval space as cpSpace ptr, byval a as cpCollisionType, byval b as cpCollisionType)



''/ Add a collision shape to the simulation.

''/ If the shape is attached to a static body, it will be added as a static shape.

declare function cpSpaceAddShape(byval space as cpSpace ptr, byval shape as cpShape ptr) as cpShape ptr

''/ Explicity add a shape as a static shape to the simulation.

declare function cpSpaceAddStaticShape(byval space as cpSpace ptr, byval shape as cpShape ptr) as cpShape ptr

''/ Add a rigid body to the simulation.

declare function cpSpaceAddBody(byval space as cpSpace ptr, byval body as cpBody ptr) as cpBody ptr

''/ Add a constraint to the simulation.

declare function cpSpaceAddConstraint(byval space as cpSpace ptr, byval constraint as cpConstraint ptr) as cpConstraint ptr



''/ Remove a collision shape from the simulation.

declare sub cpSpaceRemoveShape(byval space as cpSpace ptr, byval shape as cpShape ptr)

''/ Remove a collision shape added using cpSpaceAddStaticShape() from the simulation.

declare sub cpSpaceRemoveStaticShape(byval space as cpSpace ptr, byval shape as cpShape ptr)

''/ Remove a rigid body from the simulation.

declare sub cpSpaceRemoveBody(byval space as cpSpace ptr, byval body as cpBody ptr)

''/ Remove a constraint from the simulation.

declare sub cpSpaceRemoveConstraint(byval space as cpSpace ptr, byval constraint as cpConstraint ptr)



''/ Test if a collision shape has been added to the space.

declare function cpSpaceContainsShape(byval space as cpSpace ptr, byval shape as cpShape ptr) as cpBool

''/ Test if a rigid body has been added to the space.

declare function cpSpaceContainsBody(byval space as cpSpace ptr, byval body as cpBody ptr) as cpBool

''/ Test if a constraint has been added to the space.

declare function cpSpaceContainsConstraint(byval space as cpSpace ptr, byval constraint as cpConstraint ptr) as cpBool



''/ Post Step callback function type.

type cpPostStepFunc as sub(byval space as cpSpace ptr, byval key as any ptr, byval data as any ptr)

''/ Schedule a post-step callback to be called when cpSpaceStep() finishes.

''/ You can only register one callback per unique value for @c key.

''/ Returns true only if @c key has never been scheduled before.

''/ It's possible to pass @c NULL for @c func if you only want to mark @c key as being used.

declare function cpSpaceAddPostStepCallback(byval space as cpSpace ptr, byval func as cpPostStepFunc, byval key as any ptr, byval data as any ptr) as cpBool



''/ Point query callback function type.

type cpSpacePointQueryFunc as sub(byval shape as cpShape ptr, byval data as any ptr)

''/ Query the space at a point and call @c func for each shape found.

declare sub cpSpacePointQuery(byval space as cpSpace ptr, byval point as cpVect, byval layers as cpLayers, byval group as cpGroup, byval func as cpSpacePointQueryFunc, byval data as any ptr)

''/ Query the space at a point and return the first shape found. Returns NULL if no shapes were found.

declare function cpSpacePointQueryFirst(byval space as cpSpace ptr, byval point as cpVect, byval layers as cpLayers, byval group as cpGroup) as cpShape ptr



''/ Nearest point query callback function type.

type cpSpaceNearestPointQueryFunc as sub(byval shape as cpShape ptr, byval distance as cpFloat, byval point as cpVect, byval data as any ptr)

''/ Query the space at a point and call @c func for each shape found.

declare sub cpSpaceNearestPointQuery(byval space as cpSpace ptr, byval point as cpVect, byval maxDistance as cpFloat, byval layers as cpLayers, byval group as cpGroup, byval func as cpSpaceNearestPointQueryFunc, byval data as any ptr)

''/ Query the space at a point and return the nearest shape found. Returns NULL if no shapes were found.

declare function cpSpaceNearestPointQueryNearest(byval space as cpSpace ptr, byval point as cpVect, byval maxDistance as cpFloat, byval layers as cpLayers, byval group as cpGroup, byval out as cpNearestPointQueryInfo ptr) as cpShape ptr



''/ Segment query callback function type.

type cpSpaceSegmentQueryFunc as sub(byval shape as cpShape ptr, byval t as cpFloat, byval n as cpVect, byval data as any ptr)

''/ Perform a directed line segment query (like a raycast) against the space calling @c func for each shape intersected.

declare sub cpSpaceSegmentQuery(byval space as cpSpace ptr, byval start as cpVect, byval end as cpVect, byval layers as cpLayers, byval group as cpGroup, byval func as cpSpaceSegmentQueryFunc, byval data as any ptr)

''/ Perform a directed line segment query (like a raycast) against the space and return the first shape hit. Returns NULL if no shapes were hit.

declare function cpSpaceSegmentQueryFirst(byval space as cpSpace ptr, byval start as cpVect, byval end as cpVect, byval layers as cpLayers, byval group as cpGroup, byval out as cpSegmentQueryInfo ptr) as cpShape ptr



''/ Rectangle Query callback function type.

type cpSpaceBBQueryFunc as sub(byval shape as cpShape ptr, byval data as any ptr)

''/ Perform a fast rectangle query on the space calling @c func for each shape found.

''/ Only the shape's bounding boxes are checked for overlap, not their full shape.

declare sub cpSpaceBBQuery(byval space as cpSpace ptr, byval bb as cpBB, byval layers as cpLayers, byval group as cpGroup, byval func as cpSpaceBBQueryFunc, byval data as any ptr)



''/ Shape query callback function type.

type cpSpaceShapeQueryFunc as sub(byval shape as cpShape ptr, byval points as cpContactPointSet ptr, byval data as any ptr)

''/ Query a space for any shapes overlapping the given shape and call @c func for each shape found.

declare function cpSpaceShapeQuery(byval space as cpSpace ptr, byval shape as cpShape ptr, byval func as cpSpaceShapeQueryFunc, byval data as any ptr) as cpBool



''/ Call cpBodyActivate() for any shape that is overlaps the given shape.

declare sub cpSpaceActivateShapesTouchingShape(byval space as cpSpace ptr, byval shape as cpShape ptr)





''/ Space/body iterator callback function type.

type cpSpaceBodyIteratorFunc as sub(byval body as cpBody ptr, byval data as any ptr)

''/ Call @c func for each body in the space.

declare sub cpSpaceEachBody(byval space as cpSpace ptr, byval func as cpSpaceBodyIteratorFunc, byval data as any ptr)



''/ Space/body iterator callback function type.

type cpSpaceShapeIteratorFunc as sub(byval shape as cpShape ptr, byval data as any ptr)

''/ Call @c func for each shape in the space.

declare sub cpSpaceEachShape(byval space as cpSpace ptr, byval func as cpSpaceShapeIteratorFunc, byval data as any ptr)



''/ Space/constraint iterator callback function type.

type cpSpaceConstraintIteratorFunc as sub(byval constraint as cpConstraint ptr, byval data as any ptr)

''/ Call @c func for each shape in the space.

declare sub cpSpaceEachConstraint(byval space as cpSpace ptr, byval func as cpSpaceConstraintIteratorFunc, byval data as any ptr)



''/ Update the collision detection info for the static shapes in the space.

declare sub cpSpaceReindexStatic(byval space as cpSpace ptr)

''/ Update the collision detection data for a specific shape in the space.

declare sub cpSpaceReindexShape(byval space as cpSpace ptr, byval shape as cpShape ptr)

''/ Update the collision detection data for all shapes attached to a body.

declare sub cpSpaceReindexShapesForBody(byval space as cpSpace ptr, byval body as cpBody ptr)



''/ Switch the space to use a spatial has as it's spatial index.

declare sub cpSpaceUseSpatialHash(byval space as cpSpace ptr, byval dim as cpFloat, byval count as integer)



''/ Step the space forward in time by @c dt.

declare sub cpSpaceStep(byval space as cpSpace ptr, byval dt as cpFloat)



''/ @}

end extern

#endif