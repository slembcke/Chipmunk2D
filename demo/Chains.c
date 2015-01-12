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

#define CHAIN_COUNT 8
#define LINK_COUNT 10

static void
BreakablejointPostStepRemove(cpSpace *space, cpConstraint *joint, void *unused)
{
	cpSpaceRemoveConstraint(space, joint);
	cpConstraintFree(joint);
}

static void
BreakableJointPostSolve(cpConstraint *joint, cpSpace *space)
{
	cpFloat dt = cpSpaceGetCurrentTimeStep(space);
	
	// Convert the impulse to a force by dividing it by the timestep.
	cpFloat force = cpConstraintGetImpulse(joint)/dt;
	cpFloat maxForce = cpConstraintGetMaxForce(joint);

	// If the force is almost as big as the joint's max force, break it.
	if(force > 0.9*maxForce){
		cpSpaceAddPostStepCallback(space, (cpPostStepFunc)BreakablejointPostStepRemove, joint, NULL);
	}
}

static void
update(cpSpace *space, double dt)
{
	cpSpaceStep(space, dt);
}

static cpSpace *
init(void)
{
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -100));
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	
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
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	cpFloat mass = 1;
	cpFloat width = 20;
	cpFloat height = 30;
	
	cpFloat spacing = width*0.3;
	
	// Add lots of boxes.
	for(int i=0; i<CHAIN_COUNT; i++){
		cpBody *prev = NULL;
		
		for(int j=0; j<LINK_COUNT; j++){
			cpVect pos = cpv(40*(i - (CHAIN_COUNT - 1)/2.0), 240 - (j + 0.5)*height - (j + 1)*spacing);
			
			body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
			cpBodySetPosition(body, pos);
			
			shape = cpSpaceAddShape(space, cpSegmentShapeNew(body, cpv(0, (height - width)/2.0), cpv(0, (width - height)/2.0), width/2.0));
			cpShapeSetFriction(shape, 0.8f);
			
			cpFloat breakingForce = 80000;
			
			cpConstraint *constraint = NULL;
			if(prev == NULL){
				constraint = cpSpaceAddConstraint(space, cpSlideJointNew(body, staticBody, cpv(0, height/2), cpv(pos.x, 240), 0, spacing));
			} else {
				constraint = cpSpaceAddConstraint(space, cpSlideJointNew(body, prev, cpv(0, height/2), cpv(0, -height/2), 0, spacing));
			}
			
			cpConstraintSetMaxForce(constraint, breakingForce);
			cpConstraintSetPostSolveFunc(constraint, BreakableJointPostSolve);
			cpConstraintSetCollideBodies(constraint, cpFalse);
			
			prev = body;
		}
	}
	
	cpFloat radius = 15.0f;
	body = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
	cpBodySetPosition(body, cpv(0, -240 + radius+5));
	cpBodySetVelocity(body, cpv(0, 300));

	shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.9f);
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Chains = {
	"Breakable Chains",
	1.0/180.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
