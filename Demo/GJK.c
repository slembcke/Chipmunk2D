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
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "chipmunk_private.h"
#include "ChipmunkDemo.h"

static cpSpace *space;
static cpShape *shape1, *shape2;

static cpVect
SupportPoint_reference(cpShape *shape, cpVect n)
{
	cpFloat max = -INFINITY;
	cpVect point = cpvzero;
	
	int count = cpPolyShapeGetNumVerts(shape);
	for(int i=0; i<count; i++){
		cpVect v = cpBodyLocal2World(shape->body, cpPolyShapeGetVert(shape, i));
		cpFloat d = cpvdot(v, n);
		if(d > max){
			max = d;
			point = v;
		}
	}
	
	return point;
}

static int
SupportPointIndex(cpShape *shape, cpVect n)
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
SupportPoint(cpShape *shape, cpVect n)
{
	cpPolyShape *poly = (cpPolyShape *)shape;
	
	cpVect point = poly->tVerts[SupportPointIndex(shape, n)];
	cpVect point2 = SupportPoint_reference(shape, n);
//	printf("point:%s, ", cpvstr(cpvsub(point, shape->body->p)));
//	printf("point2:%s ", cpvstr(cpvsub(point2, shape->body->p)));
//	printf("n:%s\n", cpvstr(n));
	cpAssertHard(cpfabs(cpvdot(point, n) - cpvdot(point2, n)) < 1e-5, "Support points not equal.");
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
SupportEdge(cpShape *shape, cpVect n)
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

struct MinkowskiPoint {
	cpVect a, b, ab;
};

static struct MinkowskiPoint
Support(cpShape *shape1, cpShape *shape2, cpVect n)
{
	cpVect a = SupportPoint(shape1, cpvneg(n));
	cpVect b = SupportPoint(shape2, n);
	
	struct MinkowskiPoint point = {a, b, cpvsub(b, a)};
	return point;
}

static cpBool
ContainsOrigin(cpVect a, cpVect b, cpVect c)
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
ClosestT(cpVect a, cpVect b)
{
	cpVect delta = cpvsub(b, a);
	return cpfclamp01(cpvdot(delta, cpvneg(a))/cpvlengthsq(delta));
}

struct ClosestPoints {
	cpVect a, b;
	cpFloat d;
};

static struct ClosestPoints
ClosestPointsNew(struct MinkowskiPoint v0, struct MinkowskiPoint v1, cpFloat t, cpFloat coef)
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
EPANodeInit(struct EPANode *node, struct MinkowskiPoint v0, struct MinkowskiPoint v1)
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
EPARecurse(cpShape *a, cpShape *b, struct EPANode *root, int i)
{
	struct EPANode *best = root->best;
	cpVect closest = best->closest;
	struct MinkowskiPoint p = Support(a, b, closest);
	
	if(cpvdot(closest, p.ab) > cpfmax(cpvdot(closest, best->v0.ab), cpvdot(closest, best->v1.ab))){
//		ChipmunkDebugDrawPolygon(3, (cpVect[]){best->v0.ab, best->v1.ab, p.ab}, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
//		ChipmunkDebugDrawPoints(3.0, 1, &closest, RGBAColor(1, 1, 0, 1));
//		ChipmunkDebugDrawSegment(closest, p.ab, RGBAColor(0, 0, 1, 1));
		
		struct EPANode left; EPANodeInit(&left, best->v0, p);
		struct EPANode right; EPANodeInit(&right, p, best->v1);
		EPANodeSplit(best, &left, &right);
		
		return EPARecurse(a, b, root, i+1);
	} else {
		ChipmunkDemoPrintString("EPA iterations: %d\n", i+1);
		return ClosestPointsNew(best->v0, best->v1, best->t, -1.0f);
	}
}

static struct ClosestPoints
EPA(cpShape *a, cpShape *b, struct MinkowskiPoint v0, struct MinkowskiPoint v1, struct MinkowskiPoint v2)
{
//	ChipmunkDebugDrawPolygon(3, (cpVect[]){v0.ab, v1.ab, v2.ab}, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
		
	struct EPANode n01; EPANodeInit(&n01, v0, v1);
	struct EPANode n12; EPANodeInit(&n12, v1, v2);
	struct EPANode n20; EPANodeInit(&n20, v2, v0);
	
	struct EPANode inner, root = {};
	EPANodeSplit(&inner, &n01, &n12);
	EPANodeSplit(&root, &inner, &n20);
	
	return EPARecurse(a, b, &root, 0);
}

static struct ClosestPoints
GJKRecurse(cpShape *a, cpShape *b, struct MinkowskiPoint v0, struct MinkowskiPoint v1, int i)
{
	cpFloat t = ClosestT(v0.ab, v1.ab);
	cpVect closest = cpvlerp(v0.ab, v1.ab, t);
	struct MinkowskiPoint p = Support(a, b, cpvneg(closest));
	
//	ChipmunkDebugDrawSegment(v0.ab, v1.ab, RGBAColor(1, 1, 1, 1));
//	ChipmunkDebugDrawPoints(3.0, 1, &closest, RGBAColor(1, 1, 1, 1));
//	ChipmunkDebugDrawSegment(closest, p.ab, RGBAColor(0, 1, 0, 1));
	
	cpFloat d1 = cpvdot(closest, p.ab);
	if(d1 <= 0.0 && ContainsOrigin(v0.ab, v1.ab, p.ab)){
		ChipmunkDemoPrintString("GJK iterations: %d ", i+1);
		return EPA(a, b, v0, v1, p);
	} else if(d1 < cpfmin(cpvdot(closest, v0.ab), cpvdot(closest, v1.ab))){
		if(cpvlengthsq(v0.ab) < cpvlengthsq(v1.ab)){
			return GJKRecurse(a, b, v0, p, i+1);
		} else {
			return GJKRecurse(a, b, p, v1, i+1);
		}
	} else {
		ChipmunkDemoPrintString("GJK iterations: %d\n", i+1);
		return ClosestPointsNew(v0, v1, t, 1.0);
	}
}

static struct ClosestPoints
ClosestPoints(cpShape *a, cpShape *b)
{
	cpVect axis = cpvperp(cpvsub(a->body->p, b->body->p));
	return GJKRecurse(a, b, Support(a, b, axis), Support(a, b, cpvneg(axis)), 0);
}

static void
ClipContacts(struct Edge ref, struct Edge inc, cpFloat flipped)
{
	cpVect midref = cpvlerp(ref.a.v, ref.b.v, 0.5);
	ChipmunkDebugDrawSegment(ref.a.v, ref.b.v, RGBAColor(1, 0, 0, 1));
	ChipmunkDebugDrawSegment(midref, cpvadd(midref, cpvmult(ref.n, 10.0)), RGBAColor(1, 0, 0, 1));
	
	cpVect midinc = cpvlerp(inc.a.v, inc.b.v, 0.5);
	ChipmunkDebugDrawSegment(inc.a.v, inc.b.v, RGBAColor(0, 1, 0, 1));
	ChipmunkDebugDrawSegment(midinc, cpvadd(midinc, cpvmult(inc.n, 10.0)), RGBAColor(0, 1, 0, 1));
	
	ChipmunkDebugDrawPoints(5.0, 1, &ref.a.v, RGBAColor(1, 1, 0, 1));
	ChipmunkDebugDrawPoints(5.0, 1, &ref.b.v, RGBAColor(0, 1, 1, 1));
	
	ChipmunkDebugDrawPoints(5.0, 1, &inc.a.v, RGBAColor(1, 1, 0, 1));
	ChipmunkDebugDrawPoints(5.0, 1, &inc.b.v, RGBAColor(0, 1, 1, 1));
	
	cpFloat cian = cpvcross(inc.a.v, ref.n);
	cpFloat cibn = cpvcross(inc.b.v, ref.n);
	cpFloat cran = cpvcross(ref.a.v, ref.n);
	cpFloat crbn = cpvcross(ref.b.v, ref.n);
	
	cpFloat dran = cpvdot(ref.a.v, ref.n);
	cpFloat dian = cpvdot(inc.a.v, ref.n) - dran;
	cpFloat dibn = cpvdot(inc.b.v, ref.n) - dran;
	
	cpFloat t1 = cpfclamp01((cian - cran)/(cian - cibn));
	cpFloat d1 = cpflerp(dian, dibn, t1);
	if(d1 < 0.0){
		struct EdgePoint point = {t1 < 1.0 ? ref.a.v : inc.b.v, CP_HASH_PAIR(ref.a.hash, inc.b.hash)};
		ChipmunkDebugDrawPoints(5.0, 1, &point.v, RGBAColor(1, 0, 1, 1));
		ChipmunkDemoPrintString("1:{depth:%.2f, hash:%X}, ", d1, point.hash);
	}
	
	cpFloat t2 = cpfclamp01((cibn - crbn)/(cibn - cian));
	cpFloat d2 = cpflerp(dibn, dian, t2);
	if(d2 < 0.0){
		struct EdgePoint point = {t2 < 1.0 ? ref.b.v : inc.a.v, CP_HASH_PAIR(ref.b.hash, inc.a.hash)};
		ChipmunkDebugDrawPoints(5.0, 1, &point.v, RGBAColor(1, 0, 1, 1));
		ChipmunkDemoPrintString("2:{depth:%.2f, hash:%X}\n", d2, point.hash);
	}
}

static void
ContactPoints(cpShape *a, cpShape *b, struct ClosestPoints points)
{
	if(points.d > 0.0) return;
	
	cpVect n = cpvmult(cpvsub(points.b, points.a), 1.0f/points.d);
	struct Edge f1 = SupportEdge(shape1, n);
	struct Edge f2 = SupportEdge(shape2, cpvneg(n));
	
	if(cpvdot(f1.n, n) > -cpvdot(f2.n, n)){
		ClipContacts(f1, f2, 1.0);
	} else {
		ClipContacts(f2, f1, -1.0);
	}
}

static void
update(int ticks)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static void
draw(void)
{
	ChipmunkDemoDefaultDrawImpl();
	
//	// draw the minkowski difference origin
//	cpVect origin = cpvzero;
//	ChipmunkDebugDrawPoints(5.0, 1, &origin, RGBAColor(1,0,0,1));
//	
//	// draw the minkowski difference
//	int shape1Count = cpPolyShapeGetNumVerts(shape1);
//	int shape2Count = cpPolyShapeGetNumVerts(shape2);
//	
//	int mdiffCount = shape1Count*shape2Count;
//	cpVect *mdiffVerts = alloca(mdiffCount*sizeof(cpVect));
//	
//	for(int i=0; i<shape1Count; i++){
//		for(int j=0; j<shape2Count; j++){
//			cpVect v1 = cpBodyLocal2World(shape1->body, cpPolyShapeGetVert(shape1, i));
//			cpVect v2 = cpBodyLocal2World(shape2->body, cpPolyShapeGetVert(shape2, j));
//			mdiffVerts[i*shape2Count + j] = cpvsub(v2, v1);
//		}
//	}
//	
//	cpVect *hullVerts = alloca(mdiffCount*sizeof(cpVect));
//	int hullCount = cpConvexHull(mdiffCount, mdiffVerts, hullVerts, NULL, 0.0);
//	
//	ChipmunkDebugDrawPolygon(hullCount, hullVerts, RGBAColor(1, 0, 0, 1), RGBAColor(1, 0, 0, 0.25));
//	ChipmunkDebugDrawPoints(2.0, mdiffCount, mdiffVerts, RGBAColor(1, 0, 0, 1));
	
	struct ClosestPoints pair = ClosestPoints(shape1, shape2);
	cpVect points[] = {pair.a, pair.b};
	ChipmunkDebugDrawPoints(3.0, 2, points, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points[0], points[1], RGBAColor(1, 1, 1, 1));
	ChipmunkDemoPrintString("Distance: %.2f\n", pair.d);
	
//	if(pair.d < 0.0)
	ContactPoints(shape1, shape2, pair);
}

static cpSpace *
init(void)
{
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 5);
	space->damping = 0.1;
	
	{
		cpFloat mass = 1.0f;
		cpFloat size = 100.0f;
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, size, size)));
		cpBodySetPos(body, cpv(50.0f, 0.0f));
		
		shape1 = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size));
		shape1->group = 1;
	}
	
//	{
//		cpFloat mass = 1.0f;
//		const int NUM_VERTS = 4;
//		
//		cpVect verts[NUM_VERTS];
//		for(int i=0; i<NUM_VERTS; i++){
//			cpFloat radius = 40.0;
//			cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
//			verts[i] = cpv(radius*cos(angle), radius*sin(angle));
//		}
//		
//		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, NUM_VERTS, verts, cpvzero)));
//		cpBodySetPos(body, cpv(50.0f, 0.0f));
//		
//		shape1 = cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpvzero));
//		shape1->group = 1;
//	}
	
	{
		cpFloat mass = 1.0f;
		const int NUM_VERTS = 5;
		
		cpVect verts[NUM_VERTS];
		for(int i=0; i<NUM_VERTS; i++){
			cpFloat radius = 60.0;
			cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
			verts[i] = cpv(radius*cos(angle), radius*sin(angle));
		}
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, NUM_VERTS, verts, cpvzero)));
		cpBodySetPos(body, cpv(-50.0f, 0.0f));
		
		shape2 = cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpvzero));
		shape2->group = 1;
	}
	
//	cpBodySetAngle(shape1->body, 34.48);
//	cpShapeCacheBB(shape1);
//	int num = 40;
//	for(int i=0; i<num; i++){
//		SupportPoint(shape2, cpvforangle((cpFloat)i/(cpFloat)num*2.0*M_PI));
//	}
//	abort();
	
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo GJK = {
	"GJK",
	init,
	update,
	draw,
	destroy,
};
