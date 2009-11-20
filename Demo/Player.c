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

typedef struct PlayerStruct {
	cpFloat u;
	int contacts;
	cpShape *shape;
} PlayerStruct;

PlayerStruct playerInstance;

static void
separate(cpArbiter *arb, cpSpace *space, void *ignore)
{
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	PlayerStruct *player = a->data;
	
	player->contacts -= 1;
	
	if(!player->contacts){
		a->u = 0.0f;
	}
}

static int
collision(cpArbiter *arb, cpSpace *space, void *ignore)
{
	// Set a separate function so we can reset the pass thru state of the pair.
	arb->separationFunc = separate;
	
	cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);
	PlayerStruct *player = a->data;
	
	if(arb->stamp == space->stamp){
		player->contacts += 1;
	} else {
		a->u = player->u;
	}
	
	return 1;
}

static void
update(int ticks)
{
	playerInstance.shape->surface_v = cpv(-400*arrowDirection.x, 0);
	
	if(arrowDirection.y > 0.0 && playerInstance.contacts != 0){
		playerInstance.shape->body->v.y += 500.0;
	}
	
	int steps = 1;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
	staticBody = cpBodyNew(INFINITY, INFINITY);
	
	cpResetShapeIdCounter();
	
	space = cpSpaceNew();
	space->iterations = 10;
	cpSpaceResizeStaticHash(space, 40.0, 1000);
	cpSpaceResizeActiveHash(space, 40.0, 1000);
	space->gravity = cpv(0, -1000);
	
	cpBody *body;
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddStaticShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	shape->e = 1.0; shape->u = 1.0;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;

	shape = cpSpaceAddStaticShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	shape->e = 1.0; shape->u = 1.0;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;

	shape = cpSpaceAddStaticShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	shape->e = 1.0; shape->u = 1.0;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;
	
	// Add a ball to make things more interesting
	cpFloat radius = 15.0;
	body = cpSpaceAddBody(space, cpBodyNew(10.0, INFINITY));
	body->p = cpv(0, -240 + radius);

	shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	shape->e = 0.0; shape->u = 3.0;
	shape->collision_type = 1;
	
	playerInstance.u = shape->u;
	playerInstance.contacts = 0;
	playerInstance.shape = shape;
	shape->data = &playerInstance;
	
	cpSpaceAddCollisionPairFunc(space, 1, 2, (cpCollFunc)collision, NULL);
	
	return space;
}

static void
destroy(void)
{
	cpBodyFree(staticBody);
	cpSpaceFreeChildren(space);
	cpSpaceFree(space);
}

const chipmunkDemo Player = {
	"Player",
	NULL,
	init,
	update,
	destroy,
};
