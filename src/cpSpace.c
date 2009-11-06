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
#include <string.h>
#include <math.h>
#include <assert.h>
#ifdef _MSC_VER
	#include <malloc.h>
#endif

#include "chipmunk.h"

int cp_contact_persistence = 3;

#pragma mark Contact Set Helpers

// Equal function for contactSet.
static int
contactSetEql(cpShape **shapes, cpArbiter *arb)
{
	cpShape *a = shapes[0];
	cpShape *b = shapes[1];
	
	return ((a == arb->a && b == arb->b) || (b == arb->a && a == arb->b));
}

// Transformation function for contactSet.
static void *
contactSetTrans(cpShape **shapes, cpSpace *space)
{
	cpShape *a = shapes[0];
	cpShape *b = shapes[1];
	
	return cpArbiterNew(a, b, space->stamp);
}

#pragma mark Collision Pair Function Helpers

// Collision pair function wrapper struct.
typedef struct collFuncData {
	cpCollFunc func;
	void *data;
} collFuncData;

// Equals function for collFuncSet.
static int
collFuncSetEql(cpCollisionType *ids, cpCollPairFunc *pair)
{
	cpCollisionType a = ids[0];
	cpCollisionType b = ids[1];
	
	return ((a == pair->a && b == pair->b) || (b == pair->a && a == pair->b));
}

// Transformation function for collFuncSet.
static void *
collFuncSetTrans(cpCollisionType *ids, collFuncData *funcData)
{
	cpCollPairFunc *pair = (cpCollPairFunc *)malloc(sizeof(cpCollPairFunc));
	pair->a = ids[0];
	pair->b = ids[1];
	pair->func = funcData->func;
	pair->data = funcData->data;

	return pair;
}

#pragma mark Misc Helper Funcs

// Default collision pair function.
static int
alwaysCollide(cpShape *a, cpShape *b, cpContact *arr, int numCon, cpFloat normal_coef, void *data)
{
	return 1;
}

// BBfunc callback for the spatial hash.
static cpBB shapeBBFunc(cpShape *shape){return shape->bb;}

// Iterator functions for destructors.
static void             freeWrap(void         *ptr, void *unused){            free(ptr);}
static void        shapeFreeWrap(cpShape      *ptr, void *unused){     cpShapeFree(ptr);}
static void      arbiterFreeWrap(cpArbiter    *ptr, void *unused){   cpArbiterFree(ptr);}
static void         bodyFreeWrap(cpBody       *ptr, void *unused){      cpBodyFree(ptr);}
static void   constraintFreeWrap(cpConstraint *ptr, void *unused){cpConstraintFree(ptr);}

#pragma mark Memory Management Functions

cpSpace*
cpSpaceAlloc(void)
{
	return (cpSpace *)calloc(1, sizeof(cpSpace));
}

#define DEFAULT_DIM_SIZE 100.0f
#define DEFAULT_COUNT 1000
#define DEFAULT_ITERATIONS 10
#define DEFAULT_ELASTIC_ITERATIONS 0

cpSpace*
cpSpaceInit(cpSpace *space)
{
	space->iterations = DEFAULT_ITERATIONS;
	space->elasticIterations = DEFAULT_ELASTIC_ITERATIONS;
//	space->sleepTicks = 300;
	
	space->gravity = cpvzero;
	space->damping = 1.0f;
	
	space->stamp = 0;

	space->staticShapes = cpSpaceHashNew(DEFAULT_DIM_SIZE, DEFAULT_COUNT, (cpSpaceHashBBFunc)shapeBBFunc);
	space->activeShapes = cpSpaceHashNew(DEFAULT_DIM_SIZE, DEFAULT_COUNT, (cpSpaceHashBBFunc)shapeBBFunc);
	
	space->bodies = cpArrayNew(0);
	space->arbiters = cpArrayNew(0);
	space->contactSet = cpHashSetNew(0, (cpHashSetEqlFunc)contactSetEql, (cpHashSetTransFunc)contactSetTrans);
	
	space->constraints = cpArrayNew(0);
	
	cpCollPairFunc pairFunc = {0, 0, alwaysCollide, NULL};
	space->defaultPairFunc = pairFunc;
	space->collFuncSet = cpHashSetNew(0, (cpHashSetEqlFunc)collFuncSetEql, (cpHashSetTransFunc)collFuncSetTrans);
	space->collFuncSet->default_value = &space->defaultPairFunc;
	
	return space;
}

cpSpace*
cpSpaceNew(void)
{
	return cpSpaceInit(cpSpaceAlloc());
}

void
cpSpaceDestroy(cpSpace *space)
{
	cpSpaceHashFree(space->staticShapes);
	cpSpaceHashFree(space->activeShapes);
	
	cpArrayFree(space->bodies);
	
	cpArrayFree(space->constraints);
	
	if(space->contactSet)
		cpHashSetEach(space->contactSet, (cpHashSetIterFunc)&arbiterFreeWrap, NULL);
	
	cpHashSetFree(space->contactSet);
	cpArrayFree(space->arbiters);
	
	if(space->collFuncSet)
		cpHashSetEach(space->collFuncSet, &freeWrap, NULL);
	cpHashSetFree(space->collFuncSet);
}

void
cpSpaceFree(cpSpace *space)
{
	if(space){
		cpSpaceDestroy(space);
		free(space);
	}
}

void
cpSpaceFreeChildren(cpSpace *space)
{
	cpSpaceHashEach(space->staticShapes, (cpSpaceHashIterator)&shapeFreeWrap, NULL);
	cpSpaceHashEach(space->activeShapes, (cpSpaceHashIterator)&shapeFreeWrap, NULL);
	cpArrayEach(space->bodies,           (cpArrayIter)&bodyFreeWrap,          NULL);
	cpArrayEach(space->constraints,      (cpArrayIter)&constraintFreeWrap,    NULL);
}

#pragma mark Collision Pair Function Management

void
cpSpaceAddCollisionPairFunc(cpSpace *space, cpCollisionType a, cpCollisionType b, cpCollFunc func, void *data)
{
	cpCollisionType ids[] = {a, b};
	cpHashValue hash = CP_HASH_PAIR(a, b);
	// Remove any old function so the new one will get added.
	cpSpaceRemoveCollisionPairFunc(space, a, b);
		
	collFuncData funcData = {func, data};
	cpHashSetInsert(space->collFuncSet, hash, ids, &funcData);
}

void
cpSpaceRemoveCollisionPairFunc(cpSpace *space, cpCollisionType a, cpCollisionType b)
{
	cpCollisionType ids[] = {a, b};
	cpHashValue hash = CP_HASH_PAIR(a, b);
	cpCollPairFunc *old_pair = (cpCollPairFunc *)cpHashSetRemove(space->collFuncSet, hash, ids);
	free(old_pair);
}

void
cpSpaceSetDefaultCollisionPairFunc(cpSpace *space, cpCollFunc func, void *data)
{
	cpCollPairFunc pairFunc = {0, 0, (func ? func : alwaysCollide), (func ? data : NULL)};
	space->defaultPairFunc = pairFunc;
}

#pragma mark Body, Shape, and Joint Management

cpShape *
cpSpaceAddShape(cpSpace *space, cpShape *shape)
{
	assert(shape->body);
	cpSpaceHashInsert(space->activeShapes, shape, shape->id, shape->bb);
	
	return shape;
}

cpShape *
cpSpaceAddStaticShape(cpSpace *space, cpShape *shape)
{
	assert(shape->body);

	cpShapeCacheBB(shape);
	cpSpaceHashInsert(space->staticShapes, shape, shape->id, shape->bb);
	
	return shape;
}

cpBody *
cpSpaceAddBody(cpSpace *space, cpBody *body)
{
	cpArrayPush(space->bodies, body);
	
	return body;
}

cpConstraint *
cpSpaceAddConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpArrayPush(space->constraints, constraint);
	
	return constraint;
}

static void
shapeRemovalArbiterReject(cpSpace *space, cpShape *shape)
{
	cpArray *old_ary = space->arbiters;
	int num = old_ary->num;
	
	if(num == 0) return;
	
	// make a new arbiters array and copy over all valid arbiters
	cpArray *new_ary = cpArrayNew(num);
	
	for(int i=0; i<num; i++){
		cpArbiter *arb = (cpArbiter *)old_ary->arr[i];
		if(arb->a != shape && arb->b != shape){
			cpArrayPush(new_ary, arb);
		}
	}
	
	space->arbiters = new_ary;
	cpArrayFree(old_ary);
}

void
cpSpaceRemoveShape(cpSpace *space, cpShape *shape)
{
	cpSpaceHashRemove(space->activeShapes, shape, shape->id);
	shapeRemovalArbiterReject(space, shape);
}

void
cpSpaceRemoveStaticShape(cpSpace *space, cpShape *shape)
{
	cpSpaceHashRemove(space->staticShapes, shape, shape->id);
	shapeRemovalArbiterReject(space, shape);
}

void
cpSpaceRemoveBody(cpSpace *space, cpBody *body)
{
	cpArrayDeleteObj(space->bodies, body);
}

void
cpSpaceRemoveConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpArrayDeleteObj(space->constraints, constraint);
}

#pragma mark Point Query Functions

typedef struct pointQueryContext {
	cpLayers layers;
	cpGroup group;
	cpSpacePointQueryFunc func;
	void *data;
} pointQueryContext;

static void 
pointQueryHelper(cpVect *point, cpShape *shape, pointQueryContext *context)
{
	if(cpShapePointQuery(shape, *point, context->layers, context->group))
		context->func(shape, context->data);
}

void
cpSpacePointQuery(cpSpace *space, cpVect point, cpLayers layers, cpGroup group, cpSpacePointQueryFunc func, void *data)
{
	pointQueryContext context = {layers, group, func, data};
	cpSpaceHashPointQuery(space->activeShapes, point, (cpSpaceHashQueryFunc)pointQueryHelper, &context);
	cpSpaceHashPointQuery(space->staticShapes, point, (cpSpaceHashQueryFunc)pointQueryHelper, &context);
}

static void
rememberLastPointQuery(cpShape *shape, cpShape **outShape)
{
	(*outShape) = shape;
}

cpShape *
cpSpacePointQueryFirst(cpSpace *space, cpVect point, cpLayers layers, cpGroup group)
{
	cpShape *shape = NULL;
	cpSpacePointQuery(space, point, layers, group, (cpSpacePointQueryFunc)rememberLastPointQuery, &shape);
	
	return shape;
}

void
cpSpaceEachBody(cpSpace *space, cpSpaceBodyIterator func, void *data)
{
	cpArray *bodies = space->bodies;
	
	for(int i=0; i<bodies->num; i++)
		func((cpBody *)bodies->arr[i], data);
}

#pragma mark Segment Query Functions

typedef struct segQueryContext {
	cpVect start, end;
	cpLayers layers;
	cpGroup group;
	cpSpaceSegmentQueryFunc func;
	int anyCollision;
} segQueryContext;

static cpFloat
segQueryFunc(segQueryContext *context, cpShape *shape, void *data)
{
	cpSegmentQueryInfo info = {NULL, 0.0f, cpvzero};
	if(cpShapeSegmentQuery(shape, context->start, context->end, context->layers, context->group, &info)){
		if(context->func)
			context->func(shape, info.t, info.n, data);
		
		context->anyCollision = 1;
	}
	
	return 1.0f;
}

int
cpSpaceShapeSegmentQuery(cpSpace *space, cpVect start, cpVect end, cpLayers layers, cpGroup group, cpSpaceSegmentQueryFunc func, void *data)
{
	segQueryContext context = {
		start, end,
		layers, group,
		func,
		0,
	};
	
	cpSpaceHashSegmentQuery(space->staticShapes, &context, start, end, 1.0f, (cpSpaceHashSegmentQueryFunc)segQueryFunc, data);
	cpSpaceHashSegmentQuery(space->activeShapes, &context, start, end, 1.0f, (cpSpaceHashSegmentQueryFunc)segQueryFunc, data);
	
	return context.anyCollision;
}

typedef struct segQueryFirstContext {
	cpVect start, end;
	cpLayers layers;
	cpGroup group;
} segQueryFirstContext;

static cpFloat
segQueryFirst(segQueryFirstContext *context, cpShape *shape, cpSegmentQueryInfo *out)
{
	cpSegmentQueryInfo info = {NULL, 1.0f, cpvzero};
	if(cpShapeSegmentQuery(shape, context->start, context->end, context->layers, context->group, &info)){
		if(info.t < out->t){
			out->shape = info.shape;
			out->t = info.t;
			out->n = info.n;
		}
	}
	
	return info.t;
}

int
cpSpaceShapeSegmentQueryFirst(cpSpace *space, cpVect start, cpVect end, cpLayers layers, cpGroup group, cpSegmentQueryInfo *out)
{
	out->t = 1.0f;
	
	segQueryFirstContext context = {
		start, end,
		layers, group
	};
	
	cpSpaceHashSegmentQuery(space->staticShapes, &context, start, end, 1.0f, (cpSpaceHashSegmentQueryFunc)segQueryFirst, out);
	cpSpaceHashSegmentQuery(space->activeShapes, &context, start, end, out->t, (cpSpaceHashSegmentQueryFunc)segQueryFirst, out);
	
	return (out->shape != NULL);
}

#pragma mark Spatial Hash Management

// Iterator function used for updating shape BBoxes.
static void
updateBBCache(cpShape *shape, void *unused)
{
	cpShapeCacheBB(shape);
}

void
cpSpaceResizeStaticHash(cpSpace *space, cpFloat dim, int count)
{
	cpSpaceHashResize(space->staticShapes, dim, count);
	cpSpaceHashRehash(space->staticShapes);
}

void
cpSpaceResizeActiveHash(cpSpace *space, cpFloat dim, int count)
{
	cpSpaceHashResize(space->activeShapes, dim, count);
}

void 
cpSpaceRehashStatic(cpSpace *space)
{
	cpSpaceHashEach(space->staticShapes, (cpSpaceHashIterator)&updateBBCache, NULL);
	cpSpaceHashRehash(space->staticShapes);
}

#pragma mark Collision Detection Functions

static inline int
queryReject(cpShape *a, cpShape *b)
{
	return
		// BBoxes must overlap
		!cpBBintersects(a->bb, b->bb)
		// Don't collide shapes attached to the same body.
		|| a->body == b->body
		// Don't collide objects in the same non-zero group
		|| (a->group && b->group && a->group == b->group)
		// Don't collide objects that don't share at least on layer.
		|| !(a->layers & b->layers);
}

// Callback from the spatial hash.
static void
queryFunc(cpShape *a, cpShape *b, cpSpace *space)
{
	// Reject any of the simple cases
	if(queryReject(a,b)) return;
	
	// Shape 'a' should have the lower shape type. (required by cpCollideShapes() )
	if(a->klass->type > b->klass->type){
		cpShape *temp = a;
		a = b;
		b = temp;
	}
	
	// Narrow-phase collision detection.
	cpContact *contacts = NULL;
	int numContacts = cpCollideShapes(a, b, &contacts);
	if(!numContacts) return; // Shapes are not colliding.
	
	// Get an arbiter from space->contactSet for the two shapes.
	// This is where the persistant contact magic comes from.
	cpShape *shape_pair[] = {a, b};
	cpArbiter *arb = (cpArbiter *)cpHashSetInsert(space->contactSet, CP_HASH_PAIR(a, b), shape_pair, space);
	
	// Timestamp the arbiter.
	arb->stamp = space->stamp;
	// For collisions between two similar primitive types, the order could have been swapped.
	arb->a = a; arb->b = b;
	// Inject the new contact points into the arbiter.
	cpArbiterInject(arb, contacts, numContacts);
	
	// Add the arbiter to the list of active arbiters.
	cpArrayPush(space->arbiters, arb);
}

// Iterator for active/static hash collisions.
static void
active2staticIter(cpShape *shape, cpSpace *space)
{
	cpSpaceHashQuery(space->staticShapes, shape, shape->bb, (cpSpaceHashQueryFunc)&queryFunc, space);
}

// Hashset reject func to throw away old arbiters.
static int
contactSetReject(cpArbiter *arb, cpSpace *space)
{
	if((space->stamp - arb->stamp) > cp_contact_persistence){
		cpArbiterFree(arb);
		return 0;
	}
	
	return 1;
}

static void
filterArbiterByCallback(cpSpace *space)
{
	int num = space->arbiters->num;
	
	// copy to the stack
//	cpArbiter *ary[num];
	cpArbiter **ary = alloca(num*sizeof(cpArbiter *));
	
	memcpy(ary, space->arbiters->arr, num*sizeof(void *));
	
	for(int i=0; i<num; i++){
		cpArbiter *arb = ary[i];
		
		// The collision pair function requires objects to be ordered by their collision types.
		cpShape *a = arb->a;
		cpShape *b = arb->b;
		cpFloat normal_coef = 1.0f;
		
		// Find the collision pair function for the shapes.
		cpCollisionType ids[] = {a->collision_type, b->collision_type};
		cpHashValue hash = CP_HASH_PAIR(a->collision_type, b->collision_type);
		cpCollPairFunc *pairFunc = (cpCollPairFunc *)cpHashSetFind(space->collFuncSet, hash, ids);
		if(!pairFunc->func){
			cpArrayDeleteObj(space->arbiters, arb);
			continue; // A NULL pair function means don't collide at all.
		}
		
		// Swap them if necessary.
		if(a->collision_type != pairFunc->a){
			cpShape *temp = a;
			a = b;
			b = temp;
			normal_coef = -1.0f;
		}
		
		if(!pairFunc->func(a, b, arb->contacts, arb->numContacts, normal_coef, pairFunc->data)){
			cpArrayDeleteObj(space->arbiters, arb);
		}
	}
}

#pragma mark All Important cpSpaceStep() Function

void
cpSpaceStep(cpSpace *space, cpFloat dt)
{
	if(!dt) return; // prevents div by zero.
	cpFloat dt_inv = 1.0f/dt;

	cpArray *bodies = space->bodies;
	cpArray *constraints = space->constraints;
	
	// Empty the arbiter list.
	cpHashSetReject(space->contactSet, (cpHashSetRejectFunc)&contactSetReject, space);
	space->arbiters->num = 0;

	// Integrate positions.
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody *)bodies->arr[i];
		body->position_func(body, dt);
	}
	
	// Pre-cache BBoxes and shape data.
	cpSpaceHashEach(space->activeShapes, (cpSpaceHashIterator)&updateBBCache, NULL);
	
	// Collide!
	cpSpaceHashEach(space->activeShapes, (cpSpaceHashIterator)&active2staticIter, space);
	cpSpaceHashQueryRehash(space->activeShapes, (cpSpaceHashQueryFunc)&queryFunc, space);
	
	// Filter arbiter list based on collision callbacks
	filterArbiterByCallback(space);

	// Prestep the arbiters.
	cpArray *arbiters = space->arbiters;
	for(int i=0; i<arbiters->num; i++)
		cpArbiterPreStep((cpArbiter *)arbiters->arr[i], dt_inv);

	// Prestep the constraints.
	for(int i=0; i<constraints->num; i++){
		cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
		constraint->klass->preStep(constraint, dt, dt_inv);
	}

	for(int i=0; i<space->elasticIterations; i++){
		for(int j=0; j<arbiters->num; j++)
			cpArbiterApplyImpulse((cpArbiter *)arbiters->arr[j], 1.0f);
			
		for(int j=0; j<constraints->num; j++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[j];
			constraint->klass->applyImpulse(constraint);
		}
	}

	// Integrate velocities.
	cpFloat damping = cpfpow(1.0f/space->damping, -dt);
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody *)bodies->arr[i];
		body->velocity_func(body, space->gravity, damping, dt);
	}

	for(int i=0; i<arbiters->num; i++)
		cpArbiterApplyCachedImpulse((cpArbiter *)arbiters->arr[i]);
	
	// Run the impulse solver.
	// run the old-style elastic solver if elastic iterations are disabled
	cpFloat elasticCoef = (space->elasticIterations ? 0.0f : 1.0f);
	for(int i=0; i<space->iterations; i++){
		for(int j=0; j<arbiters->num; j++)
			cpArbiterApplyImpulse((cpArbiter *)arbiters->arr[j], elasticCoef);
			
		for(int j=0; j<constraints->num; j++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[j];
			constraint->klass->applyImpulse(constraint);
		}
	}

//	cpFloat dvsq = cpvdot(space->gravity, space->gravity);
//	dvsq *= dt*dt * space->damping*space->damping;
//	for(int i=0; i<bodies->num; i++)
//		cpBodyMarkLowEnergy(bodies->arr[i], dvsq, space->sleepTicks);
	
	// Increment the stamp.
	space->stamp++;
}
