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

static cpBody *dollyBody = NULL;
// Constraint used as a servo motor to move the dolly back and forth.
static cpConstraint *dollyServo = NULL;

// Constraint used as a winch motor to lift the load.
static cpConstraint *winchServo = NULL;

// Temporary joint used to hold the hook to the load.
static cpConstraint *hookJoint = NULL;


static void
update(cpSpace *space)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		// Set the first anchor point (the one attached to the static body) of the dolly servo to the mouse's x position.
		cpPivotJointSetAnchr1(dollyServo, cpv(ChipmunkDemoMouse.x, 100));
		
		// Set the max length of the winch servo to match the mouse's height.
		cpSlideJointSetMax(winchServo, cpfmax(100 - ChipmunkDemoMouse.y, 50));
		
		if(hookJoint && ChipmunkDemoRightClick){
			cpSpaceRemoveConstraint(space, hookJoint);
			cpConstraintFree(hookJoint);
			hookJoint = NULL;
		}
		
		cpSpaceStep(space, dt);
	}
}

enum COLLISION_TYPES {
	HOOK_SENSOR = 1,
	CRATE,
};

static void
AttachHook(cpSpace *space, cpBody *hook, cpBody *crate)
{
	hookJoint = cpSpaceAddConstraint(space, cpPivotJointNew(hook, crate, cpBodyGetPos(hook)));
}


static cpBool
HookCrate(cpArbiter *arb, cpSpace *space, void *data)
{
	if(hookJoint == NULL){
		// Get pointers to the two bodies in the collision pair and define local variables for them.
		// Their order matches the order of the collision types passed
		// to the collision handler this function was defined for
		CP_ARBITER_GET_BODIES(arb, hook, crate);
		
		// additions and removals can't be done in a normal callback.
		// Schedule a post step callback to do it.
		// Use the hook as the key and pass along the arbiter.
		cpSpaceAddPostStepCallback(space, (cpPostStepFunc)AttachHook, hook, crate);
	}
	
	return cpTrue; // return value is ignored for sensor callbacks anyway
}


static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "Control the crane by moving the mouse. Right click to release.";
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -100));
	cpSpaceSetDamping(space, 0.8);
	
	cpBody *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	// Add a body for the dolly.
	dollyBody = cpSpaceAddBody(space, cpBodyNew(10, INFINITY));
	cpBodySetPos(dollyBody, cpv(0, 100));
	
	// Add a block so you can see it.
	cpSpaceAddShape(space, cpBoxShapeNew(dollyBody, 30, 30));
	
	// Add a groove joint for it to move back and forth on.
	cpSpaceAddConstraint(space, cpGrooveJointNew(staticBody, dollyBody, cpv(-250, 100), cpv(250, 100), cpvzero));
	
	// Add a pivot joint to act as a servo motor controlling it's position
	// By updating the anchor points of the pivot joint, you can move the dolly.
	dollyServo = cpSpaceAddConstraint(space, cpPivotJointNew(staticBody, dollyBody, cpBodyGetPos(dollyBody)));
	// Max force the dolly servo can generate.
	cpConstraintSetMaxForce(dollyServo, 10000);
	// Max speed of the dolly servo
	cpConstraintSetMaxBias(dollyServo, 100);
	// You can also change the error bias to control how it slows down.
	//cpConstraintSetErrorBias(dollyServo, 0.2);
	
	
	// Add the crane hook.
	cpBody *hookBody = cpSpaceAddBody(space, cpBodyNew(1, INFINITY));
	cpBodySetPos(hookBody, cpv(0, 50));
	
	// Add a sensor shape for it. This will be used to figure out when the hook touches a box.
	shape = cpSpaceAddShape(space, cpCircleShapeNew(hookBody, 10, cpvzero));
	cpShapeSetSensor(shape, cpTrue);
	cpShapeSetCollisionType(shape, HOOK_SENSOR);
	
	// Add a slide joint to act as a winch motor
	// By updating the max length of the joint you can make it pull up the load.
	winchServo = cpSpaceAddConstraint(space, cpSlideJointNew(dollyBody, hookBody, cpvzero, cpvzero, 0, INFINITY));
	// Max force the dolly servo can generate.
	cpConstraintSetMaxForce(winchServo, 30000);
	// Max speed of the dolly servo
	cpConstraintSetMaxBias(winchServo, 60);
	
	// TODO cleanup
	// Finally a box to play with
	cpBody *boxBody = cpSpaceAddBody(space, cpBodyNew(30, cpMomentForBox(30, 50, 50)));
	cpBodySetPos(boxBody, cpv(200, -200));
	
	// Add a block so you can see it.
	shape = cpSpaceAddShape(space, cpBoxShapeNew(boxBody, 50, 50));
	cpShapeSetFriction(shape, 0.7);
	cpShapeSetCollisionType(shape, CRATE);
	
	cpSpaceAddCollisionHandler(space, HOOK_SENSOR, CRATE, (cpCollisionBeginFunc)HookCrate, NULL, NULL, NULL, NULL);
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Crane = {
	"Crane",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
