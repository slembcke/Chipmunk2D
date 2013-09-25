/* Copyright (c) 2007 Scott Lembcke
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
 */
 
/// @defgroup cpShape cpShape
/// The cpShape struct defines the shape of a rigid body.
/// @{

typedef struct cpShapeClass cpShapeClass;

/// Nearest point query info struct.
typedef struct cpPointQueryInfo {
	/// The nearest shape, NULL if no shape was within range.
	const cpShape *shape;
	/// The closest point on the shape's surface. (in world space coordinates)
	cpVect point;
	/// The distance to the point. The distance is negative if the point is inside the shape.
	cpFloat distance;
	/// The gradient of the signed distance function.
	/// The same as info.p/info.d, but accurate even for very small values of info.d.
	cpVect gradient;
} cpPointQueryInfo;

/// Segment query info struct.
typedef struct cpSegmentQueryInfo {
	/// The shape that was hit, NULL if no collision occured.
	const cpShape *shape;
	/// The point of impact.
	cpVect point;
	/// The normal of the surface hit.
	cpVect normal;
	/// The normalized distance along the query segment in the range [0, 1].
	cpFloat alpha;
} cpSegmentQueryInfo;

typedef struct cpShapeFilter {
	cpGroup group;
	cpBitmask categories;
	cpBitmask mask;
} cpShapeFilter;

static const cpShapeFilter CP_SHAPE_FILTER_ALL = {CP_NO_GROUP, CP_ALL_CATEGORIES, CP_ALL_CATEGORIES};
static const cpShapeFilter CP_SHAPE_FILTER_NONE = {CP_NO_GROUP, ~CP_ALL_CATEGORIES, ~CP_ALL_CATEGORIES};

static inline cpShapeFilter
cpShapeFilterNew(cpGroup group, cpBitmask categories, cpBitmask mask)
{
	cpShapeFilter filter = {group, categories, mask};
	return filter;
}


/// @private
struct cpShapeMassInfo {
	cpFloat m;
	cpFloat i;
	cpVect cog;
	cpFloat area;
};

/// @private
typedef enum cpShapeType{
	CP_CIRCLE_SHAPE,
	CP_SEGMENT_SHAPE,
	CP_POLY_SHAPE,
	CP_NUM_SHAPES
} cpShapeType;

typedef cpBB (*cpShapeCacheDataImpl)(cpShape *shape, cpTransform transform);
typedef void (*cpShapeDestroyImpl)(cpShape *shape);
typedef void (*cpShapePointQueryImpl)(const cpShape *shape, cpVect p, cpPointQueryInfo *info);
typedef void (*cpShapeSegmentQueryImpl)(const cpShape *shape, cpVect a, cpVect b, cpFloat radius, cpSegmentQueryInfo *info);

/// @private
struct cpShapeClass {
	cpShapeType type;
	
	cpShapeCacheDataImpl cacheData;
	cpShapeDestroyImpl destroy;
	cpShapePointQueryImpl pointQuery;
	cpShapeSegmentQueryImpl segmentQuery;
};

/// Opaque collision shape struct.
struct cpShape {
	CP_PRIVATE(const cpShapeClass *klass);
	
	/// The rigid body this collision shape is attached to.
	CP_PRIVATE(cpBody *body);
	
	// Optional mass of the shape.
	CP_PRIVATE(struct cpShapeMassInfo massInfo);

	/// The current bounding box of the shape.
	CP_PRIVATE(cpBB bb);
	
	/// Sensor flag.
	/// Sensor shapes call collision callbacks but don't produce collisions.
	CP_PRIVATE(cpBool sensor);
	
	/// Coefficient of restitution. (elasticity)
	CP_PRIVATE(cpFloat e);
	/// Coefficient of friction.
	CP_PRIVATE(cpFloat u);
	/// Surface velocity used when solving for friction.
	CP_PRIVATE(cpVect surfaceV);

	/// User definable data pointer.
	/// Generally this points to your the game object class so you can access it
	/// when given a cpShape reference in a callback.
	CP_PRIVATE(cpDataPointer userData);
	
	/// Collision type of this shape used when picking collision handlers.
	CP_PRIVATE(cpCollisionType type);
	
	CP_PRIVATE(cpShapeFilter filter);
	
	CP_PRIVATE(cpSpace *space);
	
	CP_PRIVATE(cpShape *next);
	CP_PRIVATE(cpShape *prev);
	
	CP_PRIVATE(cpHashValue hashid);
};

/// Destroy a shape.
void cpShapeDestroy(cpShape *shape);
/// Destroy and Free a shape.
void cpShapeFree(cpShape *shape);

/// Update, cache and return the bounding box of a shape based on the body it's attached to.
cpBB cpShapeCacheBB(cpShape *shape);
/// Update, cache and return the bounding box of a shape with an explicit transformation.
cpBB cpShapeUpdate(cpShape *shape, cpTransform transform);

/// Perform a nearest point query. It finds the closest point on the surface of shape to a specific point.
/// The value returned is the distance between the points. A negative distance means the point is inside the shape.
cpFloat cpShapePointQuery(const cpShape *shape, cpVect p, cpPointQueryInfo *out);

/// Perform a segment query against a shape. @c info must be a pointer to a valid cpSegmentQueryInfo structure.
cpBool cpShapeSegmentQuery(const cpShape *shape, cpVect a, cpVect b, cpFloat radius, cpSegmentQueryInfo *info);

/// Return contact information about two shapes.
cpContactPointSet cpShapesCollide(const cpShape *a, const cpShape *b);


#define CP_DefineShapeStructGetter(type, member, name) \
static inline type cpShapeGet##name(const cpShape *shape){return shape->CP_PRIVATE(member);}

#define CP_DefineShapeStructSetter(type, member, name, activates) \
static inline void cpShapeSet##name(cpShape *shape, type value){ \
	if(activates) cpBodyActivate(shape->CP_PRIVATE(body)); \
	shape->CP_PRIVATE(member) = value; \
}

#define CP_DefineShapeStructProperty(type, member, name, activates) \
CP_DefineShapeStructGetter(type, member, name) \
CP_DefineShapeStructSetter(type, member, name, activates)

CP_DefineShapeStructGetter(cpSpace*, space, Space)

CP_DefineShapeStructGetter(cpBody*, body, Body)
void cpShapeSetBody(cpShape *shape, cpBody *body);

cpFloat cpShapeGetMass(cpShape *shape);
void cpShapeSetMass(cpShape *shape, cpFloat mass);

cpFloat cpShapeGetDensity(cpShape *shape);
void cpShapeSetDensity(cpShape *shape, cpFloat density);

cpFloat cpShapeGetMoment(cpShape *shape);
cpFloat cpShapeGetArea(cpShape *shape);
cpVect cpShapeGetCenterOfGravity(cpShape *shape);

CP_DefineShapeStructGetter(cpBB, bb, BB)
CP_DefineShapeStructProperty(cpBool, sensor, Sensor, cpTrue)
CP_DefineShapeStructProperty(cpFloat, e, Elasticity, cpFalse)
CP_DefineShapeStructProperty(cpFloat, u, Friction, cpTrue)
CP_DefineShapeStructProperty(cpVect, surfaceV, SurfaceVelocity, cpTrue)
CP_DefineShapeStructProperty(cpDataPointer, userData, UserData, cpFalse)
CP_DefineShapeStructProperty(cpCollisionType, type, CollisionType, cpTrue)
CP_DefineShapeStructProperty(cpShapeFilter, filter, Filter, cpTrue)

/// When initializing a shape, it's hash value comes from a counter.
/// Because the hash value may affect iteration order, you can reset the shape ID counter
/// when recreating a space. This will make the simulation be deterministic.
void cpResetShapeIdCounter(void);

#define CP_DeclareShapeGetter(struct, type, name) type struct##Get##name(const cpShape *shape)

/// @}
/// @defgroup cpCircleShape cpCircleShape

/// @private
typedef struct cpCircleShape {
	cpShape shape;
	
	cpVect c, tc;
	cpFloat r;
} cpCircleShape;

/// Allocate a circle shape.
cpCircleShape* cpCircleShapeAlloc(void);
/// Initialize a circle shape.
cpCircleShape* cpCircleShapeInit(cpCircleShape *circle, cpBody *body, cpFloat radius, cpVect offset);
/// Allocate and initialize a circle shape.
cpShape* cpCircleShapeNew(cpBody *body, cpFloat radius, cpVect offset);

CP_DeclareShapeGetter(cpCircleShape, cpVect, Offset);
CP_DeclareShapeGetter(cpCircleShape, cpFloat, Radius);

/// @}
/// @defgroup cpSegmentShape cpSegmentShape

/// @private
typedef struct cpSegmentShape {
	cpShape shape;
	
	cpVect a, b, n;
	cpVect ta, tb, tn;
	cpFloat r;
	
	cpVect a_tangent, b_tangent;
} cpSegmentShape;

/// Allocate a segment shape.
cpSegmentShape* cpSegmentShapeAlloc(void);
/// Initialize a segment shape.
cpSegmentShape* cpSegmentShapeInit(cpSegmentShape *seg, cpBody *body, cpVect a, cpVect b, cpFloat radius);
/// Allocate and initialize a segment shape.
cpShape* cpSegmentShapeNew(cpBody *body, cpVect a, cpVect b, cpFloat radius);

/// Let Chipmunk know about the geometry of adjacent segments to avoid colliding with endcaps.
void cpSegmentShapeSetNeighbors(cpShape *shape, cpVect prev, cpVect next);

CP_DeclareShapeGetter(cpSegmentShape, cpVect, A);
CP_DeclareShapeGetter(cpSegmentShape, cpVect, B);
CP_DeclareShapeGetter(cpSegmentShape, cpVect, Normal);
CP_DeclareShapeGetter(cpSegmentShape, cpFloat, Radius);

/// @}
