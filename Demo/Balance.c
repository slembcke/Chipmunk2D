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

static cpSpace *space;

static cpBody *balanceBody;
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
		cpFloat currentLength = cpvdist(anchr1, anchr2);
		
//		cpFloat desiredLength = cpfclamp(ChipmunkDemoMouse.x + LENGTH*0.5, LENGTH*0.25, LENGTH*0.75);
		cpFloat correctionFactor = cpBodyGetAngVel(balanceBody);
		cpFloat desiredLength = currentLength + correctionFactor*cpConstraintGetMaxBias(counterweightServo);

		ChipmunkDemoPrintString("current: %5.2f desired: %5.2f", currentLength, desiredLength);
		cpPinJointSetDist(counterweightServo, cpfclamp(desiredLength, LENGTH*0.25, LENGTH*0.75));
		
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
//	ChipmunkDemoMessageString = "Control the crane by moving the mouse. Right click to release.";
	
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -100));
	cpSpaceSetDamping(space, 0.8);
	
	cpVect a = cpv(-LENGTH/2.0, 0.0);
	cpVect b = cpv( LENGTH/2.0, 0.0);
	
	// Balance bar
	{
		cpFloat mass = 10.0;
		balanceBody = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForSegment(mass, a, b)));
		
		cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(balanceBody, a, b, 10.0));
		shape->group = 1;
		
		cpSpaceAddConstraint(space, cpPivotJointNew(space->staticBody, balanceBody, cpvzero));
	}
	
	// Counterweight
	{
		cpFloat mass = 30.0;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, INFINITY));
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, 20.0, cpvzero));
		shape->group = 1;
		
		// A pin joint is a little annoying for this, but the constraint needs to be monodirectional. 
		counterweightServo = cpSpaceAddConstraint(space, cpPinJointNew(balanceBody, body, a, cpvzero));
		cpConstraintSetMaxForce(counterweightServo, 1e5);
		cpConstraintSetMaxBias(counterweightServo, 100.0);
		cpConstraintSetErrorBias(counterweightServo, 0.5);
		
		cpSpaceAddConstraint(space, cpGrooveJointNew(balanceBody, body, a, b, cpvzero));
	}
	
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
