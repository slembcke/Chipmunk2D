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
 
#include "chipmunk/chipmunk.h"
#include "ChipmunkDemo.h"

enum CollisionTypes {
	COLLISION_TYPE_ONE_WAY = 1,
};

typedef struct OneWayPlatform {
	cpVect n; // direction objects may pass through
} OneWayPlatform;

static OneWayPlatform platformInstance;

static cpBool
PreSolve(cpArbiter *arb, cpSpace *space, void *ignore)
{
	CP_ARBITER_GET_SHAPES(arb, a, b);
	OneWayPlatform *platform = (OneWayPlatform *)cpShapeGetUserData(a);
		
	if(cpvdot(cpArbiterGetNormal(arb), platform->n) < 0){
		return cpArbiterIgnore(arb);
	}
	
	return cpTrue;
}

static void
update(cpSpace *space, double dt)
{
	cpSpaceStep(space, dt);
}

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "One way platforms are trivial in Chipmunk using a very simple collision callback.";
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 10);
	cpSpaceSetGravity(space, cpv(0, -100));

	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;

	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	// Add our one way segment
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-160,-100), cpv(160,-100), 10.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetCollisionType(shape, COLLISION_TYPE_ONE_WAY);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	// We'll use the data pointer for the OneWayPlatform struct
	platformInstance.n = cpv(0, 1); // let objects pass upwards
	cpShapeSetUserData(shape, &platformInstance);
	
	
	// Add a ball to test it out
	cpFloat radius = 15.0f;
	body = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
	cpBodySetPosition(body, cpv(0, -200));
	cpBodySetVelocity(body, cpv(0, 170));

	shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.9f);
	cpShapeSetCollisionType(shape, 2);
	
	cpCollisionHandler *handler = cpSpaceAddWildcardHandler(space, COLLISION_TYPE_ONE_WAY);
	handler->preSolveFunc = PreSolve;
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo OneWay = {
	"One Way Platforms",
	1.0/60.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
