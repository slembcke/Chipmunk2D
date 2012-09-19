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

static void
update(int ticks)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static cpCollisionID id = 0;

static void
draw(void)
{
	ChipmunkDemoDefaultDrawImpl();
	cpContact arr[CP_MAX_CONTACTS_PER_ARBITER];
	cpCollideShapes(shape1, shape2, &id, arr);
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
		cpBodySetPos(body, cpv(-80.0f, 0.0f));
		cpBodySetAngle(body, M_PI_4);
		
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
//		cpBodySetPos(body, cpv(-50.0f, 0.0f));
//		
//		shape1 = cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpvzero));
//		shape1->group = 1;
//	}
	
	{
		cpFloat mass = 1.0f;
		cpFloat size = 100.0f;
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, size, size)));
		cpBodySetPos(body, cpv(50.0f, 0.0f));
		
		shape2 = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size));
		shape2->group = 1;
	}
	
//	{
//		cpFloat mass = 1.0f;
//		cpVect a = cpv( 75.0, 0.0);
//		cpVect b = cpv(-75.0, 0.0);
//		
//		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForSegment(mass, a, b)));
//		cpBodySetPos(body, cpv(-70.0f, 0.0f));
//		
//		shape1 = cpSpaceAddShape(space, cpSegmentShapeNew(body, a, b, 45.0));
//		shape1->group = 1;
//	}
//	
//	{
//		cpFloat mass = 1.0f;
//		const int NUM_VERTS = 5;
//		
//		cpVect verts[NUM_VERTS];
//		for(int i=0; i<NUM_VERTS; i++){
//			cpFloat radius = 60.0;
//			cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
//			verts[i] = cpv(radius*cos(angle), radius*sin(angle));
//		}
//		
//		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, NUM_VERTS, verts, cpvzero)));
//		cpBodySetPos(body, cpv(50.0f, 100.0f));
//		
//		cpVect a = cpv( 50.0, 0.0);
//		cpVect b = cpv(-50.0, 0.0);
//		shape2 = cpSpaceAddShape(space, cpSegmentShapeNew(body, a, b, 15.0));
////		shape2 = cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpvzero));
//		shape2->group = 1;
//	}
	
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
