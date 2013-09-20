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
 
#include <float.h>
#include <stdarg.h>

#include "chipmunk/chipmunk_private.h"

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
	
	body->userData = NULL;
	
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

// TODO static bodies should be addable
cpBody *
cpBodyInitStatic(cpBody *body)
{
	cpBodyInit(body, (cpFloat)INFINITY, (cpFloat)INFINITY);
	body->node.idleTime = (cpFloat)INFINITY;
	
	return body;
}

cpBody *
cpBodyNewStatic(void)
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

static void cpv_assert_nan(cpVect v, char *message){cpAssertHard(v.x == v.x && v.y == v.y, message);}
static void cpv_assert_infinite(cpVect v, char *message){cpAssertHard(cpfabs(v.x) != INFINITY && cpfabs(v.y) != INFINITY, message);}
static void cpv_assert_sane(cpVect v, char *message){cpv_assert_nan(v, message); cpv_assert_infinite(v, message);}

#ifdef __cplusplus
extern "C" {
#endif

void
cpBodySanityCheck(const cpBody *body)
{
	cpAssertHard(body->m == body->m && body->m_inv == body->m_inv, "Body's mass is NaN.");
	cpAssertHard(body->i == body->i && body->i_inv == body->i_inv, "Body's moment is NaN.");
	cpAssertHard(body->m >= 0.0f, "Body's mass is negative.");
	cpAssertHard(body->i >= 0.0f, "Body's moment is negative.");
	
	cpv_assert_sane(body->p, "Body's position is invalid.");
	cpv_assert_sane(body->v, "Body's velocity is invalid.");
	cpv_assert_sane(body->f, "Body's force is invalid.");

	cpAssertHard(body->a == body->a && cpfabs(body->a) != INFINITY, "Body's angle is invalid.");
	cpAssertHard(body->w == body->w && cpfabs(body->w) != INFINITY, "Body's angular velocity is invalid.");
	cpAssertHard(body->t == body->t && cpfabs(body->t) != INFINITY, "Body's torque is invalid.");
	
	cpAssertHard(body->v_limit == body->v_limit, "Body's velocity limit is invalid.");
	cpAssertHard(body->w_limit == body->w_limit, "Body's angular velocity limit is invalid.");
}

#ifdef __cplusplus
}
#endif

// Should only be called for a shape with mass info set.
void
cpBodyAccumulateMassForShape(cpBody *body, cpShape *shape)
{
	// Cache the position to realign it at the end.
	cpVect pos = cpBodyGetPosition(body);
	
	struct cpShapeMassInfo *info = &shape->massInfo;
	cpFloat msum = body->m + info->m;
	
	body->i += info->m*info->i + cpvdistsq(body->cog, info->cog)*(info->m*body->m)/msum;
	body->cog = cpvlerp(body->cog, info->cog, info->m/msum);
	body->m = msum;
	
	body->m_inv = 1.0f/body->m;
	body->i_inv = 1.0f/body->i;
	
	cpBodySetPosition(body, pos);
	cpAssertSaneBody(body);
}

// Should only be called when shapes with mass info are modified.
void
cpBodyAccumulateMass(cpBody *body)
{
	if(body == NULL) return;
	
	body->m = body->i = 0.0f;
	body->m_inv = body->i_inv = INFINITY;
	
	CP_BODY_FOREACH_SHAPE(body, shape){
		if(shape->massInfo.m > 0.0f) cpBodyAccumulateMassForShape(body, shape);
	}
}

void
cpBodySetMass(cpBody *body, cpFloat mass)
{
	cpAssertHard(mass >= 0.0f, "Mass must be positive.");
	
	cpBodyActivate(body);
	body->m = mass;
	body->m_inv = 1.0f/mass;
	cpAssertSaneBody(body);
}

void
cpBodySetMoment(cpBody *body, cpFloat moment)
{
	cpAssertHard(moment >= 0.0f, "Moment of Inertia must be positive.");
	
	cpBodyActivate(body);
	body->i = moment;
	body->i_inv = 1.0f/moment;
	cpAssertSaneBody(body);
}

cpVect
cpBodyGetRotation(const cpBody *body)
{
	return cpv(body->transform.a, body->transform.b);
}

void
cpBodyAddShape(cpBody *body, cpShape *shape)
{
	cpShape *next = body->shapeList;
	if(next) next->prev = shape;
	
	shape->next = next;
	body->shapeList = shape;
	
	if(shape->massInfo.m > 0.0f){
		cpBodyAccumulateMassForShape(body, shape);
	}
}

void
cpBodyRemoveShape(cpBody *body, cpShape *shape)
{
  cpShape *prev = shape->prev;
  cpShape *next = shape->next;
  
  if(prev){
		prev->next = next;
  } else {
		body->shapeList = next;
  }
  
  if(next){
		next->prev = prev;
	}
  
  shape->prev = NULL;
  shape->next = NULL;
	
	if(!cpBodyIsStatic(body) && shape->massInfo.m > 0.0f){
		cpBodyAccumulateMass(body);
	}
}

static cpConstraint *
filterConstraints(cpConstraint *node, cpBody *body, cpConstraint *filter)
{
	if(node == filter){
		return cpConstraintNext(node, body);
	} else if(node->a == body){
		node->next_a = filterConstraints(node->next_a, body, filter);
	} else {
		node->next_b = filterConstraints(node->next_b, body, filter);
	}
	
	return node;
}

void
cpBodyRemoveConstraint(cpBody *body, cpConstraint *constraint)
{
	body->constraintList = filterConstraints(body->constraintList, body, constraint);
}

void
SetTransform(cpBody *body, cpVect p, cpFloat a)
{
	cpVect rot = cpvforangle(a);
	cpVect c = body->cog;
	
	body->transform = cpTransformNewTranspose(
		rot.x, -rot.y, p.x - c.x*(rot.x + rot.y),
		rot.y,  rot.x, p.y - c.y*(rot.x - rot.y)
	);
}

static inline cpFloat
SetAngle(cpBody *body, cpFloat a)
{
	body->a = a;
	cpAssertSaneBody(body);
	
	return a;
}

void
cpBodySetPosition(cpBody *body, cpVect position)
{
	cpBodyActivate(body);
	cpVect p = body->p = cpvadd(cpTransformVect(body->transform, body->cog), position);
	cpAssertSaneBody(body);
	
	SetTransform(body, p, body->a);
}

void
cpBodySetAngle(cpBody *body, cpFloat angle)
{
	cpBodyActivate(body);
	SetAngle(body, angle);
	
	SetTransform(body, body->p, angle);
}

void
cpBodyUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	cpAssertSoft(body->m > 0.0f && body->i > 0.0f, "Body's mass and moment must be positive to simulate. (Mass: %f Moment: %f)", body->m, body->i);
	
	body->v = cpvclamp(cpvadd(cpvmult(body->v, damping), cpvmult(cpvadd(gravity, cpvmult(body->f, body->m_inv)), dt)), body->v_limit);
	
	cpFloat w_limit = body->w_limit;
	body->w = cpfclamp(body->w*damping + body->t*body->i_inv*dt, -w_limit, w_limit);
	
	// Reset forces.
	body->f = cpvzero;
	body->t = 0.0f;
	
	cpAssertSaneBody(body);
}

void
cpBodyUpdatePosition(cpBody *body, cpFloat dt)
{
	cpVect p = body->p = cpvadd(body->p, cpvmult(cpvadd(body->v, body->v_bias), dt));
	cpFloat a = SetAngle(body, body->a + (body->w + body->w_bias)*dt);
	SetTransform(body, p, a);
	
	body->v_bias = cpvzero;
	body->w_bias = 0.0f;
	
	cpAssertSaneBody(body);
}

void
cpBodyApplyForceAtWorldPoint(cpBody *body, cpVect force, cpVect point)
{
	cpBodyActivate(body);
	body->f = cpvadd(body->f, force);
	
	cpVect r = cpvsub(point, cpTransformPoint(body->transform, body->cog));
	body->t += cpvcross(r, force);
}

void
cpBodyApplyForceAtLocalPoint(cpBody *body, cpVect force, cpVect point)
{
	cpBodyApplyForceAtWorldPoint(body, cpTransformVect(body->transform, force), cpTransformPoint(body->transform, point));
}

void
cpBodyApplyImpulseAtWorldPoint(cpBody *body, cpVect impulse, cpVect point)
{
	cpBodyActivate(body);
	
	cpVect r = cpvsub(point, cpTransformPoint(body->transform, body->cog));
	apply_impulse(body, impulse, r);
}

void
cpBodyApplyImpulseAtLocalPoint(cpBody *body, cpVect impulse, cpVect point)
{
	cpBodyApplyImpulseAtWorldPoint(body, cpTransformVect(body->transform, impulse), cpTransformPoint(body->transform, point));
}

cpVect
cpBodyGetVelocityAtLocalPoint(const cpBody *body, cpVect point)
{
	cpVect r = cpTransformVect(body->transform, cpvsub(point, body->cog));
	return cpvadd(body->v, cpvmult(cpvperp(r), body->w));
}

cpVect
cpBodyGetVelocityAtWorldPoint(const cpBody *body, cpVect point)
{
	cpVect r = cpvsub(point, cpTransformPoint(body->transform, body->cog));
	return cpvadd(body->v, cpvmult(cpvperp(r), body->w));
}

void
cpBodyEachShape(cpBody *body, cpBodyShapeIteratorFunc func, void *data)
{
	cpShape *shape = body->shapeList;
	while(shape){
		cpShape *next = shape->next;
		func(body, shape, data);
		shape = next;
	}
}

void
cpBodyEachConstraint(cpBody *body, cpBodyConstraintIteratorFunc func, void *data)
{
	cpConstraint *constraint = body->constraintList;
	while(constraint){
		cpConstraint *next = cpConstraintNext(constraint, body);
		func(body, constraint, data);
		constraint = next;
	}
}

void
cpBodyEachArbiter(cpBody *body, cpBodyArbiterIteratorFunc func, void *data)
{
	cpArbiter *arb = body->arbiterList;
	while(arb){
		cpArbiter *next = cpArbiterNext(arb, body);
		
		cpBool swapped = arb->swapped; {
			arb->swapped = (body == arb->body_b);
			func(body, arb, data);
		} arb->swapped = swapped;
		
		arb = next;
	}
}
