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
 
#include <stdlib.h>
#include <math.h>
//#include <stdio.h>

#include "chipmunk_private.h"

#include "ChipmunkDemo.h"

typedef int (*collisionFunc)(const cpShape *, const cpShape *, cpContact *);

// Add contact points for circle to circle collisions.
// Used by several collision tests.
// TODO should accept hash parameter
static int
circle2circleQuery(const cpVect p1, const cpVect p2, const cpFloat r1, const cpFloat r2, cpContact *con)
{
	cpFloat mindist = r1 + r2;
	cpVect delta = cpvsub(p2, p1);
	cpFloat distsq = cpvlengthsq(delta);
	
	if(distsq < mindist*mindist){
		cpFloat dist = cpfsqrt(distsq);
		cpVect n = (dist ? cpvmult(delta, 1.0f/dist) : cpv(1.0f, 0.0f));
		cpContactInit(con, cpvlerp(p1, p2, r1/(r1 + r2)), n, dist - mindist, 0);
		
		return 1;
	} else {
		return 0;
	}
}

// Collide circle shapes.
static int
circle2circle(const cpCircleShape *c1, const cpCircleShape *c2, cpContact *arr)
{
	return circle2circleQuery(c1->tc, c2->tc, c1->r, c2->r, arr);
}

static int
circle2segment(const cpCircleShape *circleShape, const cpSegmentShape *segmentShape, cpContact *con)
{
	cpVect seg_a = segmentShape->ta;
	cpVect seg_b = segmentShape->tb;
	cpVect center = circleShape->tc;
	
	cpVect seg_delta = cpvsub(seg_b, seg_a);
	cpFloat closest_t = cpfclamp01(cpvdot(seg_delta, cpvsub(center, seg_a))/cpvlengthsq(seg_delta));
	cpVect closest = cpvadd(seg_a, cpvmult(seg_delta, closest_t));
	
	if(circle2circleQuery(center, closest, circleShape->r, segmentShape->r, con)){
		cpVect n = con[0].n;
		
		// Reject endcap collisions if tangents are provided.
		if(
			(closest_t == 0.0f && cpvdot(n, segmentShape->a_tangent) < 0.0) ||
			(closest_t == 1.0f && cpvdot(n, segmentShape->b_tangent) < 0.0)
		){
			return 0;
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

struct EdgePoint {
	cpVect p;
	int hash;
};

struct Edge {
	struct EdgePoint a, b;
	cpFloat r;
	cpVect n;
};

static inline struct Edge
EdgeNew(cpVect va, cpVect vb, int ha, int hb, cpFloat r)
{
	struct Edge edge = {{va, ha}, {vb, hb}, r, cpvnormalize(cpvperp(cpvsub(vb, va)))};
	return edge;
}

static struct Edge
SupportEdgeForPoly(const cpPolyShape *poly, const cpVect n)
{
	int numVerts = poly->numVerts;
	int i1 = cpSupportPointIndex(poly, n);
	int i0 = (i1 - 1 + numVerts)%numVerts;
	int i2 = (i1 + 1)%numVerts;
	
	cpVect v0 = poly->tVerts[i0];
	cpVect v1 = poly->tVerts[i1];
	cpVect v2 = poly->tVerts[i2];
	
	if(cpvdot(n, cpvsub(v1, v0)) < cpvdot(n, cpvsub(v1, v2))){
		return EdgeNew(v0, v1, CP_HASH_PAIR(poly, i0), CP_HASH_PAIR(poly, i1), 0.0f);
	} else {
		return EdgeNew(v1, v2, CP_HASH_PAIR(poly, i1), CP_HASH_PAIR(poly, i2), 0.0f);
	}
}

static struct Edge
SupportEdgeForSegment(const cpSegmentShape *seg, const cpVect n)
{
	if(cpvdot(seg->tn, n) > 0.0){
		return EdgeNew(seg->ta, seg->tb, CP_HASH_PAIR(seg, 0), CP_HASH_PAIR(seg, 1), seg->r);
	} else {
		return EdgeNew(seg->tb, seg->ta, CP_HASH_PAIR(seg, 1), CP_HASH_PAIR(seg, 0), seg->r);
	}
}

static inline int
ClipContact(const cpFloat d, const cpFloat t, const struct EdgePoint p1, const struct EdgePoint p2, const cpFloat r1, const cpFloat r2, const cpVect refn, const cpVect n, cpContact *arr)
{
	if(d <= 0.0){
		cpFloat dn = d*0.5f;
		cpVect point = t < 1.0 ? cpvadd(p1.p, cpvmult(refn, dn + r1)) : cpvadd(p2.p, cpvmult(refn, -(dn + r2)));
//		cpFloat dn = r1 + r2;
//		cpVect point = t < 1.0 ? cpvadd(p1.p, cpvmult(n, 0.0)) : cpvadd(p2.p, cpvmult(n, r1 + r2));
		cpContactInit(arr, point, n, d, CP_HASH_PAIR(p1.hash, p2.hash));
		return 1;
	} else {
		return 0;
	}
}

static int
ClipContacts(const struct Edge ref, const struct Edge inc, cpFloat flipped, cpContact *arr)
{
	cpFloat cian = cpvcross(inc.a.p, ref.n);
	cpFloat cibn = cpvcross(inc.b.p, ref.n);
	cpFloat cran = cpvcross(ref.a.p, ref.n);
	cpFloat crbn = cpvcross(ref.b.p, ref.n);
	
	cpFloat dran = cpvdot(ref.a.p, ref.n) + ref.r + inc.r;
	cpFloat dian = cpvdot(inc.a.p, ref.n) - dran;
	cpFloat dibn = cpvdot(inc.b.p, ref.n) - dran;
	
	cpVect n = cpvmult(ref.n, flipped);
	cpFloat t1 = cpfclamp01((cian - cran)/(cian - cibn));
	cpFloat t2 = cpfclamp01((cibn - crbn)/(cibn - cian));
	ChipmunkDemoPrintString("t1: %.2f, t2: %.2f, t1xt2: %.2f    %s\n", t1, t2, t1*t2, t1*t2 == 0 ? "XXXXXX" : "");
//	cpAssertWarn(t1*t2 != 0.0, "This?");
//	printf("t1*t2: %.2f\n", t1*t2);
	
	cpFloat d = -(ref.r + inc.r);
	ChipmunkDebugDrawSegment(ref.a.p, ref.b.p, RGBAColor(1, 0, 0, 1));
	ChipmunkDebugDrawSegment(cpvadd(inc.a.p, cpvmult(ref.n, d)), cpvadd(inc.b.p, cpvmult(ref.n, d)), RGBAColor(0, 1, 0, 1));
	
	ChipmunkDebugDrawFatSegment(ref.a.p, ref.b.p, ref.r + inc.r, RGBAColor(1, 0, 0, 1), RGBAColor(0, 0, 0, 0));
	ChipmunkDebugDrawSegment(inc.a.p, inc.b.p, RGBAColor(0, 1, 0, 1));
	
	if(t1*t2 != 0){
		int count = ClipContact(cpflerp(dian, dibn, t1), t1, ref.a, inc.b, ref.r, inc.r, ref.n, n, arr);
		return count + ClipContact(cpflerp(dibn, dian, t2), t2, ref.b, inc.a, ref.r, inc.r, ref.n, n, arr + count);
	} else {
		cpAssertSoft(t1 + t2 == 1.0, "These should sum to 1.0?");
		
		// TODO radii could use some tweaking here.
		if(t1 == 0){
			return circle2circleQuery(ref.a.p, inc.a.p, ref.r, inc.r, arr);
		} else {
			return circle2circleQuery(ref.b.p, inc.b.p, ref.r, inc.r, arr);
		}
	}
}

static int
ContactPoints(const struct Edge e1, const struct Edge e2, const cpVect n, cpContact *arr)
{
	if(cpvdot(e1.n, n) > -cpvdot(e2.n, n)){
		return ClipContacts(e1, e2,  1.0, arr);
	} else {
		return ClipContacts(e2, e1, -1.0, arr);
	}
}

// Find the minimum separating axis for the give poly and axis list.
static inline int
findMSA(const cpPolyShape *poly, const cpSplittingPlane *planes, const int num, cpFloat *min_out)
{
	int min_index = 0;
	cpFloat min = cpPolyShapeValueOnAxis(poly, planes->n, planes->d);
	if(min > 0.0f) return -1;
	
	for(int i=1; i<num; i++){
		cpFloat dist = cpPolyShapeValueOnAxis(poly, planes[i].n, planes[i].d);
		if(dist > 0.0f) {
			return -1;
		} else if(dist > min){
			min = dist;
			min_index = i;
		}
	}
	
	(*min_out) = min;
	return min_index;
}

// Collide poly shapes together.
static int
poly2poly(const cpPolyShape *poly1, const cpPolyShape *poly2, cpContact *arr)
{
	// TODO use the support point to find a good starting axis?
	// Does the MSA have to lie along the support vertex?
	
	cpFloat min1;
	int mini1 = findMSA(poly2, poly1->tPlanes, poly1->numVerts, &min1);
	if(mini1 == -1) return 0;
	
	cpFloat min2;
	int mini2 = findMSA(poly1, poly2->tPlanes, poly2->numVerts, &min2);
	if(mini2 == -1) return 0;
	
	// There is overlap, find the penetrating verts
	cpVect n = (min1 > min2 ? poly1->tPlanes[mini1].n : cpvneg(poly2->tPlanes[mini2].n));
	return ContactPoints(SupportEdgeForPoly(poly1, n), SupportEdgeForPoly(poly2, cpvneg(n)), n, arr);
}

// Like cpPolyValueOnAxis(), but for segments.
static inline cpFloat
segValueOnAxis(const cpSegmentShape *seg, const cpVect n, const cpFloat d)
{
	cpFloat a = cpvdot(n, seg->ta) - seg->r;
	cpFloat b = cpvdot(n, seg->tb) - seg->r;
	return cpfmin(a, b) - d;
}

// TODO: Comment me!
static int
seg2poly(const cpSegmentShape *seg, const cpPolyShape *poly, cpContact *arr)
{
	cpSplittingPlane *planes = poly->tPlanes;
	
	cpFloat segD = cpvdot(seg->tn, seg->ta);
	cpFloat minNorm = cpPolyShapeValueOnAxis(poly, seg->tn, segD) - seg->r;
	cpFloat minNeg = cpPolyShapeValueOnAxis(poly, cpvneg(seg->tn), -segD) - seg->r;
	
	cpFloat sepDist = cpfmax(minNorm, minNeg);
	if(sepDist > 0.0f) return 0;
	
	cpVect n = (sepDist == minNorm ? seg->tn : cpvneg(seg->tn));
	
	int numVerts = poly->numVerts;
	for(int i=0; i<numVerts; i++){
		cpFloat dist = segValueOnAxis(seg, planes[i].n, planes[i].d);
		if(dist > 0.0f){
			return 0;
		} else if(dist > sepDist){
			sepDist = dist;
			n = cpvneg(planes[i].n);
		}
	}
	
	return ContactPoints(SupportEdgeForSegment(seg, n), SupportEdgeForPoly(poly, cpvneg(n)), n, arr);
}

// This one is less gross, but still gross.
// TODO: Comment me!
static int
circle2poly(const cpCircleShape *circle, const cpPolyShape *poly, cpContact *con)
{
	cpSplittingPlane *planes = poly->tPlanes;
	
	int mini = 0;
	cpFloat min = cpSplittingPlaneCompare(planes[0], circle->tc) - circle->r;
	for(int i=0; i<poly->numVerts; i++){
		cpFloat dist = cpSplittingPlaneCompare(planes[i], circle->tc) - circle->r;
		if(dist > 0.0f){
			return 0;
		} else if(dist > min) {
			min = dist;
			mini = i;
		}
	}
	
	cpVect n = planes[mini].n;
	cpVect a = poly->tVerts[(mini - 1 + poly->numVerts)%poly->numVerts];
	cpVect b = poly->tVerts[mini];
	cpFloat dta = cpvcross(n, a);
	cpFloat dtb = cpvcross(n, b);
	cpFloat dt = cpvcross(n, circle->tc);
		
	if(dt < dtb){
		return circle2circleQuery(circle->tc, b, circle->r, 0.0f, con);
	} else if(dt < dta) {
		cpContactInit(
			con,
			cpvsub(circle->tc, cpvmult(n, circle->r + min/2.0f)),
			cpvneg(n),
			min,
			0				 
		);
	
		return 1;
	} else {
		return circle2circleQuery(circle->tc, a, circle->r, 0.0f, con);
	}
}

static const collisionFunc builtinCollisionFuncs[9] = {
	(collisionFunc)circle2circle,
	NULL,
	NULL,
	(collisionFunc)circle2segment,
	NULL,
	NULL,
	(collisionFunc)circle2poly,
	(collisionFunc)seg2poly,
	(collisionFunc)poly2poly,
};
static const collisionFunc *colfuncs = builtinCollisionFuncs;

int
cpCollideShapes(const cpShape *a, const cpShape *b, cpContact *arr)
{
	// Their shape types must be in order.
	cpAssertSoft(a->klass->type <= b->klass->type, "Internal Error: Collision shapes passed to cpCollideShapes() are not sorted.");
	
	collisionFunc cfunc = colfuncs[a->klass->type + b->klass->type*CP_NUM_SHAPES];
	return (cfunc) ? cfunc(a, b, arr) : 0;
}
