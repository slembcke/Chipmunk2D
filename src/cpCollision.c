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

#include <stdio.h>
#include <string.h>

#include "chipmunk_private.h"
#include "ChipmunkDemo.h"

#if DEBUG
#define DRAW_ALL 0
#define DRAW_GJK (0 || DRAW_ALL)
#define DRAW_EPA (0 || DRAW_ALL)
#define DRAW_CLOSEST (0 || DRAW_ALL)
#define DRAW_CLIP (0 || DRAW_ALL)

#define PRINT_LOG 0
#endif

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

struct SupportPoint {
	cpVect p;
	cpCollisionID id;
};

static inline struct SupportPoint
SupportPointNew(cpVect p, cpCollisionID id)
{
	struct SupportPoint point = {p, id};
	return point;
}

static struct SupportPoint
cpPolySupportPoint(const cpPolyShape *poly, const cpVect n)
{
	int i = cpSupportPointIndex(poly, n);
	return SupportPointNew(poly->tVerts[i], i);
}

static struct SupportPoint
cpSegmentSupportPoint(const cpSegmentShape *seg, const cpVect n)
{
	cpVect a = seg->ta, b = seg->tb;
	if(cpvdot(a, n) > cpvdot(b, n)){
		return SupportPointNew(a, 0);
	} else {
		return SupportPointNew(b, 1);
	}
}

typedef struct SupportPoint (*SupportFunction)(const cpShape *a, cpVect n);

struct MinkowskiPoint {
	cpVect a, b;
	cpVect ab;
	cpCollisionID id;
};

static inline struct MinkowskiPoint
MinkoskiPointNew(const struct SupportPoint a, const struct SupportPoint b)
{
	struct MinkowskiPoint point = {a.p, b.p, cpvsub(b.p, a.p), (a.id & 0xFF)<<8 | (b.id & 0xFF)};
	return point;
}

struct SupportContext {
	const cpShape *shape1, *shape2;
	SupportFunction support1, support2;
};

static inline struct MinkowskiPoint
Support(const struct SupportContext context, const cpVect n)
{
	struct SupportPoint a = context.support1(context.shape1, cpvneg(n));
	struct SupportPoint b = context.support2(context.shape2, n);
	return MinkoskiPointNew(a, b);
}

struct EdgePoint {
	cpVect p;
	cpHashValue hash;
};

struct Edge {
	struct EdgePoint a, b;
	cpFloat r;
	cpVect n;
};

static inline struct Edge
EdgeNew(cpVect va, cpVect vb, cpHashValue ha, cpHashValue hb, cpFloat r)
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
		return (struct Edge){{verts[i0], CP_HASH_PAIR(poly, i0)}, {verts[i1], CP_HASH_PAIR(poly, i1)}, poly->r, poly->tPlanes[i1].n};
	} else {
		return (struct Edge){{verts[i1], CP_HASH_PAIR(poly, i1)}, {verts[i2], CP_HASH_PAIR(poly, i2)}, poly->r, poly->tPlanes[i2].n};
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

static inline cpFloat
ClosestT(const cpVect a, const cpVect b)
{
	cpVect delta = cpvsub(b, a);
	return cpfclamp01(cpvdot(delta, cpvneg(a))/cpvlengthsq(delta));
}

struct ClosestPoints {
	cpVect a, b;
	cpVect n;
	cpFloat d;
	cpCollisionID id;
};

static inline struct ClosestPoints
ClosestPointsNew(const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, cpFloat t, cpVect p)
{
	cpVect pa = cpvlerp(v0.a, v1.a, t);
	cpVect pb = cpvlerp(v0.b, v1.b, t);
	cpCollisionID id = (v0.id & 0xFFFF)<<16 | (v1.id & 0xFFFF);
	
	cpVect delta = cpvsub(v1.ab, v0.ab);
	cpVect n = cpvnormalize(cpvperp(delta));
	cpFloat d = -cpvdot(n, p);
	
	if(d <= 0.0f || (0.0f < t && t < 1.0f)){
		struct ClosestPoints points = (struct ClosestPoints){pa, pb, cpvneg(n), d, id};
//		if(n.x != n.x || n.y != n.y){
//			printf("crap1\n");
//		}
		return points;
	} else {
		cpFloat d2 = cpvlength(p);
		cpVect n = cpvmult(p, 1.0f/(d2 + CPFLOAT_MIN));
		struct ClosestPoints points = (struct ClosestPoints){pa, pb, n, d2, id};
//		if(n.x != n.x || n.y != n.y || d2 == 0.0){
//			printf("crap2\n");
//		}
		return points;
	}
}

//MARK: EPA Functions

struct EPANode {
	struct MinkowskiPoint v0, v1;
	struct EPANode *best, *left, *right, *parent;
	cpVect closest;
	cpFloat t, dist;
};

static inline void
EPANodeInit(struct EPANode *node, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1)
{
	cpFloat t = ClosestT(v0.ab, v1.ab);
	cpVect closest = cpvlerp(v0.ab, v1.ab, t);
	
	node->v0 = v0;
	node->v1 = v1;
	node->best = node;
	node->closest = closest;
	node->t = t;
	node->dist = cpvlengthsq(closest);
}

static inline void
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

static inline cpFloat
ClosestDist(cpVect v0, cpVect v1)
{
	return cpvlengthsq(cpvlerp(v0, v1, ClosestT(v0, v1)));
}

static struct ClosestPoints
EPARecurse(const struct SupportContext context, struct EPANode *root, int i)
{
	cpAssertSoft(i < 100, "Stuck in EPA recursion.");
	
	struct EPANode *best = root->best;
	struct MinkowskiPoint p = Support(context, best->closest);
	
	struct MinkowskiPoint v0 = best->v0;
	struct MinkowskiPoint v1 = best->v1;
	
#if DRAW_EPA
	ChipmunkDebugDrawPolygon(3, (cpVect[]){v0.ab, v1.ab, p.ab}, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
	ChipmunkDebugDrawPoints(3.0, 1, &closest, RGBAColor(1, 1, 0, 1));
	ChipmunkDebugDrawSegment(closest, p.ab, RGBAColor(0, 0, 1, 1));
#endif
	
	cpFloat area = cpvcross(cpvsub(v1.ab, v0.ab), cpvsub(p.ab, v0.ab));
	if(area > 0.0f){
		struct EPANode left; EPANodeInit(&left, v0, p);
		struct EPANode right; EPANodeInit(&right, p, v1);
		EPANodeSplit(best, &left, &right);
		
		return EPARecurse(context, root, i + 1);
	} else {
//		ChipmunkDebugDrawSegment(cpBBCenter(context.shape1->bb), cpBBCenter(context.shape2->bb), RGBAColor(0, 1, 0, 0.5));
		return ClosestPointsNew(v0, v1, best->t, best->closest);
	}
}

static struct ClosestPoints
EPA(const struct SupportContext context, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, const struct MinkowskiPoint v2)
{
#if DRAW_EPA || DRAW_GJK
	ChipmunkDebugDrawPolygon(3, (cpVect[]){v0.ab, v1.ab, v2.ab}, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
#endif
	
//	cpFloat area = cpvcross(cpvsub(v1.ab, v0.ab), cpvsub(v2.ab, v0.ab));
//	if(area == 0.0){
//		printf("bad\n");
//	}
	
	struct EPANode n01, n12, n20;
	EPANodeInit(&n01, v0, v1);
	EPANodeInit(&n12, v1, v2);
	EPANodeInit(&n20, v2, v0);
	
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
	for(int i=0;; i++){
		cpAssertSoft(i < 100, "Stuck in GJK recursion.");
		
		cpVect delta = cpvsub(v1.ab, v0.ab);
		if(cpvcross(delta, cpvneg(v0.ab)) < 0.0f){
			// Origin is behind axis. Flip and try again.
			struct MinkowskiPoint tmp = v0;
			v0 = v1; v1 = tmp;
		} else {
			cpVect n = cpvperp(delta);
			struct MinkowskiPoint p = Support(context, n);
			
#if DRAW_GJK
//			ChipmunkDebugDrawSegment(v0.ab, v1.ab, RGBAColor(1, 1, 1, 1));
//			cpVect c = cpvlerp(v0.ab, v1.ab, 0.5);
//			ChipmunkDebugDrawSegment(c, cpvadd(c, cpvmult(cpvnormalize(n), 5.0)), RGBAColor(1, 0, 0, 1));
//			
//			ChipmunkDebugDrawPoints(5.0, 1, &p.ab, RGBAColor(1, 1, 1, 1));
#endif
			
			if(cpvcross(delta, cpvsub(p.ab, v0.ab)) <= 0.0f){
//				ChipmunkDebugDrawSegment(cpBBCenter(context.shape1->bb), cpBBCenter(context.shape2->bb), RGBAColor(1, 0, 0, 0.5));
				cpFloat t = ClosestT(v0.ab, v1.ab);
				return ClosestPointsNew(v0, v1, t, cpvlerp(v0.ab, v1.ab, t));
			} else if(
				cpvcross(cpvsub(v1.ab, p.ab), cpvneg(p.ab)) <= 0.0f &&
				cpvcross(cpvsub(v0.ab, p.ab), cpvneg(p.ab)) >= 0.0f
			){
				// The triangle v0, v1, p contains the origin. Use EPA to find the MSA.
				return EPA(context, v0, p, v1);
			} else {
				if(cpvdot(p.ab, delta) > 0.0){
					v1 = p;
				} else {
					v0 = p;
				}
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

#endif

static struct SupportPoint
ShapePoint(const cpShape *shape, int i)
{
	switch(shape->klass->type){
		default: return SupportPointNew(cpvzero, i);
		case CP_CIRCLE_SHAPE: return SupportPointNew(((cpCircleShape *)shape)->tc, i);
		case CP_SEGMENT_SHAPE: {
			cpSegmentShape *seg = (void *)shape;
			return SupportPointNew(i == 0 ? seg->ta : seg->tb, i);
		};
		case CP_POLY_SHAPE: return SupportPointNew(((cpPolyShape *)shape)->tVerts[i], i);
	}
}

static struct ClosestPoints
GJK(const cpShape *shape1, const cpShape *shape2, SupportFunction support1, SupportFunction support2, cpCollisionID *id)
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
			cpVect v1 = ShapePoint(shape1, i).p;
			cpVect v2 = ShapePoint(shape2, j).p;
			mdiffVerts[i*shape2Count + j] = cpvsub(v2, v1);
		}
	}
	 
	cpVect *hullVerts = alloca(mdiffCount*sizeof(cpVect));
	int hullCount = cpConvexHull(mdiffCount, mdiffVerts, hullVerts, NULL, 0.0);
	
	ChipmunkDebugDrawPolygon(hullCount, hullVerts, RGBAColor(1, 0, 0, 1), RGBAColor(1, 0, 0, 0.25));
	ChipmunkDebugDrawPoints(2.0, mdiffCount, mdiffVerts, RGBAColor(1, 0, 0, 1));
#endif
	
	struct SupportContext context = {shape1, shape2, support1, support2};
	
	struct MinkowskiPoint v0, v1;
	if(*id){
		v0 = MinkoskiPointNew(ShapePoint(shape1, (*id>>24)&0xFF), ShapePoint(shape2, (*id>>16)&0xFF));
		v1 = MinkoskiPointNew(ShapePoint(shape1, (*id>> 8)&0xFF), ShapePoint(shape2, (*id    )&0xFF));
	} else {
		// TODO use centroids as the starting axis
		cpVect axis = cpvperp(cpvsub(cpBBCenter(shape1->bb), cpBBCenter(shape2->bb)));
		v0 = Support(context, axis);
		v1 = Support(context, cpvneg(axis));
	}
	
	struct ClosestPoints points = GJKRecurse(context, v0, v1);
	*id = points.id;
	return points;
}

//MARK: Contact Clipping

static inline void
Contact1(cpFloat dist, cpVect a, cpVect b, cpFloat refr, cpFloat incr, cpVect n, cpHashValue hash, cpContact *arr)
{
	cpFloat rsum = refr + incr;
	cpFloat alpha = (rsum > 0.0f ? refr/rsum : 0.5f);
	cpVect point = cpvlerp(a, b, alpha);
	
	cpContactInit(arr, point, n, dist - rsum, hash);
}

static inline int
Contact2(cpVect refp, cpVect inca, cpVect incb, cpFloat refr, cpFloat incr, cpVect refn, cpVect n, cpHashValue hash, cpContact *arr)
{
	cpFloat cian = cpvcross(inca, refn);
	cpFloat cibn = cpvcross(incb, refn);
	cpFloat crpn = cpvcross(refp, refn);
	cpFloat t = 1.0f - cpfclamp01((cibn - crpn)/(cibn - cian));
	
	cpVect point = cpvlerp(inca, incb, t);
	cpFloat pd = cpvdot(cpvsub(point, refp), refn);
	
	if(t > 0.0f && pd <= 0.0f){
		cpFloat rsum = refr + incr;
		cpFloat alpha = (rsum > 0.0f ? incr*(1.0f - (rsum + pd)/rsum) : -0.5f*pd);
		
		cpContactInit(arr, cpvadd(point, cpvmult(refn, alpha)), n, pd, hash);
		return 1;
	} else {
		return 0;
	}
}

static inline int
ClipContacts(const struct Edge ref, const struct Edge inc, const struct ClosestPoints points, const cpFloat nflip, cpContact *arr)
{
	cpVect inc_offs = cpvmult(inc.n, inc.r);
	cpVect ref_offs = cpvmult(ref.n, ref.r);
	
	cpVect inca = cpvadd(inc.a.p, inc_offs);
	cpVect incb = cpvadd(inc.b.p, inc_offs);
	
	cpVect closest_inca = cpClosetPointOnSegment(inc.a.p, ref.a.p, ref.b.p);
	cpVect closest_incb = cpClosetPointOnSegment(inc.b.p, ref.a.p, ref.b.p);
	
	cpVect msa = cpvmult(points.n, nflip*points.d);
	cpFloat cost_a = cpvdistsq(cpvsub(inc.a.p, closest_inca), msa);
	cpFloat cost_b = cpvdistsq(cpvsub(inc.b.p, closest_incb), msa);
	
#if DRAW_CLIP
	ChipmunkDebugDrawSegment(ref.a.p, ref.b.p, RGBAColor(1, 0, 0, 1));
	ChipmunkDebugDrawSegment(inc.a.p, inc.b.p, RGBAColor(0, 1, 0, 1));
	ChipmunkDebugDrawSegment(inca, incb, RGBAColor(0, 1, 0, 1));
	
	cpVect cref = cpvlerp(ref.a.p, ref.b.p, 0.5);
	ChipmunkDebugDrawSegment(cref, cpvadd(cref, cpvmult(ref.n, 5.0)), RGBAColor(1, 0, 0, 1));
	
	cpVect cinc = cpvlerp(inc.a.p, inc.b.p, 0.5);
	ChipmunkDebugDrawSegment(cinc, cpvadd(cinc, cpvmult(inc.n, 5.0)), RGBAColor(1, 0, 0, 1));
	
	ChipmunkDebugDrawPoints(5.0, 2, (cpVect[]){ref.a.p, inc.a.p}, RGBAColor(1, 1, 0, 1));
	ChipmunkDebugDrawPoints(5.0, 2, (cpVect[]){ref.b.p, inc.b.p}, RGBAColor(0, 1, 1, 1));
	
//	ChipmunkDemoPrintString("cost_a: %5.2f, cost_b: %5.2f\n", cost_a, cost_b);
	if(cost_a < cost_b){
		ChipmunkDebugDrawSegment(closest_inca, inc.a.p, RGBAColor(1, 0, 1, 1));
	} else {
		ChipmunkDebugDrawSegment(closest_incb, inc.b.p, RGBAColor(1, 0, 1, 1));
	}
#endif
	
	cpHashValue hash_iarb = CP_HASH_PAIR(inc.a.hash, ref.b.hash);
	cpHashValue hash_ibra = CP_HASH_PAIR(inc.b.hash, ref.a.hash);
	
	if(cost_a < cost_b){
		cpVect refp = cpvadd(ref.a.p, ref_offs);
		Contact1(points.d, closest_inca, inc.a.p, ref.r, inc.r, points.n, hash_iarb, arr);
		return Contact2(refp, inca, incb, ref.r, inc.r, ref.n, points.n, hash_ibra, arr + 1) + 1;
	} else {
		cpVect refp = cpvadd(ref.b.p, ref_offs);
		Contact1(points.d, closest_incb, inc.b.p, ref.r, inc.r, points.n, hash_ibra, arr);
		return Contact2(refp, incb, inca, ref.r, inc.r, ref.n, points.n, hash_iarb, arr + 1) + 1;
	}
}

static inline int
ContactPoints(const struct Edge e1, const struct Edge e2, const struct ClosestPoints points, cpContact *arr)
{
	cpFloat mindist = e1.r + e2.r;
	if(points.d <= mindist){
		cpFloat pick = cpvdot(e1.n, points.n) + cpvdot(e2.n, points.n);
		
		if(
			(pick != 0.0f && pick > 0.0f) ||
			// If the edges are both perfectly aligned weird things happen.
			// This is *very* common at the start of a simulation.
			// Pick the longest edge as the reference to break the tie.
			(pick == 0.0f && (cpvdistsq(e1.a.p, e1.b.p) > cpvdistsq(e2.a.p, e2.b.p)))
		){
			return ClipContacts(e1, e2, points,  1.0f, arr);
		} else {
			return ClipContacts(e2, e1, points, -1.0f, arr);
		}
	} else {
		return 0;
	}
}

//MARK: Collision Functions

typedef int (*CollisionFunc)(const cpShape *a, const cpShape *b, cpCollisionID *id, cpContact *arr);

// Collide circle shapes.
static int
circle2circle(const cpCircleShape *c1, const cpCircleShape *c2, cpCollisionID *id, cpContact *arr)
{
	return circle2circleQuery(c1->tc, c2->tc, c1->r, c2->r, 0, arr);
}

static int
circle2segment(const cpCircleShape *circleShape, const cpSegmentShape *segmentShape, cpCollisionID *id, cpContact *con)
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
			(closest_t != 0.0f || cpvdot(n, cpvrotate(segmentShape->a_tangent, segmentShape->shape.body->rot)) >= 0.0) &&
			(closest_t != 1.0f || cpvdot(n, cpvrotate(segmentShape->b_tangent, segmentShape->shape.body->rot)) >= 0.0)
		){
			return 1;
		}
	}
	
	return 0;
}

static int
segment2segment(const cpSegmentShape *seg1, const cpSegmentShape *seg2, cpCollisionID *id, cpContact *arr)
{
	struct ClosestPoints points = GJK((cpShape *)seg1, (cpShape *)seg2, (SupportFunction)cpSegmentSupportPoint, (SupportFunction)cpSegmentSupportPoint, id);
	
#if DRAW_CLOSEST
#if PRINT_LOG
//	ChipmunkDemoPrintString("Distance: %.2f\n", points.d);
#endif
	
	ChipmunkDebugDrawPoints(6.0, 2, (cpVect[]){points.a, points.b}, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif
	
	cpVect n = points.n;
	if(
		points.d <= (seg1->r + seg2->r) //&&
//		(
//			(!cpveql(points.a, seg1->ta) || cpvdot(n, cpvrotate(seg1->a_tangent, seg1->shape.body->rot)) <= 0.0) &&
//			(!cpveql(points.a, seg1->tb) || cpvdot(n, cpvrotate(seg1->b_tangent, seg1->shape.body->rot)) <= 0.0) &&
//			(!cpveql(points.b, seg2->ta) || cpvdot(n, cpvrotate(seg2->a_tangent, seg2->shape.body->rot)) >= 0.0) &&
//			(!cpveql(points.b, seg2->tb) || cpvdot(n, cpvrotate(seg2->b_tangent, seg2->shape.body->rot)) >= 0.0)
//		)
	){
		return ContactPoints(SupportEdgeForSegment(seg1, n), SupportEdgeForSegment(seg2, cpvneg(n)), points, arr);
	} else {
		return 0;
	}
}

static int
poly2poly(const cpPolyShape *poly1, const cpPolyShape *poly2, cpCollisionID *id, cpContact *arr)
{
	struct ClosestPoints points = GJK((cpShape *)poly1, (cpShape *)poly2, (SupportFunction)cpPolySupportPoint, (SupportFunction)cpPolySupportPoint, id);
	
#if DRAW_CLOSEST
#if PRINT_LOG
//	ChipmunkDemoPrintString("Distance: %.2f\n", points.d);
#endif
	
	ChipmunkDebugDrawPoints(3.0, 2, (cpVect[]){points.a, points.b}, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif
	
	if(points.d - poly1->r - poly2->r <= 0.0){
		return ContactPoints(SupportEdgeForPoly(poly1, points.n), SupportEdgeForPoly(poly2, cpvneg(points.n)), points, arr);
	} else {
		return 0;
	}
}

static int
seg2poly(const cpSegmentShape *seg, const cpPolyShape *poly, cpCollisionID *id, cpContact *arr)
{
	struct ClosestPoints points = GJK((cpShape *)seg, (cpShape *)poly, (SupportFunction)cpSegmentSupportPoint, (SupportFunction)cpPolySupportPoint, id);
	
#if DRAW_CLOSEST
#if PRINT_LOG
//	ChipmunkDemoPrintString("Distance: %.2f\n", points.d);
#endif
	
	ChipmunkDebugDrawPoints(3.0, 2, (cpVect[]){points.a, points.b}, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif
	
	// Reject endcap collisions if tangents are provided.
	cpVect n = points.n;
	if(
		points.d - seg->r - poly->r <= 0.0 //&&
//		(!cpveql(points.a, seg->ta) || cpvdot(n, cpvrotate(seg->a_tangent, seg->shape.body->rot)) >= 0.0) &&
//		(!cpveql(points.a, seg->tb) || cpvdot(n, cpvrotate(seg->b_tangent, seg->shape.body->rot)) >= 0.0)
	){
		return ContactPoints(SupportEdgeForSegment(seg, n), SupportEdgeForPoly(poly, cpvneg(n)), points, arr);
	} else {
		return 0;
	}
}

// This one is less gross, but still gross.
// TODO: Comment me!
// TODO respect poly radius
static int
circle2poly(const cpCircleShape *circle, const cpPolyShape *poly, cpCollisionID *id, cpContact *con)
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
		return circle2circleQuery(circle->tc, b, circle->r, poly->r, 0, con);
	} else if(dt < dta) {
		cpVect point = cpvsub(circle->tc, cpvmult(n, circle->r + min/2.0f));
		cpContactInit(con, point, cpvneg(n), min, 0);
	
		return 1;
	} else {
		return circle2circleQuery(circle->tc, a, circle->r, poly->r, 0, con);
	}
}

static const CollisionFunc builtinCollisionFuncs[9] = {
	(CollisionFunc)circle2circle,
	NULL,
	NULL,
	(CollisionFunc)circle2segment,
	NULL,
	NULL,
	(CollisionFunc)circle2poly,
	(CollisionFunc)seg2poly,
	(CollisionFunc)poly2poly,
};
static const CollisionFunc *colfuncs = builtinCollisionFuncs;

static const CollisionFunc segmentCollisions[9] = {
	(CollisionFunc)circle2circle,
	NULL,
	NULL,
	(CollisionFunc)circle2segment,
	(CollisionFunc)segment2segment,
	NULL,
	(CollisionFunc)circle2poly,
	(CollisionFunc)seg2poly,
	(CollisionFunc)poly2poly,
};

void
cpEnableSegmentToSegmentCollisions(void)
{
	colfuncs = segmentCollisions;
}

int
cpCollideShapes(const cpShape *a, const cpShape *b, cpCollisionID *id, cpContact *arr)
{
	// Their shape types must be in order.
	cpAssertSoft(a->klass->type <= b->klass->type, "Internal Error: Collision shapes passed to cpCollideShapes() are not sorted.");
	
	CollisionFunc cfunc = colfuncs[a->klass->type + b->klass->type*CP_NUM_SHAPES];
	
	int numContacts = (cfunc? cfunc(a, b, id, arr) : 0);
	cpAssertSoft(numContacts <= CP_MAX_CONTACTS_PER_ARBITER, "Internal error: Too many contact points returned.");
	
	return numContacts;
}
