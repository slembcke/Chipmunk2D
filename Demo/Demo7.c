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

// TODO clean this up

extern cpSpace *space;
extern cpBody *staticBody;

cpConstraint *constraint;
cpBody *chassis, *wheel1, *wheel2;

void demo7_update(int ticks)
{
	int steps = 3;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
//		cpBodyResetForces(chassis);
//		cpBodyResetForces(wheel1);
//		cpBodyResetForces(wheel2);
//		cpApplyDampedSpring(chassis, wheel1, cpv(40, 15), cpvzero, 50.0f, 150.0f, 10.0f, dt);
//		cpApplyDampedSpring(chassis, wheel2, cpv(-40, 15), cpvzero, 50.0f, 150.0f, 10.0f, dt);
		
		cpSpaceStep(space, dt);
	}
}

static cpBody *
make_box(cpFloat x, cpFloat y)
{
	int num = 4;
	cpVect verts[] = {
		cpv(-15,-7),
		cpv(-15, 7),
		cpv( 15, 7),
		cpv( 15,-7),
	};
	
	cpFloat mass = 1.0;
	cpBody *body = cpBodyNew(mass, cpMomentForPoly(mass, num, verts, cpv(0,0)));
	//	cpBody *body1 = cpBodyNew(1.0/0.0, 1.0/0.0);
	body->p = cpv(x, y);
	cpSpaceAddBody(space, body);
	cpShape *shape = cpPolyShapeNew(body, num, verts, cpv(0,0));
	shape->e = 0.0; shape->u = 1.0;
	cpSpaceAddShape(space, shape);
	
	return body;
}

void demo7_init(void)
{
	staticBody = cpBodyNew(INFINITY, INFINITY);
	
	cpResetShapeIdCounter();
	space = cpSpaceNew();
	space->iterations = 30;
	cpSpaceResizeActiveHash(space, 50.0, 999);
	cpSpaceResizeStaticHash(space, 50.0, 999);
	space->gravity = cpv(0, -300);

	cpShape *shape;
	
	shape = cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);
	
	shape = cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);
	
	shape = cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);
	
	shape = cpSegmentShapeNew(staticBody, cpv(-320,70), cpv(0,-240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);
	
	shape = cpSegmentShapeNew(staticBody, cpv(0,-240), cpv(320,-200), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);
	
	shape = cpSegmentShapeNew(staticBody, cpv(200,-240), cpv(320,-100), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);
	
	cpBody *body1, *body2, *body3, *body4, *body5, *body6, *body7;
		
	cpFloat jointMax = 300000.0f;

//	body1 = make_box(-100, 100);
//	body2 = make_box(body1->p.x + 40, 100);
//	body3 = make_box(body2->p.x + 40, 100);
//	body4 = make_box(body3->p.x + 40, 100);
//	body5 = make_box(body4->p.x + 40, 100);
//	body6 = make_box(body5->p.x + 40, 100);
//	body7 = make_box(body6->p.x + 40, 100);
//	
//	constraint = cpPivotJointNew(staticBody, body1, cpv(-120,100), cpv(-20,0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPivotJointNew(body1, body2, cpv(20,0), cpv(-20,0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPivotJointNew(body2, body3, cpv(20,0), cpv(-20,0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPivotJointNew(body3, body4, cpv(20,0), cpv(-20,0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPivotJointNew(body4, body5, cpv(20,0), cpv(-20,0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPivotJointNew(body5, body6, cpv(20,0), cpv(-20,0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPivotJointNew(body6, body7, cpv(20,0), cpv(-20,0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPivotJointNew(body7, staticBody, cpv(20,0), cpv(body7->p.x + 20, 100));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
	
	
	body1 = make_box(-100, 50);
	body2 = make_box(body1->p.x + 40, 50);
	body3 = make_box(body2->p.x + 40, 50);
	body4 = make_box(body3->p.x + 40, 50);
	body5 = make_box(body4->p.x + 40, 50);
	body6 = make_box(body5->p.x + 40, 50);
	body7 = make_box(body6->p.x + 40, 50);
	
	cpFloat max = 12.0f;
	cpFloat min = 10.0f;
	
	constraint = cpSlideJointNew(staticBody, body1, cpv(body1->p.x - 15 - 10, 50), cpv(-15, 0), min, max);
	constraint->maxForce = jointMax;
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpSlideJointNew(body1, body2, cpv(15, 0), cpv(-15, 0), min, max);
	constraint->maxForce = jointMax;
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpSlideJointNew(body2, body3, cpv(15, 0), cpv(-15, 0), min, max);
	constraint->maxForce = jointMax;
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpSlideJointNew(body3, body4, cpv(15, 0), cpv(-15, 0), min, max);
	constraint->maxForce = jointMax;
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpSlideJointNew(body4, body5, cpv(15, 0), cpv(-15, 0), min, max);
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpSlideJointNew(body5, body6, cpv(15, 0), cpv(-15, 0), min, max);
	constraint->maxForce = jointMax;
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpSlideJointNew(body6, body7, cpv(15, 0), cpv(-15, 0), min, max);
	constraint->maxForce = jointMax;
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpSlideJointNew(body7, staticBody, cpv(15, 0), cpv(body7->p.x + 15 + 10, 50), min, max);
	constraint->maxForce = jointMax;
	constraint = cpBreakableJointNew(constraint, space);
	constraint->maxForce = jointMax;
	cpSpaceAddConstraint(space, constraint);
	
//	body1 = make_box(-100, 150);
//	body2 = make_box(body1->p.x + 40, 150);
//	body3 = make_box(body2->p.x + 40, 150);
//	body4 = make_box(body3->p.x + 40, 150);
//	body5 = make_box(body4->p.x + 40, 150);
//	body6 = make_box(body5->p.x + 40, 150);
//	body7 = make_box(body6->p.x + 40, 150);
//	
//	constraint = cpPinJointNew(staticBody, body1, cpv(body1->p.x - 15 - 10, 150), cpv(-15, 0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPinJointNew(body1, body2, cpv(15, 0), cpv(-15, 0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPinJointNew(body2, body3, cpv(15, 0), cpv(-15, 0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPinJointNew(body3, body4, cpv(15, 0), cpv(-15, 0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPinJointNew(body4, body5, cpv(15, 0), cpv(-15, 0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPinJointNew(body5, body6, cpv(15, 0), cpv(-15, 0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPinJointNew(body6, body7, cpv(15, 0), cpv(-15, 0));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
//	
//	constraint = cpPinJointNew(body7, staticBody, cpv(15, 0), cpv(body7->p.x + 15 + 10, 150));
//	constraint = cpBreakableJointNew(constraint, space);
//	constraint->maxForce = jointMax;
//	cpSpaceAddConstraint(space, constraint);
	
//	body1 = make_box(190, 200);
//	constraint = cpGrooveJointNew(staticBody, body1, cpv(0, 195), cpv(250, 200), cpv(-15, 0));
//	cpSpaceAddConstraint(space, constraint);
	
	int num = 4;
	cpVect verts[] = {
		cpv(-20,-15),
		cpv(-20, 15),
		cpv( 20, 15),
		cpv( 20,-15),
	};
	
	chassis = cpBodyNew(10.0, cpMomentForPoly(10.0, num, verts, cpv(0,0)));
	chassis->p = cpv(-200, 150);
//	body->v = cpv(200, 0);
	cpSpaceAddBody(space, chassis);
	shape = cpPolyShapeNew(chassis, num, verts, cpv(0,0));
	shape->e = 0.0; shape->u = 1.0;
	cpSpaceAddShape(space, shape);
	
	cpFloat radius = 15;
	cpFloat wheel_mass = 2;
	cpVect offset = cpv(radius + 30, -40);
	cpFloat stiffness = 600.0f;
	cpFloat damping = 1.0f;
	
	wheel1 = cpBodyNew(wheel_mass, cpMomentForCircle(wheel_mass, 0.0, radius, cpvzero));
	wheel1->p = cpvadd(chassis->p, offset);
	wheel1->v = chassis->v;
	cpSpaceAddBody(space, wheel1);
	shape = cpCircleShapeNew(wheel1, radius, cpvzero);
	shape->e = 0.0; shape->u = 2.5;
	cpSpaceAddShape(space, shape);
	
	constraint = cpPinJointNew(chassis, wheel1, cpvzero, cpvzero);
	cpSpaceAddConstraint(space, constraint);

	constraint = cpDampedSpringNew(chassis, wheel1, cpv(offset.x, offset.y), cpvzero, 0.0f, stiffness, damping);
	cpSpaceAddConstraint(space, constraint);

	
	
	wheel2 = cpBodyNew(wheel_mass, cpMomentForCircle(wheel_mass, 0.0, radius, cpvzero));
	wheel2->p = cpvadd(chassis->p, cpv(-offset.x, offset.y));
	wheel2->v = chassis->v;
	cpSpaceAddBody(space, wheel2);
	shape = cpCircleShapeNew(wheel2, radius, cpvzero);
	shape->e = 0.0; shape->u = 2.5;
	cpSpaceAddShape(space, shape);
	
	constraint = cpPinJointNew(chassis, wheel2, cpvzero, cpvzero);
	cpSpaceAddConstraint(space, constraint);
	
	constraint = cpDampedSpringNew(chassis, wheel2, cpv(-offset.x, offset.y), cpvzero, 0.0f, stiffness, damping);
	cpSpaceAddConstraint(space, constraint);
}
