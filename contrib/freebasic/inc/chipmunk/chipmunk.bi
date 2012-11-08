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



#ifndef CHIPMUNK_HEADER
#define CHIPMUNK_HEADER

#inclib "chipmunk"


#include "crt/stdlib.bi"
#include "crt/math.bi"

extern "c" 

#ifndef CP_ALLOW_PRIVATE_ACCESS
#	define CP_ALLOW_PRIVATE_ACCESS 0
#endif


#if CP_ALLOW_PRIVATE_ACCESS = 1
#	define CP_PRIVATE(symbol) symbol
#else
#	define CP_PRIVATE(symbol) symbol##_private
#endif

declare sub cpMessage(byval condition as const zstring ptr, byval file as const zstring ptr, byval line as integer, byval isError as integer, byval isHardError as integer, byval message as const byte ptr, ... )

#ifdef NDEBUG
#	define	cpAssertWarn( condition, args... )
#else
#	define cpAssertWarn( condition, args... ) if( not (condition)) then cpMessage( strptr( #condition ), __FILE__, __LINE__, 0, 0, args )
#endif



#ifdef NDEBUG
#	define	cpAssertSoft( condition, args... )
#else
#	define cpAssertSoft( condition, args... ) if( not (condition)) then cpMessage( strptr( #condition ), __FILE__, __LINE__, 1, 0, args )
#endif



'' Hard assertions are important and cheap to execute. They are not disabled by compiling as debug.
#define cpAssertHard(condition, args...) if( not (condition)) then cpMessage( strptr( #condition ), __FILE__, __LINE__, 1, 1, args )

#include "chipmunk_types.bi"

	

''/ @defgroup misc Misc

''/ @{

''/ Allocated size for various Chipmunk buffers

#ifndef CP_BUFFER_BYTES
#	define CP_BUFFER_BYTES (32*1024)
#endif

#ifndef cpcalloc
#	define cpcalloc calloc
#endif



#ifndef cprealloc
#	define cprealloc realloc
#endif

#ifndef cpfree
#	define cpfree free
#endif

type as cpArray cpArray_
type as cpHashSet cpHashSet_
type as cpBody cpBody_
type as cpShape cpShape_
type as cpConstraint cpConstraint_
type as cpCollisionHandler cpCollisionHandler_
type as cpArbiter cpArbiter_
type as cpSpace cpSpace_

#include "cpVect.bi"
#include "cpBB.bi"
#include "cpSpatialIndex.bi"
#include "cpBody.bi"
#include "cpShape.bi"
#include "cpPolyShape.bi"
#include "cpArbiter.bi"	
#include "constraints/cpConstraint.bi"
#include "cpSpace.bi"

'' Chipmunk 6.1.2
#define CP_VERSION_MAJOR 6
#define CP_VERSION_MINOR 1
#define CP_VERSION_RELEASE 2

''/ Version string.

extern as const zstring ptr cpVersionString

''/ @deprecated
declare sub cpInitChipmunk()

''/ Enables segment to segment shape collisions.
declare sub cpEnableSegmentToSegmentCollisions()

''/ Calculate the moment of inertia for a circle.
''/ @c r1 and @c r2 are the inner and outer diameters. A solid circle has an inner diameter of 0.
declare function cpMomentForCircle(byval m as cpFloat, byval r1 as cpFloat, byval r2 as cpFloat, byval offset as cpVect) as cpFloat

''/ Calculate area of a hollow circle.
''/ @c r1 and @c r2 are the inner and outer diameters. A solid circle has an inner diameter of 0.
declare function cpAreaForCircle(byval r1 as cpFloat, byval r2 as cpFloat) as cpFloat

''/ Calculate the moment of inertia for a line segment.
''/ Beveling radius is not supported.
declare function cpMomentForSegment(byval m as cpFloat, byval a as cpVect, byval b as cpVect) as cpFloat

''/ Calculate the area of a fattened (capsule shaped) line segment.
declare function cpAreaForSegment(byval a as cpVect, byval b as cpVect, byval r as cpFloat) as cpFloat

''/ Calculate the moment of inertia for a solid polygon shape assuming it's center of gravity is at it's centroid. The offset is added to each vertex.
declare function cpMomentForPoly(byval m as cpFloat, byval numVerts as integer, byval verts as const cpVect ptr, byval offset as cpVect) as cpFloat

''/ Calculate the signed area of a polygon. A Clockwise winding gives positive area.
''/ This is probably backwards from what you expect, but matches Chipmunk's the winding for poly shapes.
declare function cpAreaForPoly(byval numVerts as const integer, byval verts as const cpVect ptr) as cpFloat

''/ Calculate the natural centroid of a polygon.
declare function cpCentroidForPoly(byval numVerts as const integer, byval verts as const cpVect ptr) as cpVect

''/ Center the polygon on the origin. (Subtracts the centroid of the polygon from each vertex)
declare sub cpRecenterPoly(byval numVerts as const integer, byval verts as cpVect ptr)

''/ Calculate the moment of inertia for a solid box.
declare function cpMomentForBox(byval m as cpFloat, byval width as cpFloat, byval height as cpFloat) as cpFloat

''/ Calculate the moment of inertia for a solid box.
declare function cpMomentForBox2(byval m as cpFloat, byval box as cpBB) as cpFloat

''/ Calculate the convex hull of a given set of points. Returns the count of points in the hull.
''/ @c result must be a pointer to a @c cpVect array with at least @c count elements. If @c result is @c NULL, then @c verts will be reduced instead.
''/ @c first is an optional pointer to an integer to store where the first vertex in the hull came from (i.e. verts[first] == result[0])
''/ @c tol is the allowed amount to shrink the hull when simplifying it. A tolerance of 0.0 creates an exact hull.
declare function cpConvexHull(byval count as integer, byval verts as cpVect ptr, byval result as cpVect ptr, byval first as integer ptr, byval tol as cpFloat) as integer

''/ Convenience macro to work with cpConvexHull.
''/ @c count and @c verts is the input array passed to cpConvexHull().
''/ @c count_var and @c verts_var are the names of the variables the macro creates to store the result.
''/ The output vertex array is allocated on the stack using alloca() so it will be freed automatically, but cannot be returned from the current scope.

#macro CP_CONVEX_HULL( __count, __verts, __count_var, __verts_var )
dim as cpVect ptr __verts_var = cptr( cpVect ptr, allocate( __count * sizeof(cpVect) ) )
dim as integer __count_var = cpConvexHull( __count, __verts, __verts_var, NULL, 0.0 )
#endmacro

''static inline cpVect operator *(const cpVect v, const cpFloat s){return cpvmult(v, s);}
operator +(byval v1 as const cpVect, byval v2 as const cpVect) as cpVect
	return cpvadd(v1, v2)
end operator
''static inline cpVect operator +(const cpVect v1, const cpVect v2){return cpvadd(v1, v2);}

operator -(byval v1 as const cpVect, byval v2 as const cpVect) as cpVect
	return cpvsub(v1, v2)
end operator
''static inline cpVect operator -(const cpVect v1, const cpVect v2){return cpvsub(v1, v2);}

operator =(byval v1 as const cpVect, byval v2 as const cpVect) as cpBool
	return cpveql(v1, v2)
end operator
''static inline cpBool operator ==(const cpVect v1, const cpVect v2){return cpveql(v1, v2);}

operator -(byval v as const cpVect) as cpVect
	return cpvneg(v)
end operator
''static inline cpVect operator -(const cpVect v){return cpvneg(v);}

end extern

#endif

