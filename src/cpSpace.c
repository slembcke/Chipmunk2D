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

#include "chipmunk_private.h"

#pragma mark Contact Set Helpers

// Equal function for arbiterSet.
static cpBool
arbiterSetEql(cpShape **shapes, cpArbiter *arb)
{
	cpShape *a = shapes[0];
	cpShape *b = shapes[1];
	
	return ((a == arb->a && b == arb->b) || (b == arb->a && a == arb->b));
}

#pragma mark Collision Handler Set HelperFunctions

// Equals function for collisionHandlers.
static cpBool
handlerSetEql(cpCollisionHandler *check, cpCollisionHandler *pair)
{
	return ((check->a == pair->a && check->b == pair->b) || (check->b == pair->a && check->a == pair->b));
}

// Transformation function for collisionHandlers.
static void *
handlerSetTrans(cpCollisionHandler *handler, void *unused)
{
	cpCollisionHandler *copy = (cpCollisionHandler *)cpmalloc(sizeof(cpCollisionHandler));
	(*copy) = (*handler);
	
	return copy;
}

#pragma mark Misc Helper Funcs

// Default collision functions.
static cpBool alwaysCollide(cpArbiter *arb, cpSpace *space, void *data){return 1;}
static void nothing(cpArbiter *arb, cpSpace *space, void *data){}

// BBfunc callback for the spatial hash.
static cpVect shapeVelocityFunc(cpShape *shape){return shape->body->v;}

// Iterator functions for destructors.
static void             freeWrap(void         *ptr, void *unused){          cpfree(ptr);}
static void        shapeFreeWrap(cpShape      *ptr, void *unused){     cpShapeFree(ptr);}

void
cpSpaceLock(cpSpace *space)
{
	space->locked++;
}

void
cpSpaceUnlock(cpSpace *space)
{
	space->locked--;
	cpAssert(space->locked >= 0, "Internal error:Space lock underflow.");
	
	if(!space->locked){
		cpArray *waking = space->rousedBodies;
		for(int i=0, count=waking->num; i<count; i++){
			cpSpaceActivateBody(space, (cpBody *)waking->arr[i]);
		}
		
		waking->num = 0;
	}
}


#pragma mark Memory Management Functions

cpSpace *
cpSpaceAlloc(void)
{
	return (cpSpace *)cpcalloc(1, sizeof(cpSpace));
}

#define DEFAULT_DIM_SIZE 100.0f
#define DEFAULT_COUNT 1000
#define DEFAULT_ITERATIONS 10
#define DEFAULT_ELASTIC_ITERATIONS 0

cpCollisionHandler cpDefaultCollisionHandler = {0, 0, alwaysCollide, alwaysCollide, nothing, nothing, NULL};

cpSpace*
cpSpaceInit(cpSpace *space)
{
	space->iterations = DEFAULT_ITERATIONS;
	
	space->gravity = cpvzero;
	space->damping = 1.0f;
	
	space->collisionSlop = 0.1f;
	space->collisionBias = cpfpow(1.0f - 0.1f, 60.0f);
	space->collisionPersistence = 3;
	
	space->locked = 0;
	space->stamp = 0;

	space->staticShapes = cpBBTreeNew((cpSpatialIndexBBFunc)cpShapeGetBB, NULL);
	space->activeShapes = cpBBTreeNew((cpSpatialIndexBBFunc)cpShapeGetBB, space->staticShapes);
	cpBBTreeSetVelocityFunc(space->activeShapes, (cpBBTreeVelocityFunc)shapeVelocityFunc);
	
	space->allocatedBuffers = cpArrayNew(0);
	
	space->bodies = cpArrayNew(0);
	space->sleepingComponents = cpArrayNew(0);
	space->rousedBodies = cpArrayNew(0);
	
	space->sleepTimeThreshold = INFINITY;
	space->idleSpeedThreshold = 0.0f;
	
	space->arbiters = cpArrayNew(0);
	space->pooledArbiters = cpArrayNew(0);
	
	space->contactBuffersHead = NULL;
	space->cachedArbiters = cpHashSetNew(0, (cpHashSetEqlFunc)arbiterSetEql);
	
	space->constraints = cpArrayNew(0);
	
	space->defaultHandler = cpDefaultCollisionHandler;
	space->collisionHandlers = cpHashSetNew(0, (cpHashSetEqlFunc)handlerSetEql);
	cpHashSetSetDefaultValue(space->collisionHandlers, &cpDefaultCollisionHandler);
	
	space->postStepCallbacks = NULL;
	
	cpBodyInitStatic(&space->_staticBody);
	space->staticBody = &space->_staticBody;
	
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
	cpSpatialIndexFree(space->staticShapes);
	cpSpatialIndexFree(space->activeShapes);
	
	cpArrayFree(space->bodies);
	cpArrayFree(space->sleepingComponents);
	cpArrayFree(space->rousedBodies);
	
	cpArrayFree(space->constraints);
	
	cpHashSetFree(space->cachedArbiters);
	
	cpArrayFree(space->arbiters);
	cpArrayFree(space->pooledArbiters);
	
	if(space->allocatedBuffers){
		cpArrayFreeEach(space->allocatedBuffers, cpfree);
		cpArrayFree(space->allocatedBuffers);
	}
	
	if(space->postStepCallbacks){
		cpHashSetEach(space->postStepCallbacks, freeWrap, NULL);
		cpHashSetFree(space->postStepCallbacks);
	}
	
	if(space->collisionHandlers){
		cpHashSetEach(space->collisionHandlers, freeWrap, NULL);
		cpHashSetFree(space->collisionHandlers);
	}
}

void
cpSpaceFree(cpSpace *space)
{
	if(space){
		cpSpaceDestroy(space);
		cpfree(space);
	}
}

void
cpSpaceFreeChildren(cpSpace *space)
{
	cpArray *components = space->sleepingComponents;
	while(components->num) cpBodyActivate((cpBody *)components->arr[0]);
	
	cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)&shapeFreeWrap, NULL);
	cpSpatialIndexEach(space->activeShapes, (cpSpatialIndexIteratorFunc)&shapeFreeWrap, NULL);
	
	cpArrayFreeEach(space->bodies, (void (*)(void*))cpBodyFree);
	cpArrayFreeEach(space->constraints, (void (*)(void*))cpConstraintFree);
}

#pragma mark Collision Handler Function Management

void
cpSpaceAddCollisionHandler(
	cpSpace *space,
	cpCollisionType a, cpCollisionType b,
	cpCollisionBeginFunc begin,
	cpCollisionPreSolveFunc preSolve,
	cpCollisionPostSolveFunc postSolve,
	cpCollisionSeparateFunc separate,
	void *data
){
	// Remove any old function so the new one will get added.
	cpSpaceRemoveCollisionHandler(space, a, b);
	
	cpCollisionHandler handler = {
		a, b,
		begin ? begin : alwaysCollide,
		preSolve ? preSolve : alwaysCollide,
		postSolve ? postSolve : nothing,
		separate ? separate : nothing,
		data
	};
	
	cpHashSetInsert(space->collisionHandlers, CP_HASH_PAIR(a, b), &handler, NULL, (cpHashSetTransFunc)handlerSetTrans);
}

void
cpSpaceRemoveCollisionHandler(cpSpace *space, cpCollisionType a, cpCollisionType b)
{
	struct { cpCollisionType a, b; } ids = {a, b};
	cpCollisionHandler *old_handler = (cpCollisionHandler *) cpHashSetRemove(space->collisionHandlers, CP_HASH_PAIR(a, b), &ids);
	cpfree(old_handler);
}

void
cpSpaceSetDefaultCollisionHandler(
	cpSpace *space,
	cpCollisionBeginFunc begin,
	cpCollisionPreSolveFunc preSolve,
	cpCollisionPostSolveFunc postSolve,
	cpCollisionSeparateFunc separate,
	void *data
){
	cpCollisionHandler handler = {
		0, 0,
		begin ? begin : alwaysCollide,
		preSolve ? preSolve : alwaysCollide,
		postSolve ? postSolve : nothing,
		separate ? separate : nothing,
		data
	};
	
	space->defaultHandler = handler;
	cpHashSetSetDefaultValue(space->collisionHandlers, &space->defaultHandler);
}

#pragma mark Body, Shape, and Joint Management

#define cpAssertSpaceUnlocked(space) \
	cpAssert(!space->locked, \
		"This addition/removal cannot be done safely during a call to cpSpaceStep() or during a query. " \
		"Put these calls into a post-step callback." \
	);

static void
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

cpShape *
cpSpaceAddShape(cpSpace *space, cpShape *shape)
{
	cpBody *body = shape->body;
	if(cpBodyIsStatic(body)) return cpSpaceAddStaticShape(space, shape);
	
	cpAssert(!cpSpaceContainsShape(space, shape),
		"Cannot add the same shape more than once.");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(body);
	
	// Push onto the head of the body's shape list
	shape->next = body->shapeList; body->shapeList = shape;
	
	cpShapeUpdate(shape, body->p, body->rot);
	cpSpatialIndexInsert(space->activeShapes, shape, shape->hashid);
		
	return shape;
}

cpShape *
cpSpaceAddStaticShape(cpSpace *space, cpShape *shape)
{
	cpAssert(!cpSpaceContainsShape(space, shape),
		"Cannot add the same static shape more than once.");
	cpAssertSpaceUnlocked(space);
	
	cpBody *body = shape->body;
	cpShapeUpdate(shape, body->p, body->rot);
	cpSpaceActivateShapesTouchingShape(space, shape);
	cpSpatialIndexInsert(space->staticShapes, shape, shape->hashid);
	
	return shape;
}

cpBody *
cpSpaceAddBody(cpSpace *space, cpBody *body)
{
	cpAssertWarn(!cpBodyIsStatic(body), "Static bodies cannot be added to a space as they are not meant to be simulated.");
	cpAssert(!cpSpaceContainsBody(space, body),
		"Cannot add a body to a more than one space or to the same space twice.");
	cpAssertSpaceUnlocked(space);
	
	cpArrayPush(space->bodies, body);
	body->space = space;
	
	return body;
}

static void
cpBodyRemoveConstraint(cpBody *body, cpConstraint *constraint)
{
	cpConstraint **prev_ptr = &body->constraintList;
	cpConstraint *node = body->constraintList;
	
	while(node && node != constraint){
		prev_ptr = (node->a == body ? &node->nextA : &node->nextB);
		node = *prev_ptr;
	}
	
	cpAssert(node, "Attempted to remove a constraint from a body it was never attached to.");
	(*prev_ptr) = (node->a == body ? node->nextA : node->nextB);
}



cpConstraint *
cpSpaceAddConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpAssert(!cpSpaceContainsConstraint(space, constraint),
		"Cannot add the same constraint more than once.");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(constraint->a);
	cpBodyActivate(constraint->b);
	cpArrayPush(space->constraints, constraint);
	
	// Push onto the heads of the bodies' constraint lists
	cpBody *a = constraint->a, *b = constraint->b;
	constraint->nextA = a->constraintList; a->constraintList = constraint;
	constraint->nextB = b->constraintList; b->constraintList = constraint;
	
	return constraint;
}

typedef struct removalContext {
	cpSpace *space;
	cpShape *shape;
} removalContext;

// Hashset filter func to throw away old arbiters.
static cpBool
arbiterSetFilterRemovedShape(cpArbiter *arb, removalContext *context)
{
	if(context->shape == arb->a || context->shape == arb->b){
		if(arb->state != cpArbiterStateCached){
			arb->handler->separate(arb, context->space, arb->handler->data);
		}
		
		cpArrayPush(context->space->pooledArbiters, arb);
		return cpFalse;
	}
	
	return cpTrue;
}

void
cpSpaceRemoveShape(cpSpace *space, cpShape *shape)
{
	cpBody *body = shape->body;
	if(cpBodyIsStatic(body)){
		cpSpaceRemoveStaticShape(space, shape);
		return;
	}

	cpBodyActivate(body);
	
	cpAssert(cpSpaceContainsShape(space, shape),
		"Cannot remove a shape that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);
	
	cpBodyRemoveShape(body, shape);
	
	removalContext context = {space, shape};
	cpHashSetFilter(space->cachedArbiters, (cpHashSetFilterFunc)arbiterSetFilterRemovedShape, &context);
	cpSpatialIndexRemove(space->activeShapes, shape, shape->hashid);
}

void
cpSpaceRemoveStaticShape(cpSpace *space, cpShape *shape)
{
	cpAssert(cpSpaceContainsShape(space, shape),
		"Cannot remove a static or sleeping shape that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);
	
	removalContext context = {space, shape};
	cpHashSetFilter(space->cachedArbiters, (cpHashSetFilterFunc)arbiterSetFilterRemovedShape, &context);
	cpSpatialIndexRemove(space->staticShapes, shape, shape->hashid);
	
	cpSpaceActivateShapesTouchingShape(space, shape);
}

void
cpSpaceRemoveBody(cpSpace *space, cpBody *body)
{
	cpAssertWarn(cpSpaceContainsBody(space, body),
		"Cannot remove a body that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(body);
	cpArrayDeleteObj(space->bodies, body);
	body->space = NULL;
}

void
cpSpaceRemoveConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpAssertWarn(cpSpaceContainsConstraint(space, constraint),
		"Cannot remove a constraint that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(constraint->a);
	cpBodyActivate(constraint->b);
	cpArrayDeleteObj(space->constraints, constraint);
	
	cpBodyRemoveConstraint(constraint->a, constraint);
	cpBodyRemoveConstraint(constraint->b, constraint);
}

cpBool cpSpaceContainsShape(cpSpace *space, cpShape *shape)
{
	return
		cpSpatialIndexContains(space->activeShapes, shape, shape->hashid) ||
		cpSpatialIndexContains(space->staticShapes, shape, shape->hashid);
}

cpBool cpSpaceContainsBody(cpSpace *space, cpBody *body)
{
	return (body->space == space);
}

cpBool cpSpaceContainsConstraint(cpSpace *space, cpConstraint *constraint)
{
	return cpArrayContains(space->constraints, constraint);
}


#pragma mark Iteration

void
cpSpaceEachBody(cpSpace *space, cpSpaceBodyIteratorFunc func, void *data)
{
	cpSpaceLock(space); {
		cpArray *bodies = space->bodies;
		
		for(int i=0; i<bodies->num; i++){
			func((cpBody *)bodies->arr[i], data);
		}
		
		cpArray *components = space->sleepingComponents;
		for(int i=0; i<components->num; i++){
			cpBody *root = (cpBody *)components->arr[i];
			CP_BODY_FOREACH_COMPONENT(root, body) func(body, data);
		}
	} cpSpaceUnlock(space);
}

#pragma mark Spatial Index Management

static void
updateBBCache(cpShape *shape, void *unused)
{
	cpBody *body = shape->body;
	cpShapeUpdate(shape, body->p, body->rot);
}

void 
cpSpaceReindexStatic(cpSpace *space)
{
	cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)&updateBBCache, NULL);
	cpSpatialIndexReindex(space->staticShapes);
}

void
cpSpaceReindexShape(cpSpace *space, cpShape *shape)
{
	cpBody *body = shape->body;
	cpShapeUpdate(shape, body->p, body->rot);
	
	// attempt to rehash the shape in both hashes
	cpSpatialIndexReindexObject(space->activeShapes, shape, shape->hashid);
	cpSpatialIndexReindexObject(space->staticShapes, shape, shape->hashid);
}

static void
copyShapes(cpShape *shape, cpSpatialIndex *index)
{
	cpSpatialIndexInsert(index, shape, shape->hashid);
}

void
cpSpaceUseSpatialHash(cpSpace *space, cpFloat dim, int count)
{
	cpSpatialIndex *staticShapes = cpSpaceHashNew(dim, count, (cpSpatialIndexBBFunc)cpShapeGetBB, NULL);
	cpSpatialIndex *activeShapes = cpSpaceHashNew(dim, count, (cpSpatialIndexBBFunc)cpShapeGetBB, staticShapes);
	
	cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)copyShapes, staticShapes);
	cpSpatialIndexEach(space->activeShapes, (cpSpatialIndexIteratorFunc)copyShapes, activeShapes);
	
	cpSpatialIndexFree(space->staticShapes);
	cpSpatialIndexFree(space->activeShapes);
	
	space->staticShapes = staticShapes;
	space->activeShapes = activeShapes;
}
