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

enum {
	COLLIDE_STICK_SENSOR = 1,
};

#define STICK_SENSOR_THICKNESS 3.0f

static void
PostStepAddJoint(cpSpace *space, void *key, void *data)
{
//	printf("Adding joint for %p\n", data);
	
	cpConstraint *joint = (cpConstraint *)key;
	cpSpaceAddConstraint(space, joint);
}

static cpBool
StickyPreSolve(cpArbiter *arb, cpSpace *space, void *data)
{
	// All the sticky surfaces are covered by sensor shapes.
	// The sensor shapes give everything a little thicknes 
	
	// If sensor pairs don't already
	if(!cpArbiterGetUserData(arb) && cpArbiterGetDepth(arb, 0) <= -2.0f*STICK_SENSOR_THICKNESS){
		CP_ARBITER_GET_BODIES(arb, bodyA, bodyB);
		cpVect point = cpArbiterGetPoint(arb, 0);
		
		// Create a joint at the contact point to hold the body in place.
		cpConstraint *joint = cpPivotJointNew(bodyA, bodyB, point);
		
		// Give it a finite force for the stickyness.
		cpConstraintSetMaxForce(joint, 1e4);
		
		// Schedule a post-step() callback to add the joint.
		cpSpaceAddPostStepCallback(space, PostStepAddJoint, joint, NULL);
		
		// Store the joint on the arbiter so we can remove it later.
		cpArbiterSetUserData(arb, joint);
	}
	
	return cpTrue;
}

static void
PostStepRemoveJoint(cpSpace *space, void *key, void *data)
{
//	printf("Removing joint for %p\n", data);
	
	cpConstraint *joint = (cpConstraint *)key;
	cpSpaceRemoveConstraint(space, joint);
	cpConstraintFree(joint);
}

static void
StickySeparate(cpArbiter *arb, cpSpace *space, void *data)
{
	cpConstraint *joint = (cpConstraint *)cpArbiterGetUserData(arb);
	
	if(joint){
		// The joint won't be removed until the step is done.
		// Need to disable it so that it won't apply itself.
		// Setting the force to 0 will do just that
		cpConstraintSetMaxForce(joint, 0.0f);
		
		// Perform the removal in a post-step() callback.
		cpSpaceAddPostStepCallback(space, PostStepRemoveJoint, joint, NULL);
		
		// NULL out the reference to the joint.
		// Not required, but it's a good practice.
		cpArbiterSetUserData(arb, NULL);
	}
}

static void
update(cpSpace *space)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "Sticky collisions using the cpArbiter data pointer.";
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 10);
	cpSpaceSetGravity(space, cpv(0, -1000));
	cpSpaceSetCollisionSlop(space, 2.0);

	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;

	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320, 240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv( 320,-240), cpv( 320, 240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv( 320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320, 240), cpv( 320, 240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	// Also add slightly thicker sensor shapes for all of them
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320, 240), STICK_SENSOR_THICKNESS));
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	cpShapeSetCollisionType(shape, COLLIDE_STICK_SENSOR);
	cpShapeSetSensor(shape, cpTrue);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv( 320,-240), cpv( 320, 240), STICK_SENSOR_THICKNESS));
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	cpShapeSetCollisionType(shape, COLLIDE_STICK_SENSOR);
	cpShapeSetSensor(shape, cpTrue);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv( 320,-240), STICK_SENSOR_THICKNESS));
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	cpShapeSetCollisionType(shape, COLLIDE_STICK_SENSOR);
	cpShapeSetSensor(shape, cpTrue);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320, 240), cpv( 320, 240), STICK_SENSOR_THICKNESS));
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	cpShapeSetCollisionType(shape, COLLIDE_STICK_SENSOR);
	cpShapeSetSensor(shape, cpTrue);
	
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			cpFloat radius = 30.0f;
			body = cpSpaceAddBody(space, cpBodyNew(1.0f, cpMomentForCircle(1.0f, 0.0f, radius, cpvzero)));
			cpBodySetPos(body, cpv((i-2)*2.0*radius, (j-2)*2.0*radius));

			shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
			cpShapeSetFriction(shape, 0.9f);
			
			// Add on the thickened sensor shapes.
			shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius + STICK_SENSOR_THICKNESS, cpvzero));
			cpShapeSetSensor(shape, cpTrue);
			cpShapeSetCollisionType(shape, COLLIDE_STICK_SENSOR);
		}
	}
	
	cpSpaceAddCollisionHandler(space, COLLIDE_STICK_SENSOR, COLLIDE_STICK_SENSOR, NULL, StickyPreSolve, NULL, StickySeparate, NULL);
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Sticky = {
	"Sticky Surfaces",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
