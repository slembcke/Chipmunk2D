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

extern cpSpace *space;
extern cpBody *staticBody;

static void
update(int ticks)
{
	int steps = 2;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static cpBody *
add_ball(cpVect p)
{
	cpBody *body = cpBodyNew(1.0f, cpMomentForCircle(1.0f, 30, 0, cpvzero));
	body->p = p;
	cpSpaceAddBody(space, body);
	
	cpShape *shape = cpCircleShapeNew(body, 30, cpvzero);
	shape->e = 0.0; shape->u = 0.5;
	cpSpaceAddShape(space, shape);
	
	return body;
}

static void
make_gear(cpBody *body, cpFloat radius, int teeth)
{
	cpFloat circ = 2.0*radius*M_PI;
	cpFloat tooth_radius = circ/4.0/(cpFloat)teeth;
	
	for(int i=0; i<teeth; i++){
		cpFloat angle = (cpFloat)i*2.0*M_PI/(cpFloat)teeth;
		cpVect offset = cpvmult(cpvforangle(angle), radius);
		
		cpShape *tooth = cpCircleShapeNew(body, tooth_radius*0.95, offset);
		tooth->u = 0.5; tooth->layers = 2;
		cpSpaceAddShape(space, tooth);
	}
}

static cpSpace *
init(void)
{
	staticBody = cpBodyNew(INFINITY, INFINITY);
	
	space = cpSpaceNew();
	space->gravity = cpv(0, -600);
	
	cpShape *shape;
	
	// beveling all of the line segments helps prevent things from getting stuck on cracks
	shape = cpSegmentShapeNew(staticBody, cpv(-256,16), cpv(-256,240), 2.0f);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-256,16), cpv(-192,0), 2.0f);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-192,0), cpv(-192, -64), 2.0f);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-128,-64), cpv(-128,144), 2.0f);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-192,80), cpv(-192,176), 2.0f);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-192,176), cpv(-128,240), 2.0f);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-128,144), cpv(192,64), 2.0f);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddStaticShape(space, shape);

	cpVect verts[] = {
		cpv(-30,-80),
		cpv(-30, 80),
		cpv( 30, 64),
		cpv( 30,-80),
	};

	cpBody *plunger = cpBodyNew(1.0f, INFINITY);
	plunger->p = cpv(-160,-80);
	cpSpaceAddBody(space, plunger);
	
	shape = cpPolyShapeNew(plunger, 4, verts, cpvzero);
	shape->e = 1.0; shape->u = 0.5; shape->layers = 1;
	cpSpaceAddShape(space, shape);
	
	// add balls to hopper
	add_ball(cpv(-224,80));
	add_ball(cpv(-224,80+64));
	add_ball(cpv(-224,80+64*2));
	
	// add gears
	cpBody *smallGear = cpBodyNew(10.0f, cpMomentForCircle(10.0f, 80, 0, cpvzero));
	smallGear->p = cpv(-160,-160);
	cpSpaceAddBody(space, smallGear);
	
	make_gear(smallGear, 80, 10);
	cpSpaceAddConstraint(space, cpPivotJointNew(staticBody, smallGear, cpv(-160,-160), cpvzero));

	cpBody *bigGear = cpBodyNew(10.0f, cpMomentForCircle(10.0f, 80, 0, cpvzero));
	bigGear->p = cpv(80,-160);
	cpSpaceAddBody(space, bigGear);
	
	make_gear(bigGear, 160, 20);
	cpSpaceAddConstraint(space, cpPivotJointNew(staticBody, bigGear, cpv(80,-160), cpvzero));

	// connect the plunger to the small gear.
	cpSpaceAddConstraint(space, cpPinJointNew(smallGear, plunger, cpv(0,-80), cpv(0,0)));
	
	return space;
}

static void
destroy(void)
{
	cpBodyFree(staticBody);
	cpSpaceFreeChildren(space);
	cpSpaceFree(space);
}

const chipmunkDemo Pump = {
	"Pump",
	NULL,
	init,
	update,
	destroy,
};
