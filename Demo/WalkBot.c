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
#include <stdio.h>
#include <math.h>

#include "chipmunk.h"
#include "drawSpace.h"
#include "ChipmunkDemo.h"

cpSpace *space;
cpBody *staticBody;
cpConstraint *motor;

static void
update(int ticks)
{
	cpFloat coef = (2.0f + arrowDirection.y)/3.0f;
	cpFloat rate = arrowDirection.x*3.0f*coef;
	cpSimpleMotor_set_rate(motor, -rate);

	int steps = 3;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static void
add_leg(cpBody *chassis, cpBody *crank, cpFloat crankOffset)
{
	cpFloat legMass = 2.0f;
	cpFloat legHLength = 80.0f/2.0f;
	cpVect leg_a = cpv(0.0f, legHLength);
	cpVect leg_b = cpv(0.0f, -legHLength);
	cpFloat legMoment = cpMomentForSegment(legMass, leg_a, leg_b);
	
	cpBody *body = cpBodyNew(legMass, legMoment);
	body->p = cpvadd(crank->p, cpv(0.0f, -legHLength + crankOffset));
	cpSpaceAddBody(space, body);
	
	cpShape *shape = cpSegmentShapeNew(body, leg_a, leg_b, 5.0f);
	shape->group = 1;
	cpSpaceAddShape(space, shape);
	
	cpSpaceAddConstraint(space, cpPivotJointNew2(crank, body, cpv(0.0f, crankOffset), cpv(0.0f, legHLength)));
	cpSpaceAddConstraint(space, cpGrooveJointNew(body, chassis, cpv(0.0f, legHLength), cpv(0.0f, -legHLength), cpv(crank->p.x - chassis->p.x, 0.0f)));
	
	// add a foot
	shape = cpCircleShapeNew(body, 10.0f, cpv(0.0f, -legHLength));
	shape->e = 0.0f; shape->u = 1.0f;
	shape->group = 1;
	cpSpaceAddShape(space, shape);
}

static cpSpace *
init(void)
{
	staticBody = cpBodyNew(INFINITY, INFINITY);
	
	cpResetShapeIdCounter();
	
	space = cpSpaceNew();
	space->gravity = cpv(0.0, -600.0);
	space->iterations = 50;
	
	cpBody *body;
	cpShape *shape;

	// add screen border
	shape = cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);

	// Make the chassis
	cpFloat chassisHW = 120.0f/2.0;
	cpFloat chassisHH = 20.0f/2.0;
	cpFloat chassisMass = 5.0;

	int num = 4;
	cpVect verts[] = {
		cpv(-chassisHW,-chassisHH),
		cpv(-chassisHW, chassisHH),
		cpv( chassisHW, chassisHH),
		cpv( chassisHW,-chassisHH),
	};
	
	cpBody *chassis = body = cpBodyNew(chassisMass, cpMomentForPoly(chassisMass, num, verts, cpvzero));
	body->p = cpv(200, -150);
	cpSpaceAddBody(space, body);
	
	shape = cpPolyShapeNew(body, num, verts, cpvzero);
	shape->e = 0.0f; shape->u = 1.0f;
	shape->group = 1;
	cpSpaceAddShape(space, shape);
	
	// Add crankshafts
	cpFloat crankMass = 1.0f;
	cpFloat crankRadius = 20.0f;
	cpFloat crankMoment = cpMomentForCircle(crankMass, crankRadius, 0.0f, cpvzero);
	cpFloat crankSpeed = (2.0f*M_PI)/3.0f;
	cpFloat crankXOffset = chassisHW - crankRadius;
	cpFloat crankYOffset = crankRadius + 30.0f;
	
	cpBody *crank1 = body = cpBodyNew(crankMass, crankMoment);
	body->p = cpvadd(chassis->p, cpv(crankXOffset, crankYOffset));
	cpSpaceAddBody(space, body);
	
	shape = cpCircleShapeNew(body, crankRadius, cpvzero);
	shape->e = 0.0f; shape->u = 1.0f;
	shape->group = 1;
	cpSpaceAddShape(space, shape);
	
	cpSpaceAddConstraint(space, cpPivotJointNew2(chassis, crank1, cpv(crankXOffset, crankYOffset), cpvzero));
	
	cpBody *crank2 = body = cpBodyNew(crankMass, crankMoment);
	body->p = cpvadd(chassis->p, cpv(-crankXOffset, crankYOffset));
	cpSpaceAddBody(space, body);
	
	shape = cpCircleShapeNew(body, crankRadius, cpvzero);
	shape->e = 0.0f; shape->u = 1.0f;
	shape->group = 1;
	cpSpaceAddShape(space, shape);
	
	cpSpaceAddConstraint(space, cpPivotJointNew2(chassis, crank2, cpv(-crankXOffset, crankYOffset), cpvzero));
	
	motor = cpSimpleMotorNew(chassis, crank1, crankSpeed);
	cpSpaceAddConstraint(space, motor);
	cpSpaceAddConstraint(space, cpGearJointNew(crank1, crank2, 0.0f, 1.0f));
	
	// add legs
	add_leg(chassis, crank1, crankRadius);
	add_leg(chassis, crank1, -crankRadius);
	add_leg(chassis, crank2, crankRadius);
	add_leg(chassis, crank2, -crankRadius);
	
	return space;
}

static void
destroy(void)
{
	cpBodyFree(staticBody);
	cpSpaceFreeChildren(space);
	cpSpaceFree(space);
}

const chipmunkDemo WalkBot = {
	"WalkBot",
	NULL,
	init,
	update,
	destroy,
};
