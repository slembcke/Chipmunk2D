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
 
#include "chipmunk/chipmunk_private.h"
#include "ChipmunkDemo.h"

#define PLAYER_VELOCITY 500.0

#define PLAYER_GROUND_ACCEL_TIME 0.1
#define PLAYER_GROUND_ACCEL (PLAYER_VELOCITY/PLAYER_GROUND_ACCEL_TIME)

#define PLAYER_AIR_ACCEL_TIME 0.25
#define PLAYER_AIR_ACCEL (PLAYER_VELOCITY/PLAYER_AIR_ACCEL_TIME)

#define JUMP_HEIGHT 50.0
#define JUMP_BOOST_HEIGHT 55.0
#define FALL_VELOCITY 900.0
#define GRAVITY 2000.0

typedef struct cpCustomContact {
	cpConstraint constraint;
	cpArbiter *arb;
	
	cpFloat maxFriction;
} cpCustomContact;

static void
preStep(cpCustomContact *custom, cpFloat dt)
{
	cpArbiter *arb = custom->arb;
	cpFloat bias = 0.1f;
	cpFloat slop = 0.1f;
	
	cpBody *a = arb->body_a;
	cpBody *b = arb->body_b;
	cpVect n = arb->n;
	cpVect body_delta = cpvsub(b->p, a->p);
	
	for(int i=0; i<arb->count; i++){
		struct cpContact *con = &arb->contacts[i];
		
		// Calculate the mass normal and mass tangent.
		con->nMass = 1.0f/k_scalar(a, b, con->r1, con->r2, n);
		con->tMass = 1.0f/k_scalar(a, b, con->r1, con->r2, cpvperp(n));
				
		// Calculate the target bias velocity.
		cpFloat dist = cpvdot(cpvadd(cpvsub(con->r2, con->r1), body_delta), n);
		con->bias = -bias*cpfmin(0.0f, dist + slop)/dt;
		con->jBias = 0.0f;
		
		// Calculate the target bounce velocity.
		con->bounce = normal_relative_velocity(a, b, con->r1, con->r2, n)*arb->e;
	}
}

static void
applyCachedImpulse(cpCustomContact *custom, cpFloat dt_coef)
{
	cpArbiter *arb = custom->arb;
	
	if(cpArbiterIsFirstContact(arb)) return;
	
	cpBody *a = arb->body_a;
	cpBody *b = arb->body_b;
	cpVect n = arb->n;
	
	for(int i=0; i<arb->count; i++){
		struct cpContact *con = &arb->contacts[i];
		cpVect j = cpvrotate(n, cpv(con->jnAcc, con->jtAcc));
		apply_impulses(a, b, con->r1, con->r2, cpvmult(j, dt_coef));
	}
}

static void
applyImpulse(cpCustomContact *custom, cpFloat dt)
{
	cpArbiter *arb = custom->arb;
	cpBody *a = arb->body_a;
	cpBody *b = arb->body_b;
	cpVect n = arb->n;
	cpVect surface_vr = arb->surface_vr;
	cpFloat friction = arb->u;

	for(int i=0; i<arb->count; i++){
		struct cpContact *con = &arb->contacts[i];
		cpFloat nMass = con->nMass;
		cpVect r1 = con->r1;
		cpVect r2 = con->r2;
		
		cpVect vb1 = cpvadd(a->v_bias, cpvmult(cpvperp(r1), a->w_bias));
		cpVect vb2 = cpvadd(b->v_bias, cpvmult(cpvperp(r2), b->w_bias));
		cpVect vr = cpvadd(relative_velocity(a, b, r1, r2), surface_vr);
		
		cpFloat vbn = cpvdot(cpvsub(vb2, vb1), n);
		cpFloat vrn = cpvdot(vr, n);
		cpFloat vrt = cpvdot(vr, cpvperp(n));
		
		cpFloat jbn = (con->bias - vbn)*nMass;
		cpFloat jbnOld = con->jBias;
		con->jBias = cpfmax(jbnOld + jbn, 0.0f);
		
		cpFloat jn = -(con->bounce + vrn)*nMass;
		cpFloat jnOld = con->jnAcc;
		con->jnAcc = cpfmax(jnOld + jn, 0.0f);
		
		cpFloat jtMax = cpfmin(friction*con->jnAcc, custom->maxFriction*dt);
		cpFloat jt = -vrt*con->tMass;
		cpFloat jtOld = con->jtAcc;
		con->jtAcc = cpfclamp(jtOld + jt, -jtMax, jtMax);
		
		apply_bias_impulses(a, b, r1, r2, cpvmult(n, con->jBias - jbnOld));
		apply_impulses(a, b, r1, r2, cpvrotate(n, cpv(con->jnAcc - jnOld, con->jtAcc - jtOld)));
	}
}

static cpFloat
getImpulse(cpCustomContact *custom)
{
	return cpvlength(cpArbiterTotalImpulse(custom->arb));
}

static const cpConstraintClass klass = {
	(cpConstraintPreStepImpl)preStep,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulse,
	(cpConstraintApplyImpulseImpl)applyImpulse,
	(cpConstraintGetImpulseImpl)getImpulse,
};

static cpCustomContact *
cpCustomContactAlloc(void)
{
	return (cpCustomContact *)cpcalloc(1, sizeof(cpCustomContact));
}

static cpCustomContact *
cpCustomContactInit(cpCustomContact *custom, cpArbiter *arb)
{
	CP_ARBITER_GET_BODIES(arb, a, b);
	cpConstraintInit((cpConstraint *)custom, &klass, a, b);
	
	custom->arb = arb;
	custom->maxFriction = INFINITY;
	
	return custom;
}

static cpCustomContact *
cpCustomContactNew(cpArbiter *arb)
{
	return cpCustomContactInit(cpCustomContactAlloc(), arb);
}

// --------

static cpBody *playerBody = NULL;
static cpShape *playerShape = NULL;

static cpFloat remainingBoost = 0;
static cpBool grounded = cpFalse;
static cpBool lastJumpState = cpFalse;

static void
SelectPlayerGroundNormal(cpBody *body, cpArbiter *arb, cpVect *groundNormal){
	cpVect n = cpvneg(cpArbiterGetNormal(arb));
	
	if(n.y > groundNormal->y){
		(*groundNormal) = n;
	}
}

static void
playerUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	int jumpState = (ChipmunkDemoKeyboard.y > 0.0f);
	
	// Grab the grounding normal from last frame
	cpVect groundNormal = cpvzero;
	cpBodyEachArbiter(playerBody, (cpBodyArbiterIteratorFunc)SelectPlayerGroundNormal, &groundNormal);
	
	grounded = (groundNormal.y > 0.0);
	if(groundNormal.y < 0.0f) remainingBoost = 0.0f;
	
	// Do a normal-ish update
	cpBool boost = (jumpState && remainingBoost > 0.0f);
	cpVect g = (boost ? cpvzero : gravity);
	cpBodyUpdateVelocity(body, g, damping, dt);
	
	// Target horizontal speed for air/ground control
	cpFloat target_vx = PLAYER_VELOCITY*ChipmunkDemoKeyboard.x;
	
	// Update the surface velocity and friction
	// Note that the "feet" move in the opposite direction of the player.
	cpVect surface_v = cpv(-target_vx, 0.0);
	playerShape->surfaceV = surface_v;
	
	// Apply air control if not grounded
	if(!grounded){
		// Smoothly accelerate the velocity
		playerBody->v.x = cpflerpconst(playerBody->v.x, target_vx, PLAYER_AIR_ACCEL*dt);
	}
	
	body->v.y = cpfclamp(body->v.y, -FALL_VELOCITY, INFINITY);
}

static void
update(cpSpace *space, double dt)
{
	int jumpState = (ChipmunkDemoKeyboard.y > 0.0f);
	
	// If the jump key was just pressed this frame, jump!
	if(jumpState && !lastJumpState && grounded){
		cpFloat jump_v = cpfsqrt(2.0*JUMP_HEIGHT*GRAVITY);
		playerBody->v = cpvadd(playerBody->v, cpv(0.0, jump_v));
		
		remainingBoost = JUMP_BOOST_HEIGHT/jump_v;
	}
	
	// Step the space
	cpSpaceStep(space, dt);
	
	remainingBoost -= dt;
	lastJumpState = jumpState;
}

static cpBool
Begin(cpArbiter *arb, cpSpace *space, void *ptr)
{
	CP_ARBITER_GET_BODIES(arb, player, other);
	
	cpCustomContact *custom = cpCustomContactNew(arb);
	custom->maxFriction = PLAYER_GROUND_ACCEL*cpBodyGetMass(player);
	arb->customContact = (cpConstraint *)custom;
	
	
	return cpTrue;
}

static cpBool
PreSolve(cpArbiter *arb, cpSpace *space, void *ptr){
	cpFloat friction = cpfclamp01(-cpArbiterGetNormal(arb).y);
	cpArbiterSetFriction(arb, friction);
	
	return cpTrue;
}

static cpSpace *
init(void)
{
	cpSpace *space = cpSpaceNew();
	space->iterations = 10;
	space->gravity = cpv(0, -GRAVITY);
//	space->sleepTimeThreshold = 1000;

	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,240), cpv(320,240), 0.0f));
	shape->e = 1.0f; shape->u = 1.0f;
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	// Set up the player
	body = cpSpaceAddBody(space, cpBodyNew(1.0f, INFINITY));
	body->p = cpv(0, -200);
	body->velocity_func = playerUpdateVelocity;
	playerBody = body;

	shape = cpSpaceAddShape(space, cpBoxShapeNew2(body, cpBBNew(-15.0, -27.5, 15.0, 27.5), 10.0));
//	shape = cpSpaceAddShape(space, cpSegmentShapeNew(playerBody, cpvzero, cpv(0, radius), radius));
	shape->e = 0.0f; shape->u = 1.0f;
	shape->type = 1;
	playerShape = shape;
	
	// Add some boxes to jump on
	for(int i=0; i<6; i++){
		for(int j=0; j<3; j++){
			body = cpSpaceAddBody(space, cpBodyNew(4.0f, INFINITY));
			body->p = cpv(100 + j*60, -200 + i*60);
			
			shape = cpSpaceAddShape(space, cpBoxShapeNew(body, 50, 50, 0.0));
			shape->e = 0.0f; shape->u = 0.7f;
		}
	}
	
	cpCollisionHandler *handler = cpSpaceAddWildcardHandler(space, 1);
	handler->beginFunc = (cpCollisionBeginFunc)Begin;
	handler->preSolveFunc = (cpCollisionPreSolveFunc)PreSolve;
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Player = {
	"Platformer Player Controls",
	1.0/180.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
