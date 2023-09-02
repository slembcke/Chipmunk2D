/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
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

#ifndef CHIPMUNK_BB_H
#define CHIPMUNK_BB_H

#include "chipmunk_types.h"
#include "cpVect.h"

/// @defgroup cpBBB cpBB
/// Chipmunk's axis-aligned 2D bounding box type along with a few handy routines.
/// @{

/// Chipmunk's axis-aligned 2D bounding box type. (left, bottom, right, top)
typedef struct cpBB{
	cpFloat l, b, r ,t;
} cpBB;

/// Convenience constructor for cpBB structs.
inline cpBB cpBBNew(const cpFloat l, const cpFloat b, const cpFloat r, const cpFloat t)
{
	cpBB bb = {l, b, r, t};
	return bb;
}

/// Constructs a cpBB centered on a point with the given extents (half sizes).
inline cpBB
cpBBNewForExtents(const cpVect c, const cpFloat hw, const cpFloat hh)
{
	return cpBBNew(c.x - hw, c.y - hh, c.x + hw, c.y + hh);
}

/// Constructs a cpBB for a circle with the given position and radius.
inline cpBB cpBBNewForCircle(const cpVect p, const cpFloat r)
{
	return cpBBNewForExtents(p, r, r);
}

/// Returns true if @c a and @c b intersect.
inline cpBool cpBBIntersects(const cpBB a, const cpBB b)
{
	return (a.l <= b.r && b.l <= a.r && a.b <= b.t && b.b <= a.t);
}

/// Returns true if @c other lies completely within @c bb.
inline cpBool cpBBContainsBB(const cpBB bb, const cpBB other)
{
	return (bb.l <= other.l && bb.r >= other.r && bb.b <= other.b && bb.t >= other.t);
}

/// Returns true if @c bb contains @c v.
inline cpBool cpBBContainsVect(const cpBB bb, const cpVect v)
{
	return (bb.l <= v.x && bb.r >= v.x && bb.b <= v.y && bb.t >= v.y);
}

/// Returns a bounding box that holds both bounding boxes.
inline cpBB cpBBMerge(const cpBB a, const cpBB b){
	return cpBBNew(
		cpfmin(a.l, b.l),
		cpfmin(a.b, b.b),
		cpfmax(a.r, b.r),
		cpfmax(a.t, b.t)
	);
}

/// Returns a bounding box that holds both @c bb and @c v.
inline cpBB cpBBExpand(const cpBB bb, const cpVect v){
	return cpBBNew(
		cpfmin(bb.l, v.x),
		cpfmin(bb.b, v.y),
		cpfmax(bb.r, v.x),
		cpfmax(bb.t, v.y)
	);
}

/// Returns the center of a bounding box.
inline cpVect
cpBBCenter(cpBB bb)
{
	return cpvlerp(cpv(bb.l, bb.b), cpv(bb.r, bb.t), 0.5f);
}

/// Returns the area of the bounding box.
inline cpFloat cpBBArea(cpBB bb)
{
	return (bb.r - bb.l)*(bb.t - bb.b);
}

/// Merges @c a and @c b and returns the area of the merged bounding box.
inline cpFloat cpBBMergedArea(cpBB a, cpBB b)
{
	return (cpfmax(a.r, b.r) - cpfmin(a.l, b.l))*(cpfmax(a.t, b.t) - cpfmin(a.b, b.b));
}

/// Returns the fraction along the segment query the cpBB is hit. Returns INFINITY if it doesn't hit.
inline cpFloat cpBBSegmentQuery(cpBB bb, cpVect a, cpVect b)
{
	cpVect delta = cpvsub(b, a);
	cpFloat tmin = -INFINITY, tmax = INFINITY;
	
	if(delta.x == 0.0f){
		if(a.x < bb.l || bb.r < a.x) return INFINITY;
	} else {
		cpFloat t1 = (bb.l - a.x)/delta.x;
		cpFloat t2 = (bb.r - a.x)/delta.x;
		tmin = cpfmax(tmin, cpfmin(t1, t2));
		tmax = cpfmin(tmax, cpfmax(t1, t2));
	}
	
	if(delta.y == 0.0f){
		if(a.y < bb.b || bb.t < a.y) return INFINITY;
	} else {
		cpFloat t1 = (bb.b - a.y)/delta.y;
		cpFloat t2 = (bb.t - a.y)/delta.y;
		tmin = cpfmax(tmin, cpfmin(t1, t2));
		tmax = cpfmin(tmax, cpfmax(t1, t2));
	}
	
	if(tmin <= tmax && 0.0f <= tmax && tmin <= 1.0f){
		return cpfmax(tmin, 0.0f);
	} else {
		return INFINITY;
	}
}

/// Return true if the bounding box intersects the line segment with ends @c a and @c b.
inline cpBool cpBBIntersectsSegment(cpBB bb, cpVect a, cpVect b)
{
	return (cpBBSegmentQuery(bb, a, b) != INFINITY);
}

/// Clamp a vector to a bounding box.
inline cpVect
cpBBClampVect(const cpBB bb, const cpVect v)
{
	return cpv(cpfclamp(v.x, bb.l, bb.r), cpfclamp(v.y, bb.b, bb.t));
}

/// Wrap a vector to a bounding box.
inline cpVect
cpBBWrapVect(const cpBB bb, const cpVect v)
{
	cpFloat dx = cpfabs(bb.r - bb.l);
	cpFloat modx = cpfmod(v.x - bb.l, dx);
	cpFloat x = (modx > 0.0f) ? modx : modx + dx;
	
	cpFloat dy = cpfabs(bb.t - bb.b);
	cpFloat mody = cpfmod(v.y - bb.b, dy);
	cpFloat y = (mody > 0.0f) ? mody : mody + dy;
	
	return cpv(x + bb.l, y + bb.b);
}

/// Returns a bounding box offseted by @c v.
inline cpBB
cpBBOffset(const cpBB bb, const cpVect v)
{
	return cpBBNew(
		bb.l + v.x,
		bb.b + v.y,
		bb.r + v.x,
		bb.t + v.y
	);
}

///@}

#endif
