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

#ifndef CHIPMUNK_SHAPE 
#define CHIPMUNK_SHAPE

extern "c"

''/ @defgroup cpShape cpShape

''/ The cpShape struct defines the shape of a rigid body.

''/ @{



type as cpShapeClass cpShapeClass_



''/ Nearest point query info struct.

type cpNearestPointQueryInfo

	''/ The nearest shape, NULL if no shape was within range.

	as cpShape_ ptr shape

	''/ The closest point on the shape's surface. (in world space coordinates)

	as cpVect p

	''/ The distance to the point. The distance is negative if the point is inside the shape.

	as cpFloat d

end type



''/ Segment query info struct.

type cpSegmentQueryInfo

	''/ The shape that was hit, NULL if no collision occured.

	as cpShape_ ptr shape

	''/ The normalized distance along the query segment in the range [0, 1].

	as cpFloat t

	''/ The normal of the surface hit.

	as cpVect n

end type



''/ @private

enum cpShapeType

	CP_CIRCLE_SHAPE

	CP_SEGMENT_SHAPE

	CP_POLY_SHAPE

	CP_NUM_SHAPES

end enum



type cpShapeCacheDataImpl as function(byval shape as cpShape_ ptr, byval p as cpVect, byval rot as cpVect) as cpBB

type cpShapeDestroyImpl as sub(byval shape as cpShape_ ptr)

type cpShapeNearestPointQueryImpl as sub(byval shape as cpShape_ ptr, byval p as cpVect, byval info as cpNearestPointQueryInfo ptr)

type cpShapeSegmentQueryImpl as sub(byval shape as cpShape_ ptr, byval a as cpVect, byval b as cpVect, byval info as cpSegmentQueryInfo ptr)



''/ @private

type cpShapeClass

	as cpShapeType type

	

	as cpShapeCacheDataImpl cacheData

	as cpShapeDestroyImpl destroy

	as cpShapeNearestPointQueryImpl nearestPointQuery

	as cpShapeSegmentQueryImpl segmentQuery

end type



''/ Opaque collision shape struct.

type cpShape

	CP_PRIVATE(as const cpShapeClass ptr klass)

	

	''/ The rigid body this collision shape is attached to.

	as cpBody ptr body



	''/ The current bounding box of the shape.

	as cpBB bb

	

	''/ Sensor flag.

	''/ Sensor shapes call collision callbacks but don't produce collisions.

	as cpBool sensor

	

	''/ Coefficient of restitution. (elasticity)

	as cpFloat e

	''/ Coefficient of friction.

	as cpFloat u

	''/ Surface velocity used when solving for friction.

	as cpVect surface_v



	''/ User definable data pointer.

	''/ Generally this points to your the game object class so you can access it

	''/ when given a cpShape reference in a callback.

	as cpDataPointer data

	

	''/ Collision type of this shape used when picking collision handlers.

	as cpCollisionType collision_type

	''/ Group of this shape. Shapes in the same group don't collide.

	as cpGroup group

	'' Layer bitmask for this shape. Shapes only collide if the bitwise and of their layers is non-zero.

	as cpLayers layers

	

	CP_PRIVATE(as cpSpace_ ptr space)

	

	CP_PRIVATE(as cpShape_ ptr next)

	CP_PRIVATE(as cpShape_ ptr prev)

	

	CP_PRIVATE(as cpHashValue hashid)

end type



''/ Destroy a shape.

declare sub cpShapeDestroy(byval shape as cpShape ptr)

''/ Destroy and Free a shape.

declare sub cpShapeFree(byval shape as cpShape ptr)



''/ Update, cache and return the bounding box of a shape based on the body it's attached to.

declare function cpShapeCacheBB(byval shape as cpShape ptr) as cpBB

''/ Update, cache and return the bounding box of a shape with an explicit transformation.

declare function cpShapeUpdate(byval shape as cpShape ptr, byval pos as cpVect, byval rot as cpVect) as cpBB



''/ Test if a point lies within a shape.

declare function cpShapePointQuery(byval shape as cpShape ptr, byval p as cpVect) as cpBool



''/ Perform a nearest point query. It finds the closest point on the surface of shape to a specific point.
''/ The value returned is the distance between the points. A negative distance means the point is inside the shape.
declare function cpShapeNearestPointQuery(byval shape as cpShape ptr, byval p as cpVect, byval out as cpNearestPointQueryInfo ptr) as cpFloat



''/ Perform a segment query against a shape. @c info must be a pointer to a valid cpSegmentQueryInfo structure.
declare function cpShapeSegmentQuery(byval shape as cpShape ptr, byval a as cpVect, byval b as cpVect, byval info as cpSegmentQueryInfo ptr) as cpBool


''/ Get the hit point for a segment query.
#ifndef cpSegmentQueryHitPoint
#define cpSegmentQueryHitPoint( _start, _end, _info )	cpvlerp( _start, _end, _info.t )
#endif

''/ Get the hit distance for a segment query.
#ifndef cpSegmentQueryHitDist
#define cpSegmentQueryHitDist( _start, _end, _info )	( cpvdist( _start, _send ) * _info.t )
#endif


#ifndef CP_DefineShapeStructGetter
#macro CP_DefineShapeStructGetter( _type, _member, _name )
function cpSpaceGet##_name( byval shape as const cpShape ptr ) as _type
	return shape->_member
end function
#endmacro
#endif

#ifndef CP_DefineShapeStructSetter
#macro CP_DefineShapeStructSetter( _type, _member, _name, _activates )
sub cpShapeSet##_name( byval shape as cpShape ptr, byval value as _type )
	if ( (_activates = cpTrue) and (shape->body <> NULL) ) then cpBodyActivate( shape->body )
	shape->_member = value
end sub
#endmacro
#endif

#ifndef CP_DefineShapeStructProperty
#macro CP_DefineShapeStructProperty( _type, _member, _name, _activates )
CP_DefineShapeStructGetter( _type, _member, _name )
CP_DefineShapeStructSetter( _type, _member, _name, _activates )
#endmacro
#endif


CP_DefineShapeStructGetter(cpSpace_ ptr, CP_PRIVATE(space), Space)

CP_DefineShapeStructGetter(cpBody_ ptr, body, Body)

declare sub cpShapeSetBody( byval shape as cpShape_ ptr, byval body as cpBody_ ptr )


CP_DefineShapeStructGetter(cpBB, bb, BB)

CP_DefineShapeStructProperty(cpBool, sensor, Sensor, cpTrue)

CP_DefineShapeStructProperty(cpFloat, e, Elasticity, cpFalse)

CP_DefineShapeStructProperty(cpFloat, u, Friction, cpTrue)

CP_DefineShapeStructProperty(cpVect, surface_v, SurfaceVelocity, cpTrue)

CP_DefineShapeStructProperty(cpDataPointer, data, UserData, cpFalse)

CP_DefineShapeStructProperty(cpCollisionType, collision_type, CollisionType, cpTrue)

CP_DefineShapeStructProperty(cpGroup, group, Group, cpTrue)

CP_DefineShapeStructProperty(cpLayers, layers, Layers, cpTrue)

end extern

#endif

''/ When initializing a shape, it's hash value comes from a counter.

''/ Because the hash value may affect iteration order, you can reset the shape ID counter

''/ when recreating a space. This will make the simulation be deterministic.
declare sub cpResetShapeIdCounter()

#ifndef CP_DeclareShapeGetter
#define CP_DeclareShapeGetter( _struct, _type, _name ) declare function _struct##Get##_name( byval shape as const cpShape ptr ) as _type
#endif



''/ @}

''/ @defgroup cpCircleShape cpCircleShape



''/ @private

type cpCircleShape

	as cpShape shape

	

	as cpVect c, tc

	as cpFloat r

end type



''/ Allocate a circle shape.

declare function cpCircleShapeAlloc() as cpCircleShape ptr

''/ Initialize a circle shape.

declare function cpCircleShapeInit(byval circle as cpCircleShape ptr, byval body as cpBody ptr, byval radius as cpFloat, byval offset as cpVect) as cpCircleShape ptr

''/ Allocate and initialize a circle shape.

declare function cpCircleShapeNew(byval body as cpBody ptr, byval radius as cpFloat, byval offset as cpVect) as cpShape ptr


CP_DeclareShapeGetter(cpCircleShape, cpVect, Offset)

CP_DeclareShapeGetter(cpCircleShape, cpFloat, Radius)



''/ @}

''/ @defgroup cpSegmentShape cpSegmentShape



''/ @private

type cpSegmentShape

	as cpShape shape

	

	as cpVect a, b, n

	as cpVect ta, tb, tn

	as cpFloat r

	

	as cpVect a_tangent, b_tangent

end type



''/ Allocate a segment shape.

declare function cpSegmentShapeAlloc() as cpSegmentShape ptr

''/ Initialize a segment shape.

declare function cpSegmentShapeInit(byval seg as cpSegmentShape ptr, byval body as cpBody ptr, byval a as cpVect, byval b as cpVect, byval radius as cpFloat) as cpSegmentShape ptr

''/ Allocate and initialize a segment shape.

declare function cpSegmentShapeNew(byval body as cpBody ptr, byval a as cpVect, byval b as cpVect, byval radius as cpFloat) as cpShape ptr



declare sub cpSegmentShapeSetNeighbors(byval shape as cpShape ptr, byval prev as cpVect, byval next as cpVect)


CP_DeclareShapeGetter(cpSegmentShape, cpVect, A)
CP_DeclareShapeGetter(cpSegmentShape, cpVect, B)
CP_DeclareShapeGetter(cpSegmentShape, cpVect, Normal)
CP_DeclareShapeGetter(cpSegmentShape, cpFloat, Radius)



''/ @}

