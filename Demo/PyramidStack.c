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
 
#include "chipmunk.h"
#include "ChipmunkDemo.h"
#include "chipmunk_unsafe.h"

static void
update(cpSpace *space)
{
	int steps = 3;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -100));
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	cpSpaceSetCollisionSlop(space, 0.5f);
	
	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	// Add lots of boxes.
	for(int i=0; i<1; i++){
		for(int j=0; j<=i; j++){
			cpFloat s = 100.0;
			body = cpBodyNew(0.0f, 0.0f);
			cpBodySetPos(body, cpv(0, 0));

			cpFloat density = 1.0/900.0f;
			
			cpShape *shape1 = cpBoxShapeNew2(body, cpBBNew(0, 0,   s,   s));
			cpShapeSetFriction(shape1, 0.7);
			cpBodyAddMassForShape(body, shape1, density);
			
			cpShape *shape2 = cpBoxShapeNew2(body, cpBBNew(s, 0, 2*s,   s));
			cpShapeSetFriction(shape2, 0.7);
			cpBodyAddMassForShape(body, shape2, density);
			
			cpShape *shape3 = cpBoxShapeNew2(body, cpBBNew(0, s,   s, 2*s));
			cpShapeSetFriction(shape3, 0.7);
			cpBodyAddMassForShape(body, shape3, density);
			
			cpShape *shape4 = cpBoxShapeNew2(body, cpBBNew(s, s, 2*s, 2*s));
			cpShapeSetFriction(shape4, 0.7);
			cpBodyAddMassForShape(body, shape4, density);
			
			cpSpaceAddBody(space, body);
//			cpBodySetPos(body, cpvzero);
			
			cpSpaceAddShape(space, shape1);
			cpSpaceAddShape(space, shape2);
			cpSpaceAddShape(space, shape3);
			cpSpaceAddShape(space, shape4);
		}
	}
	
	// Add a ball to make things more interesting
//	cpFloat radius = 15.0f;
//	body = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
//	cpBodySetPos(body, cpv(0, -240 + radius+5));
//
//	shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
//	cpShapeSetElasticity(shape, 0.0f);
//	cpShapeSetFriction(shape, 0.9f);
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo PyramidStack = {
	"Pyramid Stack",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
