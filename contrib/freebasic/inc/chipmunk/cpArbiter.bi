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


#ifndef CHIPMUNK_ARBITER
#define CHIPMUNK_ARBITER
 
extern "c"
 
''/ @defgroup cpArbiter cpArbiter
''/ The cpArbiter struct controls pairs of colliding shapes.
''/ They are also used in conjuction with collision handler callbacks
''/ allowing you to retrieve information on the collision and control it.
''/ Collision begin event function callback type.
''/ Returning false from a begin callback causes the collision to be ignored until
''/ the the separate callback is called when the objects stop colliding.

type as function(byval arb as cpArbiter_ ptr, byval space as cpSpace_ ptr, byval data as any ptr) as cpBool cpCollisionBeginFunc

''/ Collision pre-solve event function callback type.
''/ Returning false from a pre-step callback causes the collision to be ignored until the next step.

type as function(byval arb as cpArbiter_ ptr, byval space as cpSpace_ ptr, byval data as any ptr) as cpBool cpCollisionPreSolveFunc

''/ Collision post-solve event function callback type.

type as sub(byval arb as cpArbiter_ ptr, byval space as cpSpace_ ptr, byval data as any ptr) cpCollisionPostSolveFunc

''/ Collision separate event function callback type.

type as sub(byval arb as cpArbiter_ ptr, byval space as cpSpace_ ptr, byval data as any ptr) cpCollisionSeparateFunc



''/ @private

type cpCollisionHandler

	as cpCollisionType a

	as cpCollisionType b

	as cpCollisionBeginFunc begin

	as cpCollisionPreSolveFunc preSolve

	as cpCollisionPostSolveFunc postSolve

	as cpCollisionSeparateFunc separate

	as any ptr data

end type



type as cpContact cpContact



#define CP_MAX_CONTACTS_PER_ARBITER 4



''/ @private

enum cpArbiterState

	'' Arbiter is active and its the first collision.

	cpArbiterStateFirstColl

	'' Arbiter is active and its not the first collision.

	cpArbiterStateNormal

	'' Collision has been explicitly ignored.

	'' Either by returning false from a begin collision handler or calling cpArbiterIgnore().

	cpArbiterStateIgnore

	'' Collison is no longer active. A space will cache an arbiter for up to cpSpace.collisionPersistence more steps.

	cpArbiterStateCached

end enum



''/ @private

type cpArbiterThread

	'' Links to next and previous arbiters in the contact graph.

	as cpArbiter_ ptr next, prev

end type



''/ A colliding pair of shapes.

type cpArbiter

	''/ Calculated value to use for the elasticity coefficient.

	''/ Override in a pre-solve collision handler for custom behavior.

	as cpFloat e

	''/ Calculated value to use for the friction coefficient.

	''/ Override in a pre-solve collision handler for custom behavior.

	as cpFloat u

	 ''/ Calculated value to use for applying surface velocities.

	''/ Override in a pre-solve collision handler for custom behavior.

	as cpVect surface_vr

	

	''/ User definable data pointer.

	''/ The value will persist for the pair of shapes until the separate() callback is called.

	''/ NOTE: If you need to clean up this pointer, you should implement the separate() callback to do it.

	as cpDataPointer data

	

	CP_PRIVATE(as cpShape_ ptr	a)
	CP_PRIVATE(as cpShape_ ptr	b)
	CP_PRIVATE(as cpBody_ ptr	body_a)
	CP_PRIVATE(as cpBody_ ptr	body_b)
	CP_PRIVATE(as cpArbiterThread thread_a)
	CP_PRIVATE(as cpArbiterThread thread_b)
	CP_PRIVATE(as integer 		numContacts)
	CP_PRIVATE(as cpContact ptr	contacts)
	CP_PRIVATE(as cpTimestamp	stamp)
	CP_PRIVATE(as cpCollisionHandler ptr	handler)
	CP_PRIVATE(as cpBool		swappedColl)
	CP_PRIVATE(as cpArbiterState	state)

end type

#ifndef CP_DefineArbiterStructGetter
#macro CP_DefineArbiterStructGetter( _type, _member, _name)
function cpArbiterGet##_name ( byval arb as const cpArbiter ptr) as _type
	return arb->_member
end function
#endmacro
#endif

#ifndef CP_DefineArbiterStructSetter
#macro CP_DefineArbiterStructSetter(_type, _member, _name)
sub cpArbiterSet##_name( byval arb as cpArbiter ptr, byval value as _type )
	arb->_member = value
end sub
#endmacro
#endif

#ifndef CP_DefineArbiterStructProperty
#macro CP_DefineArbiterStructProperty(_type, _member, _name)
CP_DefineArbiterStructGetter(_type, _member, _name)
CP_DefineArbiterStructSetter(_type, _member, _name)
#endmacro
#endif


CP_DefineArbiterStructProperty(cpFloat, e, Elasticity)
CP_DefineArbiterStructProperty(cpFloat, u, Friction)
CP_DefineArbiterStructProperty(cpVect, surface_vr, SurfaceVelocity)
CP_DefineArbiterStructProperty(cpDataPointer, data, UserData)


''/ Calculate the total impulse that was applied by this arbiter.
''/ This function should only be called from a post-solve, post-step or cpBodyEachArbiter callback.
declare function cpArbiterTotalImpulse( byval arb as const cpArbiter ptr ) as cpVect

''/ Calculate the total impulse including the friction that was applied by this arbiter.
''/ This function should only be called from a post-solve, post-step or cpBodyEachArbiter callback.
declare function cpArbiterTotalImpulseWithFriction(byval arb as const cpArbiter ptr) as cpVect

''/ Calculate the amount of energy lost in a collision including static, but not dynamic friction.
''/ This function should only be called from a post-solve, post-step or cpBodyEachArbiter callback.
declare function cpArbiterTotalKE(byval arb as const cpArbiter ptr) as cpFloat

''/ Causes a collision pair to be ignored as if you returned false from a begin callback.
''/ If called from a pre-step callback, you will still need to return false
''/ if you want it to be ignored in the current step.
declare sub cpArbiterIgnore(byval arb as cpArbiter ptr)


''/ Return the colliding shapes involved for this arbiter.
''/ The order of their cpSpace.collision_type values will match
''/ the order set when the collision handler was registered.
#ifndef cpArbiterGetShapes
#macro cpArbiterGetShapes( arb, a, b )
if (arb->CP_PRIVATE(swappedColl)) then
	(*a) = arb->CP_PRIVATE(b)
	(*b) = arb->CP_PRIVATE(a)
else
	(*a) = arb->CP_PRIVATE(a)
	(*b) = arb->CP_PRIVATE(b)
end if
#endmacro
#endif

''/ A macro shortcut for defining and retrieving the shapes from an arbiter.

#ifndef CP_ARBITER_GET_SHAPES
#define CP_ARBITER_GET_SHAPES(arb, a, b) dim as cpShape ptr a : dim as cpShape ptr b : cpArbiterGetShapes(arb, @a, @b);
#endif

''/ Return the colliding bodies involved for this arbiter.
''/ The order of the cpSpace.collision_type the bodies are associated with values will match
''/ the order set when the collision handler was registered.
#ifndef cpArbiterGetBodies
#define cpArbiterGetBodies( arb, a, b )	CP_ARBITER_GET_SHAPES( arb, shape_a, shape_b ) : (*a) = shape_a->body : (*b) = shape_b->body
#endif

''/ A macro shortcut for defining and retrieving the bodies from an arbiter.
#ifndef CP_ARBITER_GET_BODIES
#define CP_ARBITER_GET_BODIES( arb, a, b )	dim as cpBody ptr a, b : cpArbiterGetBodies( arb, @a, @b )
#endif


''/ A struct that wraps up the important collision data for an arbiter.
type _cpContactPoint
	as cpVect	point
	as cpVect	normal
	as cpFloat	dist
end type

type cpContactPointSet
	as integer			count
	as _cpContactPoint	points(0 to CP_MAX_CONTACTS_PER_ARBITER-1)
end type

''/ Return a contact set from an arbiter.
declare function cpArbiterGetContactPointSet(byval arb as const cpArbiter ptr) as cpContactPointSet


''/ Returns true if this is the first step a pair of objects started colliding.
declare function cpArbiterIsFirstContact(byval arb as const cpArbiter ptr) as cpBool
''/ Get the number of contact points for this arbiter.
declare function cpArbiterGetCount(byval arb as const cpArbiter ptr) as integer
''/ Get the normal of the @c ith contact point.
declare function cpArbiterGetNormal(byval arb as const cpArbiter ptr, byval i as integer) as cpVect
''/ Get the position of the @c ith contact point.
declare function cpArbiterGetPoint(byval arb as const cpArbiter ptr, byval i as integer) as cpVect
''/ Get the depth of the @c ith contact point.
declare function cpArbiterGetDepth(byval arb as const cpArbiter ptr, byval i as integer) as cpFloat

end extern

#endif