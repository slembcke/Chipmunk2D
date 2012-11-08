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

#ifndef CHIPMUNK_BODY
#define CHIPMUNK_BODY	1

extern "c"

''/ @defgroup cpBody cpBody

''/ Chipmunk's rigid body type. Rigid bodies hold the physical properties of an object like

''/ it's mass, and position and velocity of it's center of gravity. They don't have an shape on their own.

''/ They are given a shape by creating collision shapes (cpShape) that point to the body.

''/ @{



''/ Rigid body velocity update function type.

type as sub(byval body as cpBody ptr, byval gravity as cpVect, byval damping as cpFloat, byval dt as cpFloat) cpBodyVelocityFunc

''/ Rigid body position update function type.

type as sub(byval body as cpBody ptr, byval dt as cpFloat) cpBodyPositionFunc



''/ Used internally to track information on the collision graph.

''/ @private

type cpComponentNode

	as cpBody ptr root

	as cpBody ptr next

	as cpFloat idleTime

end type



''/ Chipmunk's rigid body struct.

type cpBody

	''/ Function that is called to integrate the body's velocity. (Defaults to cpBodyUpdateVelocity)

	as cpBodyVelocityFunc velocity_func

	

	''/ Function that is called to integrate the body's position. (Defaults to cpBodyUpdatePosition)

	as cpBodyPositionFunc position_func

	

	''/ Mass of the body.

	''/ Must agree with cpBody.m_inv! Use cpBodySetMass() when changing the mass for this reason.

	as cpFloat m

	''/ Mass inverse.

	as cpFloat m_inv

	

	''/ Moment of inertia of the body.

	''/ Must agree with cpBody.i_inv! Use cpBodySetMoment() when changing the moment for this reason.

	as cpFloat i

	''/ Moment of inertia inverse.

	as cpFloat i_inv

	

	''/ Position of the rigid body's center of gravity.

	as cpVect p

	''/ Velocity of the rigid body's center of gravity.

	as cpVect v

	''/ Force acting on the rigid body's center of gravity.

	as cpVect f

	

	''/ Rotation of the body around it's center of gravity in radians.

	''/ Must agree with cpBody.rot! Use cpBodySetAngle() when changing the angle for this reason.

	as cpFloat a

	''/ Angular velocity of the body around it's center of gravity in radians/second.

	as cpFloat w

	''/ Torque applied to the body around it's center of gravity.

	as cpFloat t

	

	''/ Cached unit length vector representing the angle of the body.

	''/ Used for fast rotations using cpvrotate().

	as cpVect rot

	

	''/ User definable data pointer.

	''/ Generally this points to your the game object class so you can access it

	''/ when given a cpBody reference in a callback.

	as cpDataPointer data

	

	''/ Maximum velocity allowed when updating the velocity.

	as cpFloat v_limit

	''/ Maximum rotational rate (in radians/second) allowed when updating the angular velocity.

	as cpFloat w_limit

	

	'' TODO: translate (sorry)
	CP_PRIVATE(as cpVect v_bias);

	'' TODO: translate (sorry)
	CP_PRIVATE(as cpFloat w_bias);

	

	'' TODO: translate (sorry)
	CP_PRIVATE(as cpSpace ptr space);

	

	'' TODO: translate (sorry)
	CP_PRIVATE(as cpShape ptr shapeList);

	'' TODO: translate (sorry)
	CP_PRIVATE(as cpArbiter ptr arbiterList);

	'' TODO: translate (sorry)
	CP_PRIVATE(as cpConstraint ptr constraintList);

	

	'' TODO: translate (sorry)
	CP_PRIVATE(as cpComponentNode node);

end type



''/ Allocate a cpBody.

declare function cpBodyAlloc() as cpBody ptr

''/ Initialize a cpBody.

declare function cpBodyInit(byval body as cpBody ptr, byval m as cpFloat, byval i as cpFloat) as cpBody ptr

''/ Allocate and initialize a cpBody.

declare function cpBodyNew(byval m as cpFloat, byval i as cpFloat) as cpBody ptr



''/ Initialize a static cpBody.

declare function cpBodyInitStatic(byval body as cpBody ptr) as cpBody ptr

''/ Allocate and initialize a static cpBody.

declare function cpBodyNewStatic() as cpBody ptr



''/ Destroy a cpBody.

declare sub cpBodyDestroy(byval body as cpBody ptr)

''/ Destroy and free a cpBody.

declare sub cpBodyFree(byval body as cpBody ptr)



''/ Check that the properties of a body is sane. (Only in debug mode)

#ifdef NDEBUG

	#define	cpBodyAssertSane(body)

#else

	declare sub cpBodySanityCheck(byval body as cpBody ptr)

	#define	cpBodyAssertSane(body) cpBodySanityCheck(body)

#endif



'' Defined in cpSpace.c

''/ Wake up a sleeping or idle body.

declare sub cpBodyActivate(byval body as cpBody ptr)

''/ Wake up any sleeping or idle bodies touching a static body.

declare sub cpBodyActivateStatic(byval body as cpBody ptr, byval filter as cpShape ptr)



''/ Force a body to fall asleep immediately.

declare sub cpBodySleep(byval body as cpBody ptr)

''/ Force a body to fall asleep immediately along with other bodies in a group.

declare sub cpBodySleepWithGroup(byval body as cpBody ptr, byval group as cpBody ptr)



''/ Returns true if the body is sleeping.

'' TODO: translate (sorry)
#ifndef cpBodyIsSleeping
#define cpBodyIsSleeping( body )	iif( CP_PRIVATE(body->node).root <> cptr(cpBody ptr, NULL), cpTrue, cpFalse )
#endif


''/ Returns true if the body is static.
#ifndef cpBodyIsState
#define cpBodyIsStatic( body )	iif( CP_PRIVATE(body->node).idleTime = INFINITY, cpTrue, cpFalse )
#endif


''/ Returns true if the body has not been added to a space.
#ifndef cpBodyIsRogue
#define cpBodyIsRogue( body )	iif( body->CP_PRIVATE(space) = cptr(cpSpace ptr, NULL), cpTrue, cpFalse )
#endif




#ifndef CP_DefineBodyStructGetter
#macro CP_DefineBodyStructGetter( _type, _member, _name )
function cpBodyGet##_name( byval body as const cpBody ptr ) as _type
	return body->member
end function
#endmacro
#endif

#ifndef CP_DefineBodyStructSetter
#macro CP_DefineBodyStructSetter( _type, _member, _name )
sub cpBodySet##_name( byval body as cpBody ptr, byval value as const _type )
	cpBodyActivate( body )
	body->(member) = value
	cpBodyAssertSane(body)
end sub
#endmacro
#endif

#ifndef CP_DefineBodyStructProperty
#macro CP_DefineBodyStructProperty( _type, _member, _name )
CP_DefineBodyStructGetter( _type, _member, _name )
CP_DefineBodyStructSetter( _type, _member, _name )
#endmacro
#endif

CP_DefineBodyStructGetter(cpSpace ptr, CP_PRIVATE(space), Space)

CP_DefineBodyStructGetter(cpFloat, m, Mass)

''/ Set the mass of a body.
declare sub cpBodySetMass(byval body as cpBody ptr, byval m as cpFloat)

CP_DefineBodyStructGetter(cpFloat, i, Moment)

''/ Set the moment of a body.
declare sub cpBodySetMoment( byval body as cpBody ptr, byval i as cpFloat )


CP_DefineBodyStructGetter(cpVect, p, Pos)

''/ Set the position of a body.
declare sub cpBodySetPos( byval body as cpBody ptr, byval pos as cpVect )

CP_DefineBodyStructProperty(cpVect, v, Vel)
CP_DefineBodyStructProperty(cpVect, f, Force)
CP_DefineBodyStructGetter(cpFloat, a, Angle)

''/ Set the angle of a body.
declare sub cpBodySetAngle( byval body as cpBody ptr, byval a as cpFloat )void cpBodySetAngle(cpBody *body, cpFloat a);

CP_DefineBodyStructProperty(cpFloat, w, AngVel)
CP_DefineBodyStructProperty(cpFloat, t, Torque)
CP_DefineBodyStructGetter(cpVect, rot, Rot)
CP_DefineBodyStructProperty(cpFloat, v_limit, VelLimit)
CP_DefineBodyStructProperty(cpFloat, w_limit, AngVelLimit)
CP_DefineBodyStructProperty(cpDataPointer, data, UserData)



''/ Default Integration functions.
declare sub cpBodyUpdateVelocity( byval body as cpBody ptr, byval gravity as cpVect, byval damping as cpFloat, byval dt as cpFloat )
declare sub cpBodyUpdatePosition(byval body as cpBody ptr, byval dt as cpFloat)

''/ Convert body relative/local coordinates to absolute/world coordinates.
#ifndef cpBodyLocal2World
#define cpBodyLocal2World( body, v )	cpvadd( body->p, cpvrotate( v, body->rot ) )
#endif


''/ Convert body absolute/world coordinates to  relative/local coordinates.
#ifndef cpBodyWorld2Local
#define cpBodyWorld2Local( body, v )	cpvunrotate( cpvsub( v, body->p ), body->rot )
#endif


''/ Set the forces and torque or a body to zero.
declare sub cpBodyResetForces( byval body as cpBody ptr )

''/ Apply an force (in world coordinates) to the body at a point relative to the center of gravity (also in world coordinates).
declare sub cpBodyApplyForce(byval body as cpBody ptr, byval f as const cpVect, byval r as const cpVect)

''/ Apply an impulse (in world coordinates) to the body at a point relative to the center of gravity (also in world coordinates).
declare sub cpBodyApplyImpulse(byval body as cpBody ptr, byval j as const cpVect, byval r as const cpVect)

''/ Get the velocity on a body (in world units) at a point on the body in world coordinates.
declare function cpBodyGetVelAtWorldPoint(byval body as cpBody ptr, byval point as cpVect) as cpVect
''/ Get the velocity on a body (in world units) at a point on the body in local coordinates.
declare function cpBodyGetVelAtLocalPoint(byval body as cpBody ptr, byval point as cpVect) as cpVect





''/ Get the kinetic energy of a body.
#ifndef cpBodyKineticEnergy
#define cpBodyKineticEnergy( body )	(iif( cpvdot( body->v, body->v ), cpvdot( body->v, body->v ) * body->m, 0.0) + iif( body->w*body->w, body->w*body->w * body->i, 0.0 ))
#endif

''/ Body/shape iterator callback function type. 

type cpBodyShapeIteratorFunc as sub cdecl( byval body as cpBody ptr, byval shape as cpShape ptr, byval _data as any ptr )

''/ Call @c func once for each shape attached to @c body and added to the space.
declare sub cpBodyEachShape(byval body as cpBody ptr, byval func as cpBodyShapeIteratorFunc, byval data as any ptr)

''/ Body/constraint iterator callback function type. 

type cpBodyConstraintIteratorFunc as sub(byval body as cpBody ptr, byval constraint as cpConstraint ptr, byval data as any ptr)

''/ Call @c func once for each constraint attached to @c body and added to the space.

declare sub cpBodyEachConstraint(byval body as cpBody ptr, byval func as cpBodyConstraintIteratorFunc, byval data as any ptr)



''/ Body/arbiter iterator callback function type. 

type as sub(byval body as cpBody ptr, byval arbiter as cpArbiter ptr, byval data as any ptr) cpBodyArbiterIteratorFunc

''/ Call @c func once for each arbiter that is currently active on the body.

declare sub cpBodyEachArbiter(byval body as cpBody ptr, byval func as cpBodyArbiterIteratorFunc, byval data as any ptr)

end extern

#endif