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

// TODO
#include "chipmunk_private.h"
#include "ChipmunkDemo.h"

static cpSpace *space;

typedef struct PlayerStruct {
	cpFloat u;
	cpShape *shape;
} PlayerStruct;

PlayerStruct playerInstance;

static void
playerUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpBodyUpdateVelocity(body, gravity, damping, dt);
	body->v.y = cpfmax(body->v.y, -700);
	body->v.x = cpfclamp(body->v.x, -400, 400);
}

static void
SelectPlayerGroundNormal(cpBody *body, cpArbiter *arb, cpVect *groundNormal){
	cpVect n = cpArbiterGetNormal(arb, 0);
	
	if(n.y > groundNormal->y){
		*groundNormal = n;
	}
}

static void
update(int ticks)
{
	static int lastJumpState = 0;
	int jumpState = (ChipmunkDemoKeyboard.y > 0.0f);
	
	cpBody *body = playerInstance.shape->body;
	
	cpVect groundNormal = cpvzero;
	cpBodyEachArbiter(body, (cpBodyArbiterIteratorFunc)SelectPlayerGroundNormal, &groundNormal);
	
	if(groundNormal.y > 0.0f){
		playerInstance.shape->surface_v = cpv(400.0f*ChipmunkDemoKeyboard.x, 0.0f);//cpvmult(cpvperp(groundNormal), 400.0f*arrowDirection.x);
		if(ChipmunkDemoKeyboard.x) cpBodyActivate(body);
	} else {
		playerInstance.shape->surface_v = cpvzero;
	}
	
	// apply jump
	if(jumpState && !lastJumpState && cpvlengthsq(groundNormal)){
//		body->v = cpvmult(cpvslerp(groundNormal, cpv(0.0f, 1.0f), 0.5f), 500.0f);
		body->v = cpvadd(body->v, cpvmult(cpvslerp(groundNormal, cpv(0.0f, 1.0f), 0.75f), 500.0f));
		cpBodyActivate(body);
	}
	
	if(cpvlengthsq(groundNormal)){
		cpFloat air_accel = body->v.x + ChipmunkDemoKeyboard.x*(2000.0f);
		body->f.x = body->m*air_accel;
//		body->v.x = cpflerpconst(body->v.x, 400.0f*arrowDirection.x, 2000.0f/60.0f);
	}
	
	int steps = 3;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
	
	lastJumpState = jumpState;
}

static cpSpace *
init(void)
{
	space = cpSpaceNew();
	space->iterations = 10;
	space->gravity = cpv(0, -1500);
	space->sleepTimeThreshold = 999999;

	cpBody *body, *staticBody = space->staticBody;
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,240), cpv(320,240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;
	
	// add some other segments to play with
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-220,-200), cpv(-220,240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,-240), cpv(320,-200), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(200,-240), cpv(320,-100), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-220,-80), cpv(200,-80), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	shape->layers = NOT_GRABABLE_MASK;
	shape->collision_type = 2;
	
	// Set up the player
	cpFloat radius = 15.0f;
	body = cpSpaceAddBody(space, cpBodyNew(10.0f, INFINITY));
	body->p = cpv(0, -220);
	body->velocity_func = playerUpdateVelocity;

	shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	shape->e = 0.0f; shape->u = 2.0f;
	shape->collision_type = 1;
	
	playerInstance.u = shape->u;
	playerInstance.shape = shape;
	shape->data = &playerInstance;
	
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Player = {
	"Player",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
