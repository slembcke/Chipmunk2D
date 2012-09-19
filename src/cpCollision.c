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
 
#include "chipmunk_private.h"
#include "ChipmunkDemo.h"

#define USE_GJK 0
#define DRAW_GJK 0
#define DRAW_EPA 0
#define DRAW_CLOSEST 0
#define DRAW_CLIP 0
#define PRINT_LOG 0

// Add contact points for circle to circle collisions.
// Used by several collision tests.
// TODO should accept hash parameter
static int
circle2circleQuery(const cpVect p1, const cpVect p2, const cpFloat r1, const cpFloat r2, cpHashValue hash, cpContact *con)
{
	cpFloat mindist = r1 + r2;
	cpVect delta = cpvsub(p2, p1);
	cpFloat distsq = cpvlengthsq(delta);
	
	if(distsq < mindist*mindist){
		cpFloat dist = cpfsqrt(distsq);
		cpVect n = (dist ? cpvmult(delta, 1.0f/dist) : cpv(1.0f, 0.0f));
		cpContactInit(con, cpvlerp(p1, p2, r1/(r1 + r2)), n, dist - mindist, hash);
		
		return 1;
	} else {
		return 0;
	}
}

//MARK: Support Points and Edges:

//static inline int
//cpSupportPointIndex(const cpPolyShape *poly, const cpVect n)
//{
//	cpFloat max = -INFINITY;
//	int index = 0;
//	
//	int numVerts = poly->numVerts;
//	cpVect *verts = poly->tVerts;
//	for(int i=0; i<numVerts; i++){
//		cpVect v = verts[i];
//		cpFloat d = cpvdot(v, n);
//		if(d > max){
//			max = d;
//			index = i;
//		}
//	}
//	
//	return index;
//}

static inline int
cpSupportPointIndex(const cpPolyShape *poly, const cpVect n)
{
	cpSplittingPlane *planes = poly->tPlanes;
	
	int min, max;
	if(cpvcross(planes[0].n, n) < 0.0){
		min = 0;
		max = poly->splitLeft;
	} else {
		min = poly->splitRight;
		max = poly->numVerts - 1;
	}
	
	while(min != max){
		int mid = (min + max + 1)/2;
		if(cpvcross(planes[mid].n, n) > 0.0){
			max = mid - 1;
		} else {
			min = mid;
		}
	}
	
	return min;
}

#if USE_GJK
static cpVect
cpPolySupportPoint(const cpPolyShape *poly, const cpVect n)
{
	return poly->tVerts[cpSupportPointIndex(poly, n)];
}

static cpVect
cpSegmentSupportPoint(const cpSegmentShape *seg, const cpVect n)
{
	cpVect a = seg->ta, b = seg->tb;
	return (cpvdot(a, n) > cpvdot(b, n) ? a : b);
}
#endif

//static cpVect
//cpCircleSupportPoint(const cpCircleShape *circle, const cpVect n)
//{
//	return circle->tc;
//}

static inline cpFloat
cpPolyShapeValueOnAxis(const cpPolyShape *poly, const cpVect n, const cpFloat d)
{
	cpVect p = poly->tVerts[cpSupportPointIndex(poly, cpvneg(n))];
	return cpvdot(n, p) - d;
}

struct MinkowskiPoint {
	cpVect a, b, ab;
};

typedef cpVect (*SupportFunction)(const cpShape *a, cpVect n);

struct SupportContext {
	const cpShape *shape1, *shape2;
	SupportFunction support1, support2;
};

static inline struct MinkowskiPoint
Support(const struct SupportContext context, const cpVect n)
{
	cpVect a = context.support1(context.shape1, cpvneg(n));
	cpVect b = context.support2(context.shape2, n);
	
	struct MinkowskiPoint point = {a, b, cpvsub(b, a)};
	return point;
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
	
	// TODO get rid of mod eventually, very expensive on ARM
	int i0 = (i1 - 1 + numVerts)%numVerts;
	int i2 = (i1 + 1)%numVerts;
	
	cpVect *verts = poly->tVerts;
	if(cpvdot(n, poly->tPlanes[i1].n) > cpvdot(n, poly->tPlanes[i2].n)){
		return (struct Edge){{verts[i0], CP_HASH_PAIR(poly, i0)}, {verts[i1], CP_HASH_PAIR(poly, i1)}, 0.0, poly->tPlanes[i1].n};
	} else {
		return (struct Edge){{verts[i1], CP_HASH_PAIR(poly, i1)}, {verts[i2], CP_HASH_PAIR(poly, i2)}, 0.0, poly->tPlanes[i2].n};
	}
}

static struct Edge
SupportEdgeForSegment(const cpSegmentShape *seg, const cpVect n)
{
	if(cpvdot(seg->tn, n) > 0.0){
		return (struct Edge){{seg->ta, CP_HASH_PAIR(seg, 0)}, {seg->tb, CP_HASH_PAIR(seg, 1)}, seg->r, seg->tn};
	} else {
		return (struct Edge){{seg->tb, CP_HASH_PAIR(seg, 1)}, {seg->ta, CP_HASH_PAIR(seg, 0)}, seg->r, cpvneg(seg->tn)};
	}
}

#if USE_GJK
static cpFloat
ClosestT(const cpVect a, const cpVect b)
{
	cpVect delta = cpvsub(b, a);
	return cpfclamp01(cpvdot(delta, cpvneg(a))/cpvlengthsq(delta));
}

struct ClosestPoints {
	cpVect a, b;
	cpFloat d;
	cpVect n;
};

static struct ClosestPoints
ClosestPointsNew(const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, const cpFloat t, const cpFloat coef)
{
	cpVect pa = cpvlerp(v0.a, v1.a, t);
	cpVect pb = cpvlerp(v0.b, v1.b, t);
	
	cpFloat d = coef*cpvdist(pa, pb);
	cpVect n = cpvnormalize(cpvmult(d != 0.0 ? cpvsub(pb, pa) : cpvperp(cpvsub(v0.ab, v1.ab)), coef));
	struct ClosestPoints points = {pa, pb, d, n};
	cpAssertWarn(cpvdist(cpvadd(points.a, cpvmult(points.n, points.d)), points.b) < 1e-5, "Bad closest points?");
	return points;
}

//MARK: EPA Functions

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
EPARecurse(const struct SupportContext context, struct EPANode *root, int i)
{
	cpAssertHard(i < 20, "Stuck in recursion?");
	
	struct EPANode *best = root->best;
	cpVect closest = best->closest;
	struct MinkowskiPoint p = Support(context, closest);
	
	struct MinkowskiPoint v0 = best->v0;
	struct MinkowskiPoint v1 = best->v1;
	
#if DRAW_EPA
	ChipmunkDebugDrawPolygon(3, (cpVect[]){v0.ab, v1.ab, p.ab}, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
	ChipmunkDebugDrawPoints(3.0, 1, &closest, RGBAColor(1, 1, 0, 1));
	ChipmunkDebugDrawSegment(closest, p.ab, RGBAColor(0, 0, 1, 1));
#endif
	
	if(cpvcross(cpvsub(v1.ab, v0.ab), cpvsub(p.ab, v0.ab)) < 0.0f){
		struct EPANode left; EPANodeInit(&left, v0, p);
		struct EPANode right; EPANodeInit(&right, p, v1);
		EPANodeSplit(best, &left, &right);
		
		return EPARecurse(context, root, i + 1);
	} else {
		return ClosestPointsNew(v0, v1, best->t, -1.0f);
	}
}

static struct ClosestPoints
EPA(const struct SupportContext context, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, const struct MinkowskiPoint v2)
{
#if DRAW_EPA || DRAW_GJK
	ChipmunkDebugDrawPolygon(3, (cpVect[]){v0.ab, v1.ab, v2.ab}, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
#endif

	struct EPANode n01; EPANodeInit(&n01, v0, v1);
	struct EPANode n12; EPANodeInit(&n12, v1, v2);
	struct EPANode n20; EPANodeInit(&n20, v2, v0);
	
	struct EPANode inner, root;
	bzero(&inner, sizeof(struct EPANode));
	bzero(&root, sizeof(struct EPANode));
	
	EPANodeSplit(&inner, &n01, &n12);
	EPANodeSplit(&root, &inner, &n20);
	
	return EPARecurse(context, &root, 1);
}

//MARK: GJK Functions.

static inline struct ClosestPoints
GJKRecurse(const struct SupportContext context, struct MinkowskiPoint v0, struct MinkowskiPoint v1)
{
	for(int i=1;; i++){
//		cpAssertSoft(cpvcross(cpvsub(v1.ab, v0.ab), cpvsub(cpvzero, v0.ab)) >= 0.0, "Segment oriented the wrong way.");
		cpAssertSoft(i < 100, "Stuck in GJK recursion");
		
		cpFloat t = ClosestT(v0.ab, v1.ab);
		cpVect closest = cpvlerp(v0.ab, v1.ab, t);
		struct MinkowskiPoint p = Support(context, cpvneg(closest));
		
#if DRAW_GJK
		ChipmunkDebugDrawSegment(v0.ab, v1.ab, RGBAColor(1, 1, 1, 1));
		ChipmunkDebugDrawPoints(3.0, 1, &closest, RGBAColor(1, 1, 1, 1));
		ChipmunkDebugDrawSegment(closest, p.ab, RGBAColor(0, 1, 0, 1));
#endif
		
		cpFloat area_01p = cpvcross(cpvsub(v1.ab, v0.ab), cpvsub(p.ab, v0.ab));
		cpFloat area_0pO = cpvcross(cpvsub(p.ab, v0.ab), cpvsub(cpvzero, v0.ab));
		cpFloat area_1pO = cpvcross(cpvsub(v1.ab, p.ab), cpvsub(cpvzero, p.ab));

		if(area_0pO <= 0.0f && area_1pO <= 0.0f){
			return EPA(context, v0, v1, p);
		} else if(area_01p <= 0.0f){
			return ClosestPointsNew(v0, v1, t, 1.0);
		} else {
			if(area_0pO > area_1pO){
				v1 = p;
			} else {
				v0 = p;
			}
		}
	}
}

#if DRAW_GJK || DRAW_EPA

static int
ShapePointCount(const cpShape *shape)
{
	switch(shape->klass->type){
		default: return 0;
		case CP_CIRCLE_SHAPE: return 1;
		case CP_SEGMENT_SHAPE: return 2;
		case CP_POLY_SHAPE: return ((cpPolyShape *)shape)->numVerts;
	}
}

static cpVect
ShapePoint(const cpShape *shape, int i)
{
	switch(shape->klass->type){
		default: return cpvzero;
		case CP_CIRCLE_SHAPE: return ((cpCircleShape *)shape)->tc;
		case CP_SEGMENT_SHAPE: {
			cpSegmentShape *seg = (void *)shape;
			return (i == 0 ? seg->ta : seg->tb);
		};
		case CP_POLY_SHAPE: return ((cpPolyShape *)shape)->tVerts[i];
	}
}

#endif

static struct ClosestPoints
GJK(const cpShape *shape1, const cpShape *shape2, SupportFunction support1, SupportFunction support2)
{
#if DRAW_GJK || DRAW_EPA
	// draw the minkowski difference origin
	cpVect origin = cpvzero;
	ChipmunkDebugDrawPoints(5.0, 1, &origin, RGBAColor(1,0,0,1));
	
	// draw the minkowski difference
	int shape1Count = ShapePointCount(shape1);
	int shape2Count = ShapePointCount(shape2);
	
	int mdiffCount = shape1Count*shape2Count;
	cpVect *mdiffVerts = alloca(mdiffCount*sizeof(cpVect));
	
	for(int i=0; i<shape1Count; i++){
		for(int j=0; j<shape2Count; j++){
			cpVect v1 = ShapePoint(shape1, i);
			cpVect v2 = ShapePoint(shape2, j);
			mdiffVerts[i*shape2Count + j] = cpvsub(v2, v1);
		}
	}
	
	cpVect *hullVerts = alloca(mdiffCount*sizeof(cpVect));
	int hullCount = cpConvexHull(mdiffCount, mdiffVerts, hullVerts, NULL, 0.0);
	
	ChipmunkDebugDrawPolygon(hullCount, hullVerts, RGBAColor(1, 0, 0, 1), RGBAColor(1, 0, 0, 0.25));
	ChipmunkDebugDrawPoints(2.0, mdiffCount, mdiffVerts, RGBAColor(1, 0, 0, 1));
#endif
	
	struct SupportContext context = {shape1, shape2, support1, support2};
	
	// TODO use centroids as the starting axis
	cpVect axis = cpvperp(cpvsub(shape1->body->p, shape2->body->p));
	struct MinkowskiPoint p1 = Support(context, axis);
	struct MinkowskiPoint p2 = Support(context, cpvneg(axis));
	
	// GJKRecurse requires a specific winding.
	cpFloat area = cpvcross(cpvsub(p2.ab, p1.ab), cpvsub(cpvzero, p1.ab));
	struct ClosestPoints points = (area > 0.0 ? GJKRecurse(context, p1, p2) : GJKRecurse(context, p2, p1));
	
	if(cpfabs(points.d) < MAGIC_EPSILON){
		// If the closest points are very close together, might need to estimate the normal.
		// TODO fix the constness to get rid of the cast.
		cpNearestPointQueryInfo info;
		cpShapeNearestPointQuery((cpShape *)shape1, points.b, &info);
		points.n = info.g;
		
		return points;
	} else {
		return points;
	}
}
#endif

//MARK: Contact Clipping

static inline int
ClipContact(
	const cpFloat pd, const cpFloat t,
	const struct EdgePoint p1, const struct EdgePoint p2,
	const cpFloat r1, const cpFloat r2,
	const cpVect refn, const cpVect n, cpContact *arr
){
	if(pd <= 0.0){
		cpFloat rsum = r1 + r2;
		cpFloat alpha = (rsum > 0.0f ? (rsum + pd)/rsum : 0.0f);
		cpVect point = (t < 1.0 ? cpvadd(p1.p, cpvmult(refn, r1*alpha)) : cpvadd(p2.p, cpvmult(refn, -r2*alpha)));
		cpContactInit(arr, point, n, pd, CP_HASH_PAIR(p1.hash, p2.hash));
		return 1;
	} else {
		return 0;
	}
}

static int
ClipContacts(const struct Edge ref, const struct Edge inc, cpVect n, cpContact *arr)
{
	// Cross products of all points along the reference axis.
	cpFloat cian = cpvcross(inc.a.p, ref.n);
	cpFloat cibn = cpvcross(inc.b.p, ref.n);
	cpFloat cran = cpvcross(ref.a.p, ref.n);
	cpFloat crbn = cpvcross(ref.b.p, ref.n);
	
	cpFloat dran = cpvdot(ref.a.p, ref.n) + ref.r + inc.r;
	cpFloat dian = cpvdot(inc.a.p, ref.n) - dran;
	cpFloat dibn = cpvdot(inc.b.p, ref.n) - dran;
	
	// t-value of the incident axis endpoints projected onto the reference segment.
	cpFloat t1 = cpfclamp01((cian - cran)/(cian - cibn));
	cpFloat t2 = cpfclamp01((cibn - crbn)/(cibn - cian));
	
#if DRAW_CLIP
#if PRINT_LOG
//	cpFloat dot = 1.0 - cpfclamp01(cpfabs(cpvdot(ref.n, n)));
//	ChipmunkDemoPrintString("dot %.2e %s\n", dot, dot < 1e-5 ? "TRUE" : "FALSE");
	ChipmunkDemoPrintString("t1: %.2f, t2: %.2f, t1xt2: %.2f    %s\n", t1, t2, t1*t2, t1*t2 == 0 ? "XXXXXX" : "");
//	cpAssertWarn(t1*t2 != 0.0, "This?");
//	printf("t1*t2: %.2f\n", t1*t2);
#endif

	cpFloat d = -(ref.r + inc.r);
	ChipmunkDebugDrawSegment(ref.a.p, ref.b.p, RGBAColor(1, 0, 0, 1));
	ChipmunkDebugDrawSegment(cpvadd(inc.a.p, cpvmult(ref.n, d)), cpvadd(inc.b.p, cpvmult(ref.n, d)), RGBAColor(0, 1, 0, 1));
	
	cpVect cref = cpvlerp(ref.a.p, ref.b.p, 0.5);
	ChipmunkDebugDrawSegment(cref, cpvadd(cref, cpvmult(ref.n, 5.0)), RGBAColor(1, 0, 0, 1));
	
	cpVect cinc = cpvlerp(inc.a.p, inc.b.p, 0.5);
	ChipmunkDebugDrawSegment(cinc, cpvadd(cinc, cpvmult(inc.n, 5.0)), RGBAColor(1, 0, 0, 1));
	
	ChipmunkDebugDrawFatSegment(ref.a.p, ref.b.p, ref.r + inc.r, RGBAColor(1, 0, 0, 1), RGBAColor(0, 0, 0, 0));
	ChipmunkDebugDrawSegment(inc.a.p, inc.b.p, RGBAColor(0, 1, 0, 1));
	
	ChipmunkDebugDrawPoints(5.0, 2, (cpVect[]){ref.a.p, inc.a.p}, RGBAColor(1, 1, 0, 1));
	ChipmunkDebugDrawPoints(5.0, 2, (cpVect[]){ref.b.p, inc.b.p}, RGBAColor(0, 1, 1, 1));
#endif
	
	// If both end points are clipped, one of them always be 0.0 and the other 1.0.
	if(t1*t2 != 0){
		// One or neither endpoint is clipped. Try both.
		int count = ClipContact(cpflerp(dian, dibn, t1), t1, ref.a, inc.b, ref.r, inc.r, ref.n, n, arr);
		return count + ClipContact(cpflerp(dibn, dian, t2), t2, ref.b, inc.a, ref.r, inc.r, ref.n, n, arr + count);
	} else {
		// Collide the endpoints against each other instead.
		cpAssertSoft(t1 + t2 == 1.0, "These should sum to 1.0?");
		
		// TODO radii could use some tweaking here.
		cpFloat ndot = cpvdot(ref.n, inc.n);
		if(t1 == 0){
			struct EdgePoint refp = (ndot < 0.0f ? ref.a : ref.b);
			return circle2circleQuery(refp.p, inc.a.p, ref.r, inc.r, CP_HASH_PAIR(refp.hash, inc.b.hash), arr);
		} else {
			struct EdgePoint refp = (ndot < 0.0f ? ref.b : ref.a);
			return circle2circleQuery(refp.p, inc.b.p, ref.r, inc.r, CP_HASH_PAIR(refp.hash, inc.a.hash), arr);
		}
	}
}

static int
ContactPoints(const struct Edge e1, const struct Edge e2, const cpVect n, cpContact *arr)
{
	if(cpvdot(e1.n, n) > -cpvdot(e2.n, n)){
		return ClipContacts(e1, e2, n, arr);
	} else {
		return ClipContacts(e2, e1, n, arr);
	}
}

//MARK: Collision Functions

typedef int (*collisionFunc)(const cpShape *, const cpShape *, cpContact *);

// Collide circle shapes.
static int
circle2circle(const cpCircleShape *c1, const cpCircleShape *c2, cpContact *arr)
{
	return circle2circleQuery(c1->tc, c2->tc, c1->r, c2->r, 0, arr);
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
	
	if(circle2circleQuery(center, closest, circleShape->r, segmentShape->r, 0, con)){
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

#if USE_GJK

static int
segment2segment(const cpSegmentShape *seg1, const cpSegmentShape *seg2, cpContact *arr)
{
	struct ClosestPoints points = GJK((cpShape *)seg1, (cpShape *)seg2, (SupportFunction)cpSegmentSupportPoint, (SupportFunction)cpSegmentSupportPoint);
	
#if DRAW_CLOSEST
#if PRINT_LOG
	ChipmunkDemoPrintString("Distance: %.2f\n", points.d);
#endif
	
	ChipmunkDebugDrawPoints(6.0, 2, (cpVect[]){points.a, points.b}, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif
	
	cpVect n = points.n;
	if(points.d - (seg1->r + seg2->r) <= 0.0){
		return ContactPoints(SupportEdgeForSegment(seg1, n), SupportEdgeForSegment(seg2, cpvneg(n)), n, arr);
	} else {
		return 0;
	}
}

#else 

static int
segment2segment(const cpSegmentShape *seg1, const cpSegmentShape *seg2, cpContact *con)
{
	return 0;
}

#endif

#if USE_GJK

static int
poly2poly(const cpPolyShape *poly1, const cpPolyShape *poly2, cpContact *arr)
{
	struct ClosestPoints points = GJK((cpShape *)poly1, (cpShape *)poly2, (SupportFunction)cpPolySupportPoint, (SupportFunction)cpPolySupportPoint);
	
#if DRAW_CLOSEST
#if PRINT_LOG
	ChipmunkDemoPrintString("Distance: %.2f\n", points.d);
#endif
	
	ChipmunkDebugDrawPoints(3.0, 2, (cpVect[]){points.a, points.b}, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif
	
	if(points.d <= 0.0){
		return ContactPoints(SupportEdgeForPoly(poly1, points.n), SupportEdgeForPoly(poly2, cpvneg(points.n)), points.n, arr);
	} else {
		return 0;
	}
}

#else

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

#endif

#if USE_GJK

static int
seg2poly(const cpSegmentShape *seg, const cpPolyShape *poly, cpContact *arr)
{
	struct ClosestPoints points = GJK((cpShape *)seg, (cpShape *)poly, (SupportFunction)cpSegmentSupportPoint, (SupportFunction)cpPolySupportPoint);
	
#if DRAW_CLOSEST
#if PRINT_LOG
	ChipmunkDemoPrintString("Distance: %.2f\n", points.d);
#endif
	
	ChipmunkDebugDrawPoints(3.0, 2, (cpVect[]){points.a, points.b}, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif
	
	if(points.d - seg->r <= 0.0){
		return ContactPoints(SupportEdgeForSegment(seg, points.n), SupportEdgeForPoly(poly, cpvneg(points.n)), points.n, arr);
	} else {
		return 0;
	}
}

#else

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

#endif

// This one is less gross, but still gross.
// TODO: Comment me!
static int
circle2poly(const cpCircleShape *circle, const cpPolyShape *poly, cpContact *con)
{
	cpSplittingPlane *planes = poly->tPlanes;
	
	int numVerts = poly->numVerts;
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
	cpVect a = poly->tVerts[(mini - 1 + numVerts)%numVerts];
	cpVect b = poly->tVerts[mini];
	cpFloat dta = cpvcross(n, a);
	cpFloat dtb = cpvcross(n, b);
	cpFloat dt = cpvcross(n, circle->tc);
		
	if(dt < dtb){
		return circle2circleQuery(circle->tc, b, circle->r, 0.0f, 0, con);
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
		return circle2circleQuery(circle->tc, a, circle->r, 0.0f, 0, con);
	}
}

static const collisionFunc builtinCollisionFuncs[9] = {
	(collisionFunc)circle2circle,
	NULL,
	NULL,
	(collisionFunc)circle2segment,
	(collisionFunc)segment2segment,
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
	
	int numContacts = (cfunc? cfunc(a, b, arr) : 0);
	cpAssertHard(numContacts <= 2, "What the heck?");
	
#if DRAW_CLIP
#if PRINT_LOG
	ChipmunkDemoPrintString("Contacts: %d", numContacts);
	
	if(numContacts == 1){
		ChipmunkDemoPrintString(" %X\n", arr[0].hash);
	} else if(numContacts == 2){
		cpHashValue a = arr[0].hash;
		cpHashValue b = arr[1].hash;
		ChipmunkDemoPrintString(" %X %X\n", (a < b ? a : b), (a > b ? a : b));
	}
#endif
	
	for(int i=0; i<numContacts; i++){
		cpVect p = arr[i].p;
		ChipmunkDebugDrawPoints(5.0, 1, &p, RGBAColor(1, 0, 0, 1));
		
		cpVect n = arr[i].n;
		cpFloat d = -arr[i].dist;
		ChipmunkDebugDrawSegment(cpvadd(p, cpvmult(n, d)), cpvadd(p, cpvmult(n, -d)), RGBAColor(1, 0, 0, 1));
	}
#endif
	
	return numContacts;
}
