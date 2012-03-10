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
#include <math.h>

#include "chipmunk.h"
#include "ChipmunkDemo.h"

#include "util.h"

static cpSpace *space;

static cpBody *balanceBody;
static cpFloat lastW = 0.0;
static cpConstraint *counterweightServo;

#define LENGTH 400.0


static void
update(int ticks)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpVect anchr1 = cpBodyLocal2World(cpConstraintGetA(counterweightServo), cpPinJointGetAnchr1(counterweightServo));
		cpVect anchr2 = cpBodyLocal2World(cpConstraintGetB(counterweightServo), cpPinJointGetAnchr2(counterweightServo));
		cpFloat currentOffset = cpvdist(anchr1, anchr2) - LENGTH/2.0;
		
		cpFloat a = cpBodyGetAngle(balanceBody);
		cpFloat w = cpBodyGetAngVel(balanceBody);
		cpFloat aa = (w - lastW)/dt;
		lastW = w;
		
		cpFloat target_a = 0.0;
		cpFloat target_w = bias_coef(0.1, dt)*(target_a - a)/dt;
		cpFloat target_aa = bias_coef(0.1, dt)*(target_w - w)/dt;
		ChipmunkDemoPrintString("a:%f, target_a:%f\n", a, target_a);
		ChipmunkDemoPrintString("w:%f, target_w:%f\n", w, target_w);
		ChipmunkDemoPrintString("aa:%f, target_aa:%f\n", aa, target_aa);
		
		cpFloat correctionFactor = (target_aa - aa)*-1.0;
		cpFloat desiredOffset = currentOffset + correctionFactor;
		ChipmunkDemoPrintString("correction: %f\n", correctionFactor);

//		cpConstraintSetMaxForce(counterweightServo, 1e5);
//		cpConstraintSetMaxBias(counterweightServo, 10000.0);
		cpConstraintSetErrorBias(counterweightServo, 0.0);
		cpPinJointSetDist(counterweightServo, cpfclamp(desiredOffset + LENGTH/2.0, LENGTH*0.05, LENGTH*0.95));
		
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
//	ChipmunkDemoMessageString = "Control the crane by moving the mouse. Right click to release.";
	
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -1));
//	cpSpaceSetDamping(space, 0.8);
	
	cpVect a = cpv(-LENGTH/2.0, 0.0);
	cpVect b = cpv( LENGTH/2.0, 0.0);
	
	// Balance bar
	{
		cpFloat mass = 10.0;
		balanceBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForSegment(mass, a, b)));
		cpBodySetAngle(balanceBody, 1.0);
		
		cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(balanceBody, a, b, 10.0));
		cpShapeSetFriction(shape, 1.0);
		
		cpSpaceAddConstraint(space, cpPivotJointNew(space->staticBody, balanceBody, cpvzero));
	}
	
	// Counterweight
	{
		cpFloat mass = 30.0;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, INFINITY));
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, 20.0, cpvzero));
		cpShapeSetLayers(shape, 0);
		
		// A pin joint is a little annoying for this, but the constraint needs to be monodirectional. 
		counterweightServo = cpSpaceAddConstraint(space, cpPinJointNew(balanceBody, body, a, cpvzero));
		
		cpSpaceAddConstraint(space, cpGrooveJointNew(balanceBody, body, a, b, cpvzero));
	}
	
//	{
//		cpBody *boxBody = cpSpaceAddBody(space, cpBodyNew(20, cpMomentForBox(30, 50, 50)));
//		cpBodySetPos(boxBody, cpv(0, 100));
//		
//		cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(boxBody, 50, 50));
//		cpShapeSetFriction(shape, 1.0);
//	}
	
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Balance = {
	"Balance",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
