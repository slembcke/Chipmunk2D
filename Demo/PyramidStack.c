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

static void
update(int ticks)
{
	int steps = 3;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static void
untouch(cpArbiter *arb, cpSpace *space, void *unused)
{
	printf("untouched a wall\n");
}

// This function has existed for a while, but was not useful until now really
cpVect cpContactsSumImpulses(cpContact *contacts, int numContacts);

static void
printCollisionImpulse(cpArbiter *arb, void *unused)
{
	cpVect j = cpContactsSumImpulses(arb->contacts, arb->numContacts);
	printf("Collision impulse:%s magnitude:%f\n", cpvstr(j), cpvlength(j));
}

static int
collision(cpArbiter *arb, cpSpace *space, void *data){
// The old arguments would still be easy to get. I'd probably write helper/getter functions for these.
//	int swapped = arb->swappedColl;
//	cpShape *a = (swapped ? arb->b : arb->a);
//	cpShape *b = (swapped ? arb->a : arb->b);
//	cpContact *contacts = arb->contacts;
//	int numContacts = arb->numContacts;
//	cpFloat normal_coef = (swapped ? -1.0 : 1.0);
	
	// Now for some neat new stuff
	if(space->stamp == arb->stamp){ // if these are equal, the collision is brand new
		printf("Just touched a wall\n");
		// Need to come up with a name and some useful arguments, but arb->func() will
		// be called when the objects untouch.
		arb->separationFunc = untouch;
		arb->separationData = NULL; // This is passed to the separation func. Just showing it exists for the moment.
		
		// Let's register a post step callback to further process the arbiter
		cpSpaceAddPostStepCallback(space, (cpPostStepFunc)printCollisionImpulse, arb, space);
	} else {
		// This collision is not brand new, do something else maybe?
	}
	
	return 1;
}

static cpSpace *
init(void)
{
	staticBody = cpBodyNew(INFINITY, INFINITY);
	
	cpResetShapeIdCounter();
	
	space = cpSpaceNew();
	space->iterations = 20;
	cpSpaceResizeStaticHash(space, 40.0, 1000);
	cpSpaceResizeActiveHash(space, 40.0, 1000);
	space->gravity = cpv(0, -100);
	
	cpBody *body;
	cpShape *shape;
	
	int num = 4;
	cpVect verts[] = {
		cpv(-15,-15),
		cpv(-15, 15),
		cpv( 15, 15),
		cpv( 15,-15),
	};
	
	// Create segments around the edge of the screen.
	shape = cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	shape->layers = NOT_GRABABLE_MASK;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	shape->layers = NOT_GRABABLE_MASK;
	cpSpaceAddStaticShape(space, shape);

	shape = cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	shape->layers = NOT_GRABABLE_MASK;
	cpSpaceAddStaticShape(space, shape);
	
	// Add lots of boxes.
	for(int i=0; i<2; i++){
		for(int j=0; j<=i; j++){
			body = cpBodyNew(1.0, cpMomentForPoly(1.0, num, verts, cpvzero));
			body->p = cpv(j*32 - i*16, 300 - i*32);
			cpSpaceAddBody(space, body);
			shape = cpPolyShapeNew(body, num, verts, cpvzero);
			shape->e = 0.0; shape->u = 0.8;
			shape->collision_type = 1;
			cpSpaceAddShape(space, shape);
		}
	}
	
	// Add a ball to make things more interesting
	cpFloat radius = 15.0;
	body = cpBodyNew(10.0, cpMomentForCircle(10.0, 0.0, radius, cpvzero));
	body->p = cpv(0, -240 + radius+5);
//	cpSpaceAddBody(space, body);

	shape = cpCircleShapeNew(body, radius, cpvzero);
	shape->e = 0.0; shape->u = 0.9;
	shape->collision_type = 2;
	shape->sensor = 1;
	cpSpaceAddShape(space, shape);
	
	cpSpaceAddCollisionPairFunc(space, 1, 2, collision, NULL);
	
	return space;
}

static void
destroy(void)
{
	cpBodyFree(staticBody);
	cpSpaceFreeChildren(space);
	cpSpaceFree(space);
}

const chipmunkDemo PyramidStack = {
	"Pyramid Stack",
	NULL,
	init,
	update,
	destroy,
};
