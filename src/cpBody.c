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

cpBody*
cpBodyAlloc(void)
{
	return (cpBody *)cpcalloc(1, sizeof(cpBody));
}

cpBody *
cpBodyInit(cpBody *body, cpFloat mass, cpFloat moment)
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
	cpBodySetMass(body, mass);
	cpBodySetMoment(body, moment);
	cpBodySetAngle(body, 0.0f);
	
	return body;
}

cpBody*
cpBodyNew(cpFloat mass, cpFloat moment)
{
	return cpBodyInit(cpBodyAlloc(), mass, moment);
}

cpBody*
cpBodyNewKinematic()
{
	cpBody *body = cpBodyNew(0.0f, 0.0f);
	cpBodySetType(body, CP_BODY_TYPE_KINEMATIC);
	
	return body;
}

cpBody*
cpBodyNewStatic()
{
	cpBody *body = cpBodyNew(0.0f, 0.0f);
	cpBodySetType(body, CP_BODY_TYPE_STATIC);
	
	return body;
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


void
cpBodySetType(cpBody *body, cpBodyType type)
{
	cpBodyType oldType = cpBodyGetType(body);
	if(oldType == type) return;
	
	// Static bodies have their idle timers set to infinity.
	// Non-static bodies should have their idle timer reset.
	body->node.idleTime = (type == CP_BODY_TYPE_STATIC ? INFINITY : 0.0f);
	
	if(type == CP_BODY_TYPE_DYNAMIC){
		body->m = body->i = 0.0f;
		body->m_inv = body->i_inv = INFINITY;
		
		cpBodyAccumulateMassFromShapes(body);
	} else {
		body->m = body->i = INFINITY;
		body->m_inv = body->i_inv = 0.0f;
		
		body->v = cpvzero;
		body->w = 0.0f;
	}
	
	// If the body is added to a space already, we'll need to update some space data structures.
	cpSpace *space = cpBodyGetSpace(body);
	if(space != NULL){
		cpAssertSpaceUnlocked(space);
		
		if(oldType == CP_BODY_TYPE_STATIC){
			// TODO This is probably not necessary
//			cpBodyActivateStatic(body, NULL);
		} else {
			cpBodyActivate(body);
		}
		
		// Move the bodies to the correct array.
		cpArray *fromArray = (oldType == CP_BODY_TYPE_DYNAMIC ? space->dynamicBodies : space->otherBodies);
		cpArray *toArray = (type == CP_BODY_TYPE_DYNAMIC ? space->dynamicBodies : space->otherBodies);
		if(fromArray != toArray){
			cpArrayDeleteObj(fromArray, body);
			cpArrayPush(toArray, body);
		}
		
		// Move the body's shapes to the correct spatial index.
		cpSpatialIndex *fromIndex = (oldType == CP_BODY_TYPE_STATIC ? space->staticShapes : space->dynamicShapes);
		cpSpatialIndex *toIndex = (type == CP_BODY_TYPE_STATIC ? space->staticShapes : space->dynamicShapes);
		if(fromIndex != toIndex){
			CP_BODY_FOREACH_SHAPE(body, shape){
				cpSpatialIndexRemove(fromIndex, shape, shape->hashid);
				cpSpatialIndexInsert(toIndex, shape, shape->hashid);
			}
		}
	}
}


// Should *only* be called when shapes with mass info are modified, added or removed.
void
cpBodyAccumulateMassFromShapes(cpBody *body)
{
	if(body == NULL || !cpBodyIsDynamic(body)) return;
	
	// Reset the body's mass data.
	body->m = body->i = 0.0f;
	body->cog = cpvzero;
	
	// Cache the position to realign it at the end.
	cpVect pos = cpBodyGetPosition(body);
	
	// Accumulate mass from shapes.
	CP_BODY_FOREACH_SHAPE(body, shape){
		struct cpShapeMassInfo *info = &shape->massInfo;
		cpFloat m = info->m;
		
		if(m > 0.0f){
			cpFloat msum = body->m + m;
			
			body->i += m*info->i + cpvdistsq(body->cog, info->cog)*(m*body->m)/msum;
			body->cog = cpvlerp(body->cog, info->cog, m/msum);
			body->m = msum;
		}
	}
	
	// Recalculate the inverses.
	body->m_inv = 1.0f/body->m;
	body->i_inv = 1.0f/body->i;
	
	// Realign the body since the CoG has probably moved.
	cpBodySetPosition(body, pos);
	cpAssertSaneBody(body);
}

void
cpBodySetMass(cpBody *body, cpFloat mass)
{
	cpAssertHard(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC, "You cannot set the mass of kinematic or static bodies.");
	cpAssertHard(0.0f <= mass && mass < INFINITY, "Mass must be positive and finite.");
	
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
		cpBodyAccumulateMassFromShapes(body);
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
	
	if(cpBodyIsDynamic(body) && shape->massInfo.m > 0.0f){
		cpBodyAccumulateMassFromShapes(body);
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

// 'p' is the position of the CoG
void
SetTransform(cpBody *body, cpVect p, cpFloat a)
{
	cpVect rot = cpvforangle(a);
	cpVect c = body->cog;
	
	body->transform = cpTransformNewTranspose(
		rot.x, -rot.y, p.x - (c.x*rot.x - c.y*rot.y),
		rot.y,  rot.x, p.y - (c.x*rot.y + c.y*rot.x)
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
