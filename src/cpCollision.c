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

//static cpVect
//SupportPoint_reference(const cpShape *shape, const cpVect n)
//{
//	cpFloat max = -INFINITY;
//	cpVect point = cpvzero;
//	
//	int count = cpPolyShapeGetNumVerts(shape);
//	for(int i=0; i<count; i++){
//		cpVect v = cpBodyLocal2World(shape->body, cpPolyShapeGetVert(shape, i));
//		cpFloat d = cpvdot(v, n);
//		if(d > max){
//			max = d;
//			point = v;
//		}
//	}
//	
//	return point;
//}

static inline int
SupportPointIndex(const cpShape *shape, const cpVect n)
{
	cpPolyShape *poly = (cpPolyShape *)shape;
	
	int min, max;
	if(cpvcross(poly->tPlanes[0].n, n) < 0.0){
		min = 0;
		max = poly->splitLeft;
	} else {
		min = poly->splitRight;
		max = poly->numVerts - 1;
	}
	
	while(min != max){
		int mid = (min + max + 1)/2;
		if(cpvcross(poly->tPlanes[mid].n, n) > 0.0){
			max = mid - 1;
		} else {
			min = mid;
		}
	}
	
	return min;
}

static cpVect
SupportPoint(const cpShape *shape, const cpVect n)
{
	cpPolyShape *poly = (cpPolyShape *)shape;
	
	cpVect point = poly->tVerts[SupportPointIndex(shape, n)];
//	cpVect point2 = SupportPoint_reference(shape, n);
//	cpAssertHard(cpfabs(cpvdot(point, n) - cpvdot(point2, n)) < 1e-5, "Support points not equal.");
	return point;
}

struct MinkowskiPoint {
	cpVect a, b, ab;
};

static inline struct MinkowskiPoint
Support(const cpShape *shape1, const cpShape *shape2, const cpVect n)
{
	cpVect a = SupportPoint(shape1, cpvneg(n));
	cpVect b = SupportPoint(shape2, n);
	
	struct MinkowskiPoint point = {a, b, cpvsub(b, a)};
	return point;
}

struct EdgePoint {
	cpVect v;
	int hash;
};

struct Edge {
	struct EdgePoint a, b;
	cpVect n;
};

static inline struct Edge
EdgeNew(cpVect va, cpVect vb, int ha, int hb)
{
	struct Edge edge = {{va, ha}, {vb, hb}, cpvnormalize(cpvperp(cpvsub(vb, va)))};
	return edge;
}

static struct Edge
SupportEdge(const cpShape *shape, const cpVect n)
{
	cpPolyShape *poly = (cpPolyShape *)shape;
	int numVerts = poly->numVerts;
	
	int i1 = SupportPointIndex(shape, n);
	int i0 = (i1 - 1 + numVerts)%numVerts;
	int i2 = (i1 + 1)%numVerts;
	
	cpVect v0 = poly->tVerts[i0];
	cpVect v1 = poly->tVerts[i1];
	cpVect v2 = poly->tVerts[i2];
	
	if(cpvdot(n, cpvsub(v1, v0)) < cpvdot(n, cpvsub(v1, v2))){
		return EdgeNew(v0, v1, CP_HASH_PAIR(shape, i0), CP_HASH_PAIR(shape, i1));
	} else {
		return EdgeNew(v1, v2, CP_HASH_PAIR(shape, i1), CP_HASH_PAIR(shape, i2));
	}
}

static cpBool
ContainsOrigin(const cpVect a, const cpVect b, const cpVect c)
{
	cpVect v0 = cpvsub(a, b);
	cpVect v1 = cpvsub(c, b);

	cpFloat dot00 = cpvdot(v0, v0);
	cpFloat dot01 = cpvdot(v0, v1);
	cpFloat dot0v = cpvdot(v0, cpvneg(b));
	cpFloat dot11 = cpvdot(v1, v1);
	cpFloat dot1v = cpvdot(v1, cpvneg(b));

	cpFloat det = dot00*dot11 - dot01*dot01;
	cpVect v = cpvmult(cpv(dot11*dot0v - dot01*dot1v, dot00*dot1v - dot01*dot0v), 1.0/det);
	return (v.x >= 0.0 && v.y >= 0.0 && v.x + v.y <= 1.0);
}

static cpFloat
ClosestT(const cpVect a, const cpVect b)
{
	cpVect delta = cpvsub(b, a);
	return cpfclamp01(cpvdot(delta, cpvneg(a))/cpvlengthsq(delta));
}

struct ClosestPoints {
	cpVect a, b;
	cpFloat d;
};

static struct ClosestPoints
ClosestPointsNew(const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, const cpFloat t, const cpFloat coef)
{
	cpVect pa = cpvlerp(v0.a, v1.a, t);
	cpVect pb = cpvlerp(v0.b, v1.b, t);
	
	struct ClosestPoints points = {pa, pb, coef*cpvdist(pa, pb)};
	return points;
}

struct EPANode {
	struct MinkowskiPoint v0, v1;
	struct EPANode *best, *left, *right, *parent;
	cpVect closest;
	cpFloat t, dist;
};

static void
EPANodeInit(struct EPANode *node, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1)
{
	cpFloat t = ClosestT(v0.ab, v1.ab);
	cpVect closest = cpvlerp(v0.ab, v1.ab, t);
	
	node->v0 = v0;
	node->v1 = v1;
	node->best = node;
	node->closest = closest;
	node->t = t;
	node->dist = cpvlength(closest);
}

static void
EPANodeSplit(struct EPANode *parent, struct EPANode *left, struct EPANode *right)
{
	parent->left = left;
	parent->right = right;
	left->parent = right->parent = parent;
	
	for(struct EPANode *node = parent; node; node = node->parent){
		if(node->left->dist < node->right->dist){
			node->dist = node->left->dist;
			node->best = node->left->best;
		} else {
			node->dist = node->right->dist;
			node->best = node->right->best;
		}
	}
}

static struct ClosestPoints
EPARecurse(const cpShape *a, const cpShape *b, struct EPANode *root)//, int i)
{
//	cpAssertHard(i < 20, "Stuck in recursion?");
	
	struct EPANode *best = root->best;
	cpVect closest = best->closest;
	struct MinkowskiPoint p = Support(a, b, closest);
	
	cpFloat dp = cpvdot(closest, p.ab);
	cpFloat d2 = cpfmax(cpvdot(closest, best->v0.ab), cpvdot(closest, best->v1.ab));
	
//	if(dp > d2){
	if(dp - d2 > 1e-5f){ // TODO eww, magic number
		struct EPANode left; EPANodeInit(&left, best->v0, p);
		struct EPANode right; EPANodeInit(&right, p, best->v1);
		EPANodeSplit(best, &left, &right);
		
		return EPARecurse(a, b, root);//, i+1);
	} else {
//		printf("EPA iterations %d\n", i);
		return ClosestPointsNew(best->v0, best->v1, best->t, -1.0f);
	}
}

static struct ClosestPoints
EPA(const cpShape *a, const cpShape *b, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, const struct MinkowskiPoint v2)
{
	struct EPANode n01; EPANodeInit(&n01, v0, v1);
	struct EPANode n12; EPANodeInit(&n12, v1, v2);
	struct EPANode n20; EPANodeInit(&n20, v2, v0);
	
	struct EPANode inner = {}, root = {};
	EPANodeSplit(&inner, &n01, &n12);
	EPANodeSplit(&root, &inner, &n20);
	
	return EPARecurse(a, b, &root);//, 1);
}

static struct ClosestPoints
GJKRecurse(const cpShape *a, const cpShape *b, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1)//, int i)
{
//	cpAssertHard(i < 20, "Stuck in recursion?");
	
	cpFloat t = ClosestT(v0.ab, v1.ab);
	cpVect closest = cpvlerp(v0.ab, v1.ab, t);
	struct MinkowskiPoint p = Support(a, b, cpvneg(closest));
	
	cpFloat dp = cpvdot(closest, p.ab);
	cpFloat d2 = cpfmin(cpvdot(closest, v0.ab), cpvdot(closest, v1.ab));
//	cpFloat area = cpvcross(cpvsub(v1.ab, v0.ab), cpvsub(p.ab, v0.ab));
//	printf("dp:%f, d2:%f, area:%f\n", dp, d2, area);
	if(dp <= 0.0 && ContainsOrigin(v0.ab, v1.ab, p.ab)){
//		printf("GJK iterations %d\n", i);
		return EPA(a, b, v0, v1, p);
//	} else if(dp < cpfmin(cpvdot(closest, v0.ab), cpvdot(closest, v1.ab))){
	} else if(dp - d2 < -1e-5f){ // TODO eww, magic number
		if(cpvlengthsq(v0.ab) <= cpvlengthsq(v1.ab)){
			return GJKRecurse(a, b, v0, p);//, i+1);
		} else {
			return GJKRecurse(a, b, p, v1);//, i+1);
		}
	} else {
//		printf("GJK iterations %d\n", i);
		return ClosestPointsNew(v0, v1, t, 1.0);
	}
}

static struct ClosestPoints
ClosestPoints(const cpShape *a, const cpShape *b)
{
	cpVect axis = cpvperp(cpvsub(a->body->p, b->body->p));
	return GJKRecurse(a, b, Support(a, b, axis), Support(a, b, cpvneg(axis)));//, 1);
}

// Helper function for working with contact buffers
// This used to malloc/realloc memory on the fly but was repurposed.
static cpContact *
nextContactPoint(cpContact *arr, int *numPtr)
{
	int index = *numPtr;
	
	if(index < CP_MAX_CONTACTS_PER_ARBITER){
		(*numPtr) = index + 1;
		return &arr[index];
	} else {
		return &arr[CP_MAX_CONTACTS_PER_ARBITER - 1];
	}
}

static int
ClipContacts(const struct Edge ref, const struct Edge inc, cpFloat flipped, cpContact *arr)
{
	cpFloat cian = cpvcross(inc.a.v, ref.n);
	cpFloat cibn = cpvcross(inc.b.v, ref.n);
	cpFloat cran = cpvcross(ref.a.v, ref.n);
	cpFloat crbn = cpvcross(ref.b.v, ref.n);
	
	cpFloat dran = cpvdot(ref.a.v, ref.n);
	cpFloat dian = cpvdot(inc.a.v, ref.n) - dran;
	cpFloat dibn = cpvdot(inc.b.v, ref.n) - dran;
	
	int numContacts = 0;
	
	cpFloat t1 = cpfclamp01((cian - cran)/(cian - cibn));
	cpFloat d1 = cpflerp(dian, dibn, t1);
	if(d1 < 0.0){
		cpContactInit(nextContactPoint(arr, &numContacts), t1 < 1.0 ? ref.a.v : inc.b.v, cpvmult(ref.n, flipped), d1, CP_HASH_PAIR(ref.a.hash, inc.b.hash));
	}
	
	cpFloat t2 = cpfclamp01((cibn - crbn)/(cibn - cian));
	cpFloat d2 = cpflerp(dibn, dian, t2);
	if(d2 < 0.0){
		cpContactInit(nextContactPoint(arr, &numContacts), t2 < 1.0 ? ref.b.v : inc.a.v, cpvmult(ref.n, flipped), d2, CP_HASH_PAIR(ref.b.hash, inc.a.hash));
	}
	
	cpAssertWarn(numContacts > 0, "No contacts?");
	return numContacts;
}

static int
ContactPoints(const cpShape *a, const cpShape *b, const struct ClosestPoints points, cpContact *arr)
{
	if(points.d > 0.0) return 0;
	
	cpVect n = cpvmult(cpvsub(points.b, points.a), 1.0f/points.d);
	struct Edge f1 = SupportEdge(a, n);
	struct Edge f2 = SupportEdge(b, cpvneg(n));
	
	if(cpvdot(f1.n, n) > -cpvdot(f2.n, n)){
		return ClipContacts(f1, f2,  1.0, arr);
	} else {
		return ClipContacts(f2, f1, -1.0, arr);
	}
}

typedef int (*collisionFunc)(const cpShape *, const cpShape *, cpContact *);

// Add contact points for circle to circle collisions.
// Used by several collision tests.
static int
circle2circleQuery(const cpVect p1, const cpVect p2, const cpFloat r1, const cpFloat r2, cpContact *con)
{
	cpFloat mindist = r1 + r2;
	cpVect delta = cpvsub(p2, p1);
	cpFloat distsq = cpvlengthsq(delta);
	if(distsq >= mindist*mindist) return 0;
	
	cpFloat dist = cpfsqrt(distsq);

	// Allocate and initialize the contact.
	cpContactInit(
		con,
		cpvadd(p1, cpvmult(delta, 0.5f + (r1 - 0.5f*mindist)/(dist ? dist : INFINITY))),
		(dist ? cpvmult(delta, 1.0f/dist) : cpv(1.0f, 0.0f)),
		dist - mindist,
		0
	);
	
	return 1;
}

// Collide circle shapes.
static int
circle2circle(const cpShape *shape1, const cpShape *shape2, cpContact *arr)
{
	cpCircleShape *circ1 = (cpCircleShape *)shape1; //TODO
	cpCircleShape *circ2 = (cpCircleShape *)shape2;
	
	return circle2circleQuery(circ1->tc, circ2->tc, circ1->r, circ2->r, arr);
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
		) return 0;
		
		return 1;
	} else {
		return 0;
	}
}

//// Find the minimum separating axis for the give poly and axis list.
//static inline int
//findMSA(const cpPolyShape *poly, const cpSplittingPlane *planes, const int num, cpFloat *min_out)
//{
//	int min_index = 0;
//	cpFloat min = cpPolyShapeValueOnAxis(poly, planes->n, planes->d);
//	if(min > 0.0f) return -1;
//	
//	for(int i=1; i<num; i++){
//		cpFloat dist = cpPolyShapeValueOnAxis(poly, planes[i].n, planes[i].d);
//		if(dist > 0.0f) {
//			return -1;
//		} else if(dist > min){
//			min = dist;
//			min_index = i;
//		}
//	}
//	
//	(*min_out) = min;
//	return min_index;
//}
//
//// Add contacts for probably penetrating vertexes.
//// This handles the degenerate case where an overlap was detected, but no vertexes fall inside
//// the opposing polygon. (like a star of david)
//static inline int
//findVertsFallback(cpContact *arr, const cpPolyShape *poly1, const cpPolyShape *poly2, const cpVect n, const cpFloat dist)
//{
//	int num = 0;
//	
//	for(int i=0; i<poly1->numVerts; i++){
//		cpVect v = poly1->tVerts[i];
//		if(cpPolyShapeContainsVertPartial(poly2, v, cpvneg(n)))
//			cpContactInit(nextContactPoint(arr, &num), v, n, dist, CP_HASH_PAIR(poly1->shape.hashid, i));
//	}
//	
//	for(int i=0; i<poly2->numVerts; i++){
//		cpVect v = poly2->tVerts[i];
//		if(cpPolyShapeContainsVertPartial(poly1, v, n))
//			cpContactInit(nextContactPoint(arr, &num), v, n, dist, CP_HASH_PAIR(poly2->shape.hashid, i));
//	}
//	
//	return num;
//}
//
//// Add contacts for penetrating vertexes.
//static inline int
//findVerts(cpContact *arr, const cpPolyShape *poly1, const cpPolyShape *poly2, const cpVect n, const cpFloat dist)
//{
//	int num = 0;
//	
//	for(int i=0; i<poly1->numVerts; i++){
//		cpVect v = poly1->tVerts[i];
//		if(cpPolyShapeContainsVert(poly2, v))
//			cpContactInit(nextContactPoint(arr, &num), v, n, dist, CP_HASH_PAIR(poly1->shape.hashid, i));
//	}
//	
//	for(int i=0; i<poly2->numVerts; i++){
//		cpVect v = poly2->tVerts[i];
//		if(cpPolyShapeContainsVert(poly1, v))
//			cpContactInit(nextContactPoint(arr, &num), v, n, dist, CP_HASH_PAIR(poly2->shape.hashid, i));
//	}
//	
//	return (num ? num : findVertsFallback(arr, poly1, poly2, n, dist));
//}
//
//// Collide poly shapes together.
//static int
//poly2poly(const cpShape *shape1, const cpShape *shape2, cpContact *arr)
//{
//	cpPolyShape *poly1 = (cpPolyShape *)shape1;
//	cpPolyShape *poly2 = (cpPolyShape *)shape2;
//	
//	cpFloat min1;
//	int mini1 = findMSA(poly2, poly1->tPlanes, poly1->numVerts, &min1);
//	if(mini1 == -1) return 0;
//	
//	cpFloat min2;
//	int mini2 = findMSA(poly1, poly2->tPlanes, poly2->numVerts, &min2);
//	if(mini2 == -1) return 0;
//	
//	// There is overlap, find the penetrating verts
//	if(min1 > min2)
//		return findVerts(arr, poly1, poly2, poly1->tPlanes[mini1].n, min1);
//	else
//		return findVerts(arr, poly1, poly2, cpvneg(poly2->tPlanes[mini2].n), min2);
//}

static int
poly2poly(const cpShape *shape1, const cpShape *shape2, cpContact *arr)
{
	return ContactPoints(shape1, shape2, ClosestPoints(shape1, shape2), arr);
}

// Like cpPolyValueOnAxis(), but for segments.
static inline cpFloat
segValueOnAxis(const cpSegmentShape *seg, const cpVect n, const cpFloat d)
{
	cpFloat a = cpvdot(n, seg->ta) - seg->r;
	cpFloat b = cpvdot(n, seg->tb) - seg->r;
	return cpfmin(a, b) - d;
}

// Identify vertexes that have penetrated the segment.
static inline void
findPointsBehindSeg(cpContact *arr, int *num, const cpSegmentShape *seg, const cpPolyShape *poly, const cpFloat pDist, const cpFloat coef) 
{
	cpFloat dta = cpvcross(seg->tn, seg->ta);
	cpFloat dtb = cpvcross(seg->tn, seg->tb);
	cpVect n = cpvmult(seg->tn, coef);
	
	for(int i=0; i<poly->numVerts; i++){
		cpVect v = poly->tVerts[i];
		if(cpvdot(v, n) < cpvdot(seg->tn, seg->ta)*coef + seg->r){
			cpFloat dt = cpvcross(seg->tn, v);
			if(dta >= dt && dt >= dtb){
				cpContactInit(nextContactPoint(arr, num), v, n, pDist, CP_HASH_PAIR(poly->shape.hashid, i));
			}
		}
	}
}

// This one is complicated and gross. Just don't go there...
// TODO: Comment me!
static int
seg2poly(const cpShape *shape1, const cpShape *shape2, cpContact *arr)
{
	cpSegmentShape *seg = (cpSegmentShape *)shape1;
	cpPolyShape *poly = (cpPolyShape *)shape2;
	cpSplittingPlane *planes = poly->tPlanes;
	
	cpFloat segD = cpvdot(seg->tn, seg->ta);
	cpFloat minNorm = cpPolyShapeValueOnAxis(poly, seg->tn, segD) - seg->r;
	cpFloat minNeg = cpPolyShapeValueOnAxis(poly, cpvneg(seg->tn), -segD) - seg->r;
	if(minNeg > 0.0f || minNorm > 0.0f) return 0;
	
	int mini = 0;
	cpFloat poly_min = segValueOnAxis(seg, planes->n, planes->d);
	if(poly_min > 0.0f) return 0;
	for(int i=0; i<poly->numVerts; i++){
		cpFloat dist = segValueOnAxis(seg, planes[i].n, planes[i].d);
		if(dist > 0.0f){
			return 0;
		} else if(dist > poly_min){
			poly_min = dist;
			mini = i;
		}
	}
	
	int num = 0;
	
	cpVect poly_n = cpvneg(planes[mini].n);
	
	cpVect va = cpvadd(seg->ta, cpvmult(poly_n, seg->r));
	cpVect vb = cpvadd(seg->tb, cpvmult(poly_n, seg->r));
	if(cpPolyShapeContainsVert(poly, va))
		cpContactInit(nextContactPoint(arr, &num), va, poly_n, poly_min, CP_HASH_PAIR(seg->shape.hashid, 0));
	if(cpPolyShapeContainsVert(poly, vb))
		cpContactInit(nextContactPoint(arr, &num), vb, poly_n, poly_min, CP_HASH_PAIR(seg->shape.hashid, 1));
	
	// Floating point precision problems here.
	// This will have to do for now.
//	poly_min -= cp_collision_slop; // TODO is this needed anymore?
	
	if(minNorm >= poly_min || minNeg >= poly_min) {
		if(minNorm > minNeg)
			findPointsBehindSeg(arr, &num, seg, poly, minNorm, 1.0f);
		else
			findPointsBehindSeg(arr, &num, seg, poly, minNeg, -1.0f);
	}
	
	// If no other collision points are found, try colliding endpoints.
	if(num == 0){
		cpVect poly_a = poly->tVerts[mini];
		cpVect poly_b = poly->tVerts[(mini + 1)%poly->numVerts];
		
		if(circle2circleQuery(seg->ta, poly_a, seg->r, 0.0f, arr)) return 1;
		if(circle2circleQuery(seg->tb, poly_a, seg->r, 0.0f, arr)) return 1;
		if(circle2circleQuery(seg->ta, poly_b, seg->r, 0.0f, arr)) return 1;
		if(circle2circleQuery(seg->tb, poly_b, seg->r, 0.0f, arr)) return 1;
	}

	return num;
}

// This one is less gross, but still gross.
// TODO: Comment me!
static int
circle2poly(const cpShape *shape1, const cpShape *shape2, cpContact *con)
{
	cpCircleShape *circ = (cpCircleShape *)shape1;
	cpPolyShape *poly = (cpPolyShape *)shape2;
	cpSplittingPlane *planes = poly->tPlanes;
	
	int numVerts = poly->numVerts;
	int mini = 0;
	cpFloat min = cpSplittingPlaneCompare(planes[0], circ->tc) - circ->r;
	for(int i=0; i<numVerts; i++){
		cpFloat dist = cpSplittingPlaneCompare(planes[i], circ->tc) - circ->r;
		if(dist > 0.0f){
			return 0;
		} else if(dist > min) {
			min = dist;
			mini = i;
		}
	}
	
	cpVect n = planes[mini].n;
	cpVect a = poly->tVerts[(mini - 1 + numVerts)%numVerts];
	cpVect b = poly->tVerts[mini];
	cpFloat dta = cpvcross(n, a);
	cpFloat dtb = cpvcross(n, b);
	cpFloat dt = cpvcross(n, circ->tc);
		
	if(dt < dtb){
		return circle2circleQuery(circ->tc, b, circ->r, 0.0f, con);
	} else if(dt < dta) {
		cpContactInit(
			con,
			cpvsub(circ->tc, cpvmult(n, circ->r + min/2.0f)),
			cpvneg(n),
			min,
			0				 
		);
	
		return 1;
	} else {
		return circle2circleQuery(circ->tc, a, circ->r, 0.0f, con);
	}
}

static const collisionFunc builtinCollisionFuncs[9] = {
	circle2circle,
	NULL,
	NULL,
	(collisionFunc)circle2segment,
	NULL,
	NULL,
	circle2poly,
	seg2poly,
	poly2poly,
};
static const collisionFunc *colfuncs = builtinCollisionFuncs;

int
cpCollideShapes(const cpShape *a, const cpShape *b, cpContact *arr)
{
	// Their shape types must be in order.
	cpAssertSoft(a->klass->type <= b->klass->type, "Collision shapes passed to cpCollideShapes() are not sorted.");
	
	collisionFunc cfunc = colfuncs[a->klass->type + b->klass->type*CP_NUM_SHAPES];
	return (cfunc) ? cfunc(a, b, arr) : 0;
}
