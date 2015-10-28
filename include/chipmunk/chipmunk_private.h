/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
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

#ifndef CHIPMUNK_PRIVATE_H
#define CHIPMUNK_PRIVATE_H

#include "chipmunk/chipmunk.h"
#include "chipmunk/chipmunk_structs.h"

#define CP_HASH_COEF (3344921057ul)
#define CP_HASH_PAIR(A, B) ((cpHashValue)(A)*CP_HASH_COEF ^ (cpHashValue)(B)*CP_HASH_COEF)

// TODO: Eww. Magic numbers.
#define MAGIC_EPSILON 1e-5


//MARK: cpArray

cpArray *cpArrayNew(int size);

void cpArrayFree(cpArray *arr);

void cpArrayPush(cpArray *arr, void *object);
void *cpArrayPop(cpArray *arr);
void cpArrayDeleteObj(cpArray *arr, void *obj);
cpBool cpArrayContains(cpArray *arr, void *ptr);

void cpArrayFreeEach(cpArray *arr, void (freeFunc)(void*));


//MARK: cpHashSet

typedef cpBool (*cpHashSetEqlFunc)(void *ptr, void *elt);
typedef void *(*cpHashSetTransFunc)(void *ptr, void *data);

cpHashSet *cpHashSetNew(int size, cpHashSetEqlFunc eqlFunc);
void cpHashSetSetDefaultValue(cpHashSet *set, void *default_value);

void cpHashSetFree(cpHashSet *set);

int cpHashSetCount(cpHashSet *set);
void *cpHashSetInsert(cpHashSet *set, cpHashValue hash, void *ptr, cpHashSetTransFunc trans, void *data);
void *cpHashSetRemove(cpHashSet *set, cpHashValue hash, void *ptr);
void *cpHashSetFind(cpHashSet *set, cpHashValue hash, void *ptr);

typedef void (*cpHashSetIteratorFunc)(void *elt, void *data);
void cpHashSetEach(cpHashSet *set, cpHashSetIteratorFunc func, void *data);

typedef cpBool (*cpHashSetFilterFunc)(void *elt, void *data);
void cpHashSetFilter(cpHashSet *set, cpHashSetFilterFunc func, void *data);


//MARK: Bodies

void cpBodyAddShape(cpBody *body, cpShape *shape);
void cpBodyRemoveShape(cpBody *body, cpShape *shape);

//void cpBodyAccumulateMassForShape(cpBody *body, cpShape *shape);
void cpBodyAccumulateMassFromShapes(cpBody *body);

void cpBodyRemoveConstraint(cpBody *body, cpConstraint *constraint);


//MARK: Spatial Index Functions

cpSpatialIndex *cpSpatialIndexInit(cpSpatialIndex *index, cpSpatialIndexClass *klass, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);


//MARK: Arbiters

cpArbiter* cpArbiterInit(cpArbiter *arb, cpShape *a, cpShape *b);

static inline struct cpArbiterThread *
cpArbiterThreadForBody(cpArbiter *arb, cpBody *body)
{
	return (arb->body_a == body ? &arb->thread_a : &arb->thread_b);
}

void cpArbiterUnthread(cpArbiter *arb);

void cpArbiterUpdate(cpArbiter *arb, struct cpCollisionInfo *info, cpSpace *space);
void cpArbiterPreStep(cpArbiter *arb, cpFloat dt, cpFloat bias, cpFloat slop);
void cpArbiterApplyCachedImpulse(cpArbiter *arb, cpFloat dt_coef);
void cpArbiterApplyImpulse(cpArbiter *arb);


//MARK: Shapes/Collisions

cpShape *cpShapeInit(cpShape *shape, const cpShapeClass *klass, cpBody *body, struct cpShapeMassInfo massInfo);

static inline cpBool
cpShapeActive(cpShape *shape)
{
	// checks if the shape is added to a shape list.
	// TODO could this just check the space now?
	return (shape->prev || (shape->body && shape->body->shapeList == shape));
}

// Note: This function returns contact points with r1/r2 in absolute coordinates, not body relative.
struct cpCollisionInfo cpCollide(const cpShape *a, const cpShape *b, cpCollisionID id, struct cpContact *contacts);

static inline void
CircleSegmentQuery(cpShape *shape, cpVect center, cpFloat r1, cpVect a, cpVect b, cpFloat r2, cpSegmentQueryInfo *info)
{
	cpVect da = cpvsub(a, center);
	cpVect db = cpvsub(b, center);
	cpFloat rsum = r1 + r2;
	
	cpFloat qa = cpvdot(da, da) - 2.0f*cpvdot(da, db) + cpvdot(db, db);
	cpFloat qb = cpvdot(da, db) - cpvdot(da, da);
	cpFloat det = qb*qb - qa*(cpvdot(da, da) - rsum*rsum);
	
	if(det >= 0.0f){
		cpFloat t = (-qb - cpfsqrt(det))/(qa);
		if(0.0f<= t && t <= 1.0f){
			cpVect n = cpvnormalize(cpvlerp(da, db, t));
			
			info->shape = shape;
			info->point = cpvsub(cpvlerp(a, b, t), cpvmult(n, r2));
			info->normal = n;
			info->alpha = t;
		}
	}
}

static inline cpBool
cpShapeFilterReject(cpShapeFilter a, cpShapeFilter b)
{
	// Reject the collision if:
	return (
		// They are in the same non-zero group.
		(a.group != 0 && a.group == b.group) ||
		// One of the category/mask combinations fails.
		(a.categories & b.mask) == 0 ||
		(b.categories & a.mask) == 0
	);
}

void cpLoopIndexes(const cpVect *verts, int count, int *start, int *end);


//MARK: Constraints
// TODO naming conventions here

void cpConstraintInit(cpConstraint *constraint, const struct cpConstraintClass *klass, cpBody *a, cpBody *b);

static inline void
cpConstraintActivateBodies(cpConstraint *constraint)
{
	cpBody *a = constraint->a; cpBodyActivate(a);
	cpBody *b = constraint->b; cpBodyActivate(b);
}

static inline cpVect
relative_velocity(cpBody *a, cpBody *b, cpVect r1, cpVect r2){
	cpVect v1_sum = cpvadd(a->v, cpvmult(cpvperp(r1), a->w));
	cpVect v2_sum = cpvadd(b->v, cpvmult(cpvperp(r2), b->w));
	
	return cpvsub(v2_sum, v1_sum);
}

static inline cpFloat
normal_relative_velocity(cpBody *a, cpBody *b, cpVect r1, cpVect r2, cpVect n){
	return cpvdot(relative_velocity(a, b, r1, r2), n);
}

static inline void
apply_impulse(cpBody *body, cpVect j, cpVect r){
	body->v = cpvadd(body->v, cpvmult(j, body->m_inv));
	body->w += body->i_inv*cpvcross(r, j);
}

static inline void
apply_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j)
{
	apply_impulse(a, cpvneg(j), r1);
	apply_impulse(b, j, r2);
}

static inline void
apply_bias_impulse(cpBody *body, cpVect j, cpVect r)
{
	body->v_bias = cpvadd(body->v_bias, cpvmult(j, body->m_inv));
	body->w_bias += body->i_inv*cpvcross(r, j);
}

static inline void
apply_bias_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j)
{
	apply_bias_impulse(a, cpvneg(j), r1);
	apply_bias_impulse(b, j, r2);
}

static inline cpFloat
k_scalar_body(cpBody *body, cpVect r, cpVect n)
{
	cpFloat rcn = cpvcross(r, n);
	return body->m_inv + body->i_inv*rcn*rcn;
}

static inline cpFloat
k_scalar(cpBody *a, cpBody *b, cpVect r1, cpVect r2, cpVect n)
{
	cpFloat value = k_scalar_body(a, r1, n) + k_scalar_body(b, r2, n);
	cpAssertSoft(value != 0.0, "Unsolvable collision or constraint.");
	
	return value;
}

static inline cpMat2x2
k_tensor(cpBody *a, cpBody *b, cpVect r1, cpVect r2)
{
	cpFloat m_sum = a->m_inv + b->m_inv;
	
	// start with Identity*m_sum
	cpFloat k11 = m_sum, k12 = 0.0f;
	cpFloat k21 = 0.0f,  k22 = m_sum;
	
	// add the influence from r1
	cpFloat a_i_inv = a->i_inv;
	cpFloat r1xsq =  r1.x * r1.x * a_i_inv;
	cpFloat r1ysq =  r1.y * r1.y * a_i_inv;
	cpFloat r1nxy = -r1.x * r1.y * a_i_inv;
	k11 += r1ysq; k12 += r1nxy;
	k21 += r1nxy; k22 += r1xsq;
	
	// add the influnce from r2
	cpFloat b_i_inv = b->i_inv;
	cpFloat r2xsq =  r2.x * r2.x * b_i_inv;
	cpFloat r2ysq =  r2.y * r2.y * b_i_inv;
	cpFloat r2nxy = -r2.x * r2.y * b_i_inv;
	k11 += r2ysq; k12 += r2nxy;
	k21 += r2nxy; k22 += r2xsq;
	
	// invert
	cpFloat det = k11*k22 - k12*k21;
	cpAssertSoft(det != 0.0, "Unsolvable constraint.");
	
	cpFloat det_inv = 1.0f/det;
	return cpMat2x2New(
		 k22*det_inv, -k12*det_inv,
		-k21*det_inv,  k11*det_inv
 	);
}

static inline cpFloat
bias_coef(cpFloat errorBias, cpFloat dt)
{
	return 1.0f - cpfpow(errorBias, dt);
}


//MARK: Spaces

#define cpAssertSpaceUnlocked(space) \
	cpAssertHard(!space->locked, \
		"This operation cannot be done safely during a call to cpSpaceStep() or during a query. " \
		"Put these calls into a post-step callback." \
	);

void cpSpaceSetStaticBody(cpSpace *space, cpBody *body);

extern cpCollisionHandler cpCollisionHandlerDoNothing;

void cpSpaceProcessComponents(cpSpace *space, cpFloat dt);

void cpSpacePushFreshContactBuffer(cpSpace *space);
struct cpContact *cpContactBufferGetArray(cpSpace *space);
void cpSpacePushContacts(cpSpace *space, int count);

cpPostStepCallback *cpSpaceGetPostStepCallback(cpSpace *space, void *key);

cpBool cpSpaceArbiterSetFilter(cpArbiter *arb, cpSpace *space);
void cpSpaceFilterArbiters(cpSpace *space, cpBody *body, cpShape *filter);

void cpSpaceActivateBody(cpSpace *space, cpBody *body);
void cpSpaceLock(cpSpace *space);
void cpSpaceUnlock(cpSpace *space, cpBool runPostStep);

static inline void
cpSpaceUncacheArbiter(cpSpace *space, cpArbiter *arb)
{
	const cpShape *a = arb->a, *b = arb->b;
	const cpShape *shape_pair[] = {a, b};
	cpHashValue arbHashID = CP_HASH_PAIR((cpHashValue)a, (cpHashValue)b);
	cpHashSetRemove(space->cachedArbiters, arbHashID, shape_pair);
	cpArrayDeleteObj(space->arbiters, arb);
}

static inline cpArray *
cpSpaceArrayForBodyType(cpSpace *space, cpBodyType type)
{
	return (type == CP_BODY_TYPE_STATIC ? space->staticBodies : space->dynamicBodies);
}

void cpShapeUpdateFunc(cpShape *shape, void *unused);
cpCollisionID cpSpaceCollideShapes(cpShape *a, cpShape *b, cpCollisionID id, cpSpace *space);


//MARK: Foreach loops

static inline cpConstraint *
cpConstraintNext(cpConstraint *node, cpBody *body)
{
	return (node->a == body ? node->next_a : node->next_b);
}

#define CP_BODY_FOREACH_CONSTRAINT(bdy, var)\
	for(cpConstraint *var = bdy->constraintList; var; var = cpConstraintNext(var, bdy))

static inline cpArbiter *
cpArbiterNext(cpArbiter *node, cpBody *body)
{
	return (node->body_a == body ? node->thread_a.next : node->thread_b.next);
}

#define CP_BODY_FOREACH_ARBITER(bdy, var)\
	for(cpArbiter *var = bdy->arbiterList; var; var = cpArbiterNext(var, bdy))

#define CP_BODY_FOREACH_SHAPE(body, var)\
	for(cpShape *var = body->shapeList; var; var = var->next)

#define CP_BODY_FOREACH_COMPONENT(root, var)\
	for(cpBody *var = root; var; var = var->sleeping.next)

#endif
