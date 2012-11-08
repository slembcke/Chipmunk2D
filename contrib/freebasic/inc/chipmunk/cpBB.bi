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

#ifndef CHIPMUNK_BB
#define CHIPMUNK_BB	1

extern "c"

''/ Chipmunk's axis-aligned 2D bounding box type. (left, bottom, right, top)
type cpBB

	as cpFloat l, b, r ,t

end type : type as cpBB cpBB



''/ Convenience constructor for cpBB structs.
#ifndef cpBBNew
#define cpBBNew( l, b, r, t )	type<cpBB>( l, b, r, t )
#endif


''/ Constructs a cpBB for a circle with the given position and radius.
#ifndef cpBBNewForCircle
#define cpBBNewForCircle( p, r )	type<cpBB>( p.x - r, p.y - r, p.x + r, p.y + r )
#endif


''/ Returns true if @c a and @c b intersect.
#ifndef cpBBIntersects
#define cpBBIntersects( a, b )		iif( a.l <= b.r and b.l <= a.r and a.b <= b.t and b.b <= a.t, cpTrue, cpFalse)
#endif


''/ Returns true if @c other lies completely within @c bb.
#ifndef cpBBContainsBB
#define cpBBContainsBB( bb, other )	iif(  bb.l <= other.l and bb.r >= other.r and bb.b <= other.b and bb.t >= other.t, cpTrue, cpFalse )
#endif

''/ Returns true if @c bb contains @c v.
#ifndef cpBBContainsVect
#define cpBBContainsVect( bb, v )	iif( bb.l <= v.x and bb.r >= v.x and bb.b <= v.y and bb.t >= v.y, cpTrue, cpFalse )
#endif


''/ Returns a bounding box that holds both bounding boxes.
#ifndef cpBBMerge
#define cpBBMerge( a, b )	type<cpBB>( cpfmin( a.l, b.l ), cpfmin( a.b, b.b ), cpfmax( a.r, b.r ), cpfmax( a.t, b.t ) )
#endif



''/ Returns a bounding box that holds both @c bb and @c v.
#ifndef cpBBExpand
#define cpBBMerge( a, v )	type<cpBB>( cpfmin( bb.l, v.x ), cpfmin( bb.b, v.y ), cpfmax( bb.r, v.xr ), cpfmax( bb.t, v.y ) )
#endif



''/ Returns the area of the bounding box.
#ifndef cpBBArea
#define cpBBArea( bb )	cast( cpFloat, (bb.r - bb.l) * (bb.t*bb.b) )
#endif


''/ Merges @c a and @c b and returns the area of the merged bounding box.
#ifndef cpBBMergedArea
#define cpBBMergedArea( a, b )	cast( cpFloat, (cpfmax(a.r, b.r) - cpfmin(a.l, b.l))*(cpfmax(a.t, b.t) - cpfmin(a.b, b.b)) )
#endif

''/ Returns the fraction along the segment query the cpBB is hit. Returns INFINITY if it doesn't hit.
'' TODO....
static inline cpFloat cpBBSegmentQuery(cpBB bb, cpVect a, cpVect b)
{
	cpFloat idx = 1.0f/(b.x - a.x);
	cpFloat tx1 = (bb.l == a.x ? -INFINITY : (bb.l - a.x)*idx);
	cpFloat tx2 = (bb.r == a.x ?  INFINITY : (bb.r - a.x)*idx);
	cpFloat txmin = cpfmin(tx1, tx2);
	cpFloat txmax = cpfmax(tx1, tx2);
	
	cpFloat idy = 1.0f/(b.y - a.y);
	cpFloat ty1 = (bb.b == a.y ? -INFINITY : (bb.b - a.y)*idy);
	cpFloat ty2 = (bb.t == a.y ?  INFINITY : (bb.t - a.y)*idy);
	cpFloat tymin = cpfmin(ty1, ty2);
	cpFloat tymax = cpfmax(ty1, ty2);
	
	if(tymin <= txmax && txmin <= tymax){
		cpFloat min = cpfmax(txmin, tymin);
		cpFloat max = cpfmin(txmax, tymax);
		
		if(0.0 <= max && min <= 1.0) return cpfmax(min, 0.0);
	}
	
	return INFINITY;
}



''/ Return true if the bounding box intersects the line segment with ends @c a and @c b.
#ifndef cpBBIntersectsSegment
#define cpBBIntersectsSegment( bb, a, b )	iif( cpBBSegmentQuery( bb, a, b ) != INFINITY, cpTrue, cpFalse )
#endif


''/ Clamp a vector to a bounding box.
#ifndef cpBBClampVect
#define cpBBClampVect( bb, v )	type<cpVect>( cpfclamp( v.x, bb.l, bb.r ), cpfclamp( v.y, bb.b, bb.t ) )
#endif


''/ Wrap a vector to a bounding box.
cpVect cpBBWrapVect(const cpBB bb, const cpVect v); '' wrap a vector to a bbox
declare function cpBBWrapVect( byval bb as const cpBB, byval v as const cpVect ) as cpVect

end extern

#endif