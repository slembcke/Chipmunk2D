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
#include <float.h>
#include <math.h>

#include "chipmunk_private.h"
#include "constraints/util.h"

// initialized in cpInitChipmunk()
cpBody cpStaticBodySingleton;

cpBody*
cpBodyAlloc(void)
{
	return (cpBody *)cpcalloc(1, sizeof(cpBody));
}

cpBody *
cpBodyInit(cpBody *body, cpFloat m, cpFloat i)
{
	body->space = NULL;
	body->shapeList = NULL;
	body->arbiterList = NULL;
	body->constraintList = NULL;
	
	body->velocity_func = cpBodyUpdateVelocity;
	body->position_func = cpBodyUpdatePosition;
	
	cpComponentNode node = {NULL, NULL, 0.0f};
	body->node = node;
	
	body->p = cpvzero;
	body->v = cpvzero;
	body->f = cpvzero;
	
	body->w = 0.0f;
	body->t = 0.0f;
	
	body->v_bias = cpvzero;
	body->w_bias = 0.0f;
	
	body->v_limit = (cpFloat)INFINITY;
	body->w_limit = (cpFloat)INFINITY;
	
	body->data = NULL;
	
	// Setters must be called after full initialization so the sanity checks don't assert on garbage data.
	cpBodySetMass(body, m);
	cpBodySetMoment(body, i);
	cpBodySetAngle(body, 0.0f);
	
	return body;
}

cpBody*
cpBodyNew(cpFloat m, cpFloat i)
{
	return cpBodyInit(cpBodyAlloc(), m, i);
}

cpBody *
cpBodyInitStatic(cpBody *body)
{
	cpBodyInit(body, (cpFloat)INFINITY, (cpFloat)INFINITY);
	body->node.idleTime = (cpFloat)INFINITY;
	
	return body;
}

cpBody *
cpBodyNewStatic()
{
	return cpBodyInitStatic(cpBodyAlloc());
}

void cpBodyDestroy(cpBody *body){}

void
cpBodyFree(cpBody *body)
{
	if(body){
		cpBodyDestroy(body);
		cpfree(body);
	}
}

static void cpv_assert_nan(cpVect v, char *message){cpAssert(v.x == v.x && v.y == v.y, message);}
static void cpv_assert_infinite(cpVect v, char *message){cpAssert(cpfabs(v.x) != INFINITY && cpfabs(v.y) != INFINITY, message);}
static void cpv_assert_sane(cpVect v, char *message){cpv_assert_nan(v, message); cpv_assert_infinite(v, message);}

void
cpBodySanityCheck(cpBody *body)
{
	cpAssert(body->m == body->m && body->m_inv == body->m_inv, "Body's mass is invalid.");
	cpAssert(body->i == body->i && body->i_inv == body->i_inv, "Body's moment is invalid.");
	
	cpv_assert_sane(body->p, "Body's position is invalid.");
	cpv_assert_sane(body->v, "Body's velocity is invalid.");
	cpv_assert_sane(body->f, "Body's force is invalid.");

	cpAssert(body->a == body->a && cpfabs(body->a) != INFINITY, "Body's angle is invalid.");
	cpAssert(body->w == body->w && cpfabs(body->w) != INFINITY, "Body's angular velocity is invalid.");
	cpAssert(body->t == body->t && cpfabs(body->t) != INFINITY, "Body's torque is invalid.");
	
	cpv_assert_sane(body->rot, "Internal error: Body's rotation vector is invalid.");
	
	cpAssert(body->v_limit == body->v_limit, "Body's velocity limit is invalid.");
	cpAssert(body->w_limit == body->w_limit, "Body's angular velocity limit is invalid.");
}

void
cpBodySetMass(cpBody *body, cpFloat mass)
{
	cpBodyActivate(body);
	body->m = mass;
	body->m_inv = 1.0f/mass;
}

void
cpBodySetMoment(cpBody *body, cpFloat moment)
{
	cpBodyActivate(body);
	body->i = moment;
	body->i_inv = 1.0f/moment;
}

void
cpBodyAddShape(cpBody *body, cpShape *shape)
{
	shape->next = body->shapeList; body->shapeList = shape;
}

void
cpBodyRemoveShape(cpBody *body, cpShape *shape)
{
	cpShape **prev_ptr = &body->shapeList;
	cpShape *node = body->shapeList;
	
	while(node && node != shape){
		prev_ptr = &node->next;
		node = node->next;
	}
	
	cpAssert(node, "Attempted to remove a shape from a body it was never attached to.");
	(*prev_ptr) = node->next;
}

static inline void
updateShapes(cpBody *body){
	cpSpace *space = body->space;
	
	if(space){
		CP_BODY_FOREACH_SHAPE(body, shape) cpSpaceReindexShape(space, shape);
	}
}

void
cpBodySetPos(cpBody *body, cpVect pos)
{
	cpBodyActivate(body);
	cpBodyAssertSane(body);
	updateShapes(body);
	body->p = pos;
}

static inline void
setAngle(cpBody *body, cpFloat angle)
{
	body->a = angle;//fmod(a, (cpFloat)M_PI*2.0f);
	body->rot = cpvforangle(angle);
}

void
cpBodySetAngle(cpBody *body, cpFloat angle)
{
	cpBodyActivate(body);
	cpBodyAssertSane(body);
	updateShapes(body);
	setAngle(body, angle);
}

//void
//cpBodySlew(cpBody *body, cpVect pos, cpFloat dt)
//{
//	cpBodyActivate(body);
//	body->v = cpvmult(cpvsub(pos, body->p), 1.0f/dt);
//}

void
cpBodyUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	// TODO per body damping and gravity coefs
	body->v = cpvclamp(cpvadd(cpvmult(body->v, damping), cpvmult(cpvadd(gravity, cpvmult(body->f, body->m_inv)), dt)), body->v_limit);
	
	cpFloat w_limit = body->w_limit;
	body->w = cpfclamp(body->w*damping + body->t*body->i_inv*dt, -w_limit, w_limit);
	
	cpBodySanityCheck(body);
}

void
cpBodyUpdatePosition(cpBody *body, cpFloat dt)
{
	body->p = cpvadd(body->p, cpvmult(cpvadd(body->v, body->v_bias), dt));
	setAngle(body, body->a + (body->w + body->w_bias)*dt);
	
	body->v_bias = cpvzero;
	body->w_bias = 0.0f;
	
	cpBodySanityCheck(body);
}

void
cpBodyResetForces(cpBody *body)
{
	body->f = cpvzero;
	body->t = 0.0f;
}

void
cpBodyApplyForce(cpBody *body, cpVect force, cpVect r)
{
	body->f = cpvadd(body->f, force);
	body->t += cpvcross(r, force);
}

void
cpBodyApplyImpulse(cpBody *body, const cpVect j, const cpVect r)
{
	cpBodyActivate(body);
	apply_impulse(body, j, r);
}

void
cpBodyEachShape(cpBody *body, cpBodyShapeIteratorFunc func, void *data)
{
	CP_BODY_FOREACH_SHAPE(body, shape) func(body, shape, data);
}

void
cpBodyEachConstraint(cpBody *body, cpBodyConstraintIteratorFunc func, void *data)
{
	CP_BODY_FOREACH_CONSTRAINT(body,constraint) func(body, constraint, data);
}

void
cpBodyEachArbiter(cpBody *body, cpBodyArbiterIteratorFunc func, void *data)
{
	CP_BODY_FOREACH_ARBITER(body, arb){
		arb->swappedColl = (body == arb->body_b);
		func(body, arb, data);
	}
}
