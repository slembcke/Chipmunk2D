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
SupportPoint(cpShape *shape, cpVect n)
{
	cpFloat max = -INFINITY;
	cpVect point = cpvzero;
	
	int count = cpPolyShapeGetNumVerts(shape);
	for(int i=0; i<count; i++){
		cpVect v = cpBodyLocal2World(shape->body, cpPolyShapeGetVert(shape, i));;
		cpFloat d = cpvdot(v, n);
		if(d > max){
			max = d;
			point = v;
		}
	}
	
	return point;
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
ClosestT(cpVect a, cpVect b, cpVect p)
{
	cpVect delta = cpvsub(b, a);
	return cpfclamp01(cpvdot(delta, cpvsub(p, a))/cpvlengthsq(delta));
}

struct ClosestPoints {
	cpVect a, b;
	cpFloat d;
};

static struct ClosestPoints
EPARecurse(cpShape *a, cpShape *b)
{
	struct ClosestPoints points = {a->body->p, b->body->p, cpvdist(a->body->p, b->body->p)};
	return points;
}

static struct ClosestPoints
GJKRecurse(cpShape *a, cpShape *b, struct MinkowskiPoint v0, struct MinkowskiPoint v1)
{
	cpFloat t = ClosestT(v0.ab, v1.ab, cpvzero);
	cpVect closest = cpvlerp(v0.ab, v1.ab, t);
	struct MinkowskiPoint p = Support(a, b, cpvneg(closest));
	
//	ChipmunkDebugDrawPolygon(3, (cpVect[]){v0.ab, v1.ab, p.ab}, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
//	ChipmunkDebugDrawSegment(v0.ab, v1.ab, RGBAColor(1, 1, 1, 1));
//	ChipmunkDebugDrawPoints(3.0, 1, &closest, RGBAColor(1, 1, 1, 1));
//	ChipmunkDebugDrawSegment(closest, p.ab, RGBAColor(0, 1, 0, 1));
	
	cpFloat d1 = cpvdot(closest, p.ab);
	if(d1 <= 0.0 && ContainsOrigin(v0.ab, v1.ab, p.ab)){
		return EPARecurse(a, b);
	} else if(d1 < cpfmin(cpvdot(closest, v0.ab), cpvdot(closest, v1.ab))){
		if(cpvlengthsq(v0.ab) < cpvlengthsq(v1.ab)){
			return GJKRecurse(a, b, v0, p);
		} else {
			return GJKRecurse(a, b, p, v1);
		}
	} else {
		cpVect pa = cpvlerp(v0.a, v1.a, t);
		cpVect pb = cpvlerp(v0.b, v1.b, t);
		
		struct ClosestPoints points = {pa, pb, cpvdist(pa, pb)};
		return points;
	}
}

static struct ClosestPoints
ClosestPoints(cpShape *a, cpShape *b)
{
	cpVect axis = cpvperp(cpvsub(a->body->p, b->body->p));
	return GJKRecurse(a, b, Support(a, b, axis), Support(a, b, cpvneg(axis)));
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
	
	// draw the minkowski difference origin
	cpVect origin = cpvzero;
	ChipmunkDebugDrawPoints(5.0, 1, &origin, RGBAColor(1,0,0,1));
	
	// draw the minkowski difference
	int shape1Count = cpPolyShapeGetNumVerts(shape1);
	int shape2Count = cpPolyShapeGetNumVerts(shape2);
	
	int mdiffCount = shape1Count*shape2Count;
	cpVect *mdiffVerts = alloca(mdiffCount*sizeof(cpVect));
	
	for(int i=0; i<shape1Count; i++){
		for(int j=0; j<shape2Count; j++){
			cpVect v1 = cpBodyLocal2World(shape1->body, cpPolyShapeGetVert(shape1, i));
			cpVect v2 = cpBodyLocal2World(shape2->body, cpPolyShapeGetVert(shape2, j));
			mdiffVerts[i*shape2Count + j] = cpvsub(v2, v1);
		}
	}
	
	cpVect *hullVerts = alloca(mdiffCount*sizeof(cpVect));
	int hullCount = cpConvexHull(mdiffCount, mdiffVerts, hullVerts, NULL, 0.0);
	
	ChipmunkDebugDrawPolygon(hullCount, hullVerts, RGBAColor(1, 0, 0, 1), RGBAColor(1, 0, 0, 0.25));
	ChipmunkDebugDrawPoints(2.0, mdiffCount, mdiffVerts, RGBAColor(1, 0, 0, 1));
	
	// Draw the axis between the bodies
//	ChipmunkDebugDrawSegment(shape2->body->p, shape1->body->p, RGBAColor(1, 1, 1, 0.5));
	
	struct ClosestPoints pair = ClosestPoints(shape1, shape2);
	cpVect points[] = {pair.a, pair.b};
	ChipmunkDebugDrawPoints(3.0, 2, points, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points[0], points[1], RGBAColor(1, 1, 1, 1));
	
	ChipmunkDemoPrintString("Distance: %f", pair.d);
}

static cpSpace *
init(void)
{
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 5);
	space->damping = 0.1;
	
//	{
//		cpFloat width = 50.0;
//		cpFloat height = 70.0;
//		
//		cpFloat mass = 1.0f;
//		cpFloat moment = cpMomentForBox(mass, width, height);
//		
//		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
//		cpBodySetPos(body, cpv(50.0f, 0.0f));
//		
//		shape1 = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
//		shape1->group = 1;
//	}
//	
//	{
//		cpVect verts[] = {{-25, -25}, {-25, 25}, {50, -25}};
//		
//		cpFloat mass = 1.0f;
//		cpFloat moment = cpMomentForPoly(mass, 3, verts, cpvzero);
//		
//		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
//		cpBodySetPos(body, cpv(-50.0f, 0.0f));
//		
//		shape2 = cpSpaceAddShape(space, cpPolyShapeNew(body, 3, verts, cpvzero));
//		shape2->group = 1;
//	}
	
	{
		cpFloat mass = 1.0f;
		const int NUM_VERTS = 3;
		
		cpVect verts[NUM_VERTS];
		for(int i=0; i<NUM_VERTS; i++){
			cpFloat radius = 30.0;
			cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
			verts[i] = cpv(radius*cos(angle), radius*sin(angle));
		}
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, NUM_VERTS, verts, cpvzero)));
		cpBodySetPos(body, cpv(50.0f, 0.0f));
		
		shape1 = cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpvzero));
		shape1->group = 1;
	}
	
	{
		cpFloat mass = 1.0f;
		const int NUM_VERTS = 10;
		
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
