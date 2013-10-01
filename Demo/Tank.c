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

static cpBody *tankBody, *tankControlBody;

static void
update(cpSpace *space, double dt)
{
	// turn the control body based on the angle relative to the actual body
	cpVect mouseDelta = cpvsub(ChipmunkDemoMouse, cpBodyGetPosition(tankBody));
	cpFloat turn = cpvtoangle(cpvunrotate(cpBodyGetRotation(tankBody), mouseDelta));
	cpBodySetAngle(tankControlBody, cpBodyGetAngle(tankBody) - turn);
	
	// drive the tank towards the mouse
	if(cpvnear(ChipmunkDemoMouse, cpBodyGetPosition(tankBody), 30.0)){
		cpBodySetVelocity(tankControlBody, cpvzero); // stop
	} else {
		cpFloat direction = (cpvdot(mouseDelta, cpBodyGetRotation(tankBody)) > 0.0 ? 1.0 : -1.0);
		cpBodySetVelocity(tankControlBody, cpvrotate(cpBodyGetRotation(tankBody), cpv(30.0f*direction, 0.0f)));
	}
	
	cpSpaceStep(space, dt);
}

static cpBody *
add_box(cpSpace *space, cpFloat size, cpFloat mass)
{
	cpFloat radius = cpvlength(cpv(size, size));

	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, size, size)));
	cpBodySetPosition(body, cpv(frand()*(640 - 2*radius) - (320 - radius), frand()*(480 - 2*radius) - (240 - radius)));
	
	cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size, 0.0));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
	
	return body;
}

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "Use the mouse to drive the tank, it will follow the cursor.";
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 10);
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	
	cpBody *staticBody = cpSpaceGetStaticBody(space);
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

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	for(int i=0; i<50; i++){
		cpBody *body = add_box(space, 20, 1);
		
		cpConstraint *pivot = cpSpaceAddConstraint(space, cpPivotJointNew2(staticBody, body, cpvzero, cpvzero));
		cpConstraintSetMaxBias(pivot, 0); // disable joint correction
		cpConstraintSetMaxForce(pivot, 1000.0f); // emulate linear friction
		
		cpConstraint *gear = cpSpaceAddConstraint(space, cpGearJointNew(staticBody, body, 0.0f, 1.0f));
		cpConstraintSetMaxBias(gear, 0); // disable joint correction
		cpConstraintSetMaxForce(gear, 5000.0f); // emulate angular friction
	}
	
	// We joint the tank to the control body and control the tank indirectly by modifying the control body.
	tankControlBody = cpSpaceAddBody(space, cpBodyNewKinematic());
	tankBody = add_box(space, 30, 10);
	
	cpConstraint *pivot = cpSpaceAddConstraint(space, cpPivotJointNew2(tankControlBody, tankBody, cpvzero, cpvzero));
	cpConstraintSetMaxBias(pivot, 0); // disable joint correction
	cpConstraintSetMaxForce(pivot, 10000.0f); // emulate linear friction
	
	cpConstraint *gear = cpSpaceAddConstraint(space, cpGearJointNew(tankControlBody, tankBody, 0.0f, 1.0f));
	cpConstraintSetErrorBias(gear, 0); // attempt to fully correct the joint each step
	cpConstraintSetMaxBias(gear, 1.2f);  // but limit it's angular correction rate
	cpConstraintSetMaxForce(gear, 50000.0f); // emulate angular friction
		
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Tank = {
	"Tank",
	1.0/60.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
