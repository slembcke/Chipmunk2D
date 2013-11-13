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

enum {
	COLLISION_TYPE_STICKY = 1,
};

#define STICK_SENSOR_THICKNESS 2.5f

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
	// We want to fudge the collisions a bit to allow shapes to overlap more.
	// This simulates their squishy sticky surface, and more importantly
	// keeps them from separating and destroying the joint.
	
	// Track the deepest collision point and use that to determine if a rigid collision should occur.
	cpFloat deepest = INFINITY;
	
	// Grab the contact set and iterate over them.
	cpContactPointSet contacts = cpArbiterGetContactPointSet(arb);
	for(int i=0; i<contacts.count; i++){
		// Sink the contact points into the surface of each shape.
		contacts.points[i].point1 = cpvsub(contacts.points[i].point1, cpvmult(contacts.normal, STICK_SENSOR_THICKNESS));
		contacts.points[i].point2 = cpvadd(contacts.points[i].point2, cpvmult(contacts.normal, STICK_SENSOR_THICKNESS));
		deepest = cpfmin(deepest, contacts.points[i].distance);// + 2.0f*STICK_SENSOR_THICKNESS);
	}
	
	// Set the new contact point data.
	cpArbiterSetContactPointSet(arb, &contacts);
	
	// If the shapes are overlapping enough, then create a
	// joint that sticks them together at the first contact point.
	if(!cpArbiterGetUserData(arb) && deepest <= 0.0f){
		CP_ARBITER_GET_BODIES(arb, bodyA, bodyB);
		
		// Create a joint at the contact point to hold the body in place.
		cpVect anchorA = cpBodyWorldToLocal(bodyA, contacts.points[0].point1);
		cpVect anchorB = cpBodyWorldToLocal(bodyB, contacts.points[0].point2);
		cpConstraint *joint = cpPivotJointNew2(bodyA, bodyB, anchorA, anchorB);
		
		// Give it a finite force for the stickyness.
		cpConstraintSetMaxForce(joint, 3e3);
		
		// Schedule a post-step() callback to add the joint.
		cpSpaceAddPostStepCallback(space, PostStepAddJoint, joint, NULL);
		
		// Store the joint on the arbiter so we can remove it later.
		cpArbiterSetUserData(arb, joint);
	}
	
	// Position correction and velocity are handled separately so changing
	// the overlap distance alone won't prevent the collision from occuring.
	// Explicitly the collision for this frame if the shapes don't overlap using the new distance.
	return (deepest <= 0.0f);
	
	// Lots more that you could improve upon here as well:
	// * Modify the joint over time to make it plastic.
	// * Modify the joint in the post-step to make it conditionally plastic (like clay).
	// * Track a joint for the deepest contact point instead of the first.
	// * Track a joint for each contact point. (more complicated since you only get one data pointer).
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
update(cpSpace *space, double dt)
{
	cpSpaceStep(space, dt);
}

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "Sticky collisions using the cpArbiter data pointer.";
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 10);
	cpSpaceSetGravity(space, cpv(0, -1000));
	cpSpaceSetCollisionSlop(space, 2.0);

	cpBody *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;

	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-340,-260), cpv(-340, 260), 20.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv( 340,-260), cpv( 340, 260), 20.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-340,-260), cpv( 340,-260), 20.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-340, 260), cpv( 340, 260), 20.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	for(int i=0; i<200; i++){
		cpFloat mass = 0.15f;
		cpFloat radius = 10.0f;
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
		cpBodySetPosition(body, cpv(cpflerp(-150.0f, 150.0f, frand()), cpflerp(-150.0f, 150.0f, frand())));

		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius + STICK_SENSOR_THICKNESS, cpvzero));
		cpShapeSetFriction(shape, 0.9f);
		cpShapeSetCollisionType(shape, COLLISION_TYPE_STICKY);
	}
	
	cpCollisionHandler *handler = cpSpaceAddWildcardHandler(space, COLLISION_TYPE_STICKY);
	handler->preSolveFunc = StickyPreSolve;
	handler->separateFunc = StickySeparate;
	
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
	1.0/60.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
