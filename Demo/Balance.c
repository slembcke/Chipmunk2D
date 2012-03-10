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

static cpBody *balance_body;
static cpBody *wheel_body;
static cpConstraint *motor;

static void motor_preSolve(cpConstraint *motor, cpSpace *space)
{
	cpFloat dt = cpSpaceGetCurrentTimeStep(space);
	
	cpFloat target_a = 0.0;
	cpFloat target_w = bias_coef(0.01, dt)*(target_a - balance_body->a)/dt;
	
	cpSimpleMotorSetRate(motor, wheel_body->w + balance_body->w - target_w);
	cpConstraintSetMaxForce(motor, 1.0e6);
}


static void
update(int ticks)
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
//	ChipmunkDemoMessageString = "Control the crane by moving the mouse. Right click to release.";
	
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -500));
	
	{
		cpShape *ground = cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpv(-1000.0, -240.0), cpv(1000.0, -240.0), 0.0));
		ground->u = 1.0;
	}
	
	
	cpFloat wheel_radius = 30.0;
	
	{
		cpFloat mass = 1.0;
		
		cpFloat moment = cpMomentForCircle(mass, 0.0, wheel_radius, cpvzero);
		wheel_body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
		wheel_body->p = cpv(0.0, -240.0 + wheel_radius);
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(wheel_body, wheel_radius, cpvzero));
		shape->u = 1.0;
		shape->group = 1;
	}
	
	{
		cpFloat length = 200.0;
		cpVect a = cpv(0.0,  length/2.0);
		cpVect b = cpv(0.0, -length/2.0);
		
		cpFloat mass = 10.0;
		cpFloat moment = cpMomentForSegment(mass, a, b);
		
		balance_body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
		balance_body->p = cpv(0.0, -240.0 + length/2.0 + wheel_radius);
		
		cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(balance_body, a, b, 10.0));
		shape->u = 1.0;
		shape->group = 1;
	}
	
	cpSpaceAddConstraint(space, cpPivotJointNew(wheel_body, balance_body, wheel_body->p));
	
	motor = cpSpaceAddConstraint(space, cpSimpleMotorNew(wheel_body, balance_body, 0.0));
	motor->preSolve = motor_preSolve;
	
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
