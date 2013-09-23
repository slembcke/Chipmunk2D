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
 
#include <stdio.h>
#include <string.h>

#include "chipmunk/chipmunk_private.h"

//MARK: Contact Set Helpers

// Equal function for arbiterSet.
static cpBool
arbiterSetEql(cpShape **shapes, cpArbiter *arb)
{
	cpShape *a = shapes[0];
	cpShape *b = shapes[1];
	
	return ((a == arb->a && b == arb->b) || (b == arb->a && a == arb->b));
}

//MARK: Collision Handler Set HelperFunctions

// Equals function for collisionHandlers.
static cpBool
handlerSetEql(cpCollisionHandler *check, cpCollisionHandler *pair)
{
	return (
		(check->typeA == pair->typeA && check->typeB == pair->typeB) ||
		(check->typeB == pair->typeA && check->typeA == pair->typeB)
	);
}

// Transformation function for collisionHandlers.
static void *
handlerSetTrans(cpCollisionHandler *handler, void *unused)
{
	cpCollisionHandler *copy = (cpCollisionHandler *)cpcalloc(1, sizeof(cpCollisionHandler));
	(*copy) = (*handler);
	
	return copy;
}

//MARK: Misc Helper Funcs

// Default collision functions.

static cpBool
DefaultBegin(cpArbiter *arb, cpSpace *space, void *data){
	return cpArbiterCallWildcardBeginA(arb, space) && cpArbiterCallWildcardBeginB(arb, space);
}

static cpBool
DefaultPreSolve(cpArbiter *arb, cpSpace *space, void *data){
	return cpArbiterCallWildcardPreSolveA(arb, space) && cpArbiterCallWildcardPreSolveB(arb, space);
}

static void
DefaultPostSolve(cpArbiter *arb, cpSpace *space, void *data){
	cpArbiterCallWildcardPostSolveA(arb, space);
	cpArbiterCallWildcardPostSolveB(arb, space);
}

static void
DefaultSeparate(cpArbiter *arb, cpSpace *space, void *data){
	cpArbiterCallWildcardSeparateA(arb, space);
	cpArbiterCallWildcardSeparateB(arb, space);
}

// Use the wildcard identifier since  the default handler should never match any type pair.
static cpCollisionHandler cpCollisionHandlerDefault = {
	CP_WILDCARD_COLLISION_TYPE, CP_WILDCARD_COLLISION_TYPE,
	DefaultBegin, DefaultPreSolve, DefaultPostSolve, DefaultSeparate, NULL
};

static cpBool AlwaysCollide(cpArbiter *arb, cpSpace *space, void *data){return cpTrue;}
static void DoNothing(cpArbiter *arb, cpSpace *space, void *data){}

cpCollisionHandler cpCollisionHandlerDoNothing = {
	CP_WILDCARD_COLLISION_TYPE, CP_WILDCARD_COLLISION_TYPE,
	AlwaysCollide, AlwaysCollide, DoNothing, DoNothing, NULL
};

// function to get the estimated velocity of a shape for the cpBBTree.
static cpVect ShapeVelocityFunc(cpShape *shape){return shape->body->v;}

// Used for disposing of collision handlers.
static void FreeWrap(void *ptr, void *unused){cpfree(ptr);}

//MARK: Memory Management Functions

cpSpace *
cpSpaceAlloc(void)
{
	return (cpSpace *)cpcalloc(1, sizeof(cpSpace));
}

cpSpace*
cpSpaceInit(cpSpace *space)
{
#ifndef NDEBUG
	static cpBool done = cpFalse;
	if(!done){
		printf("Initializing cpSpace - Chipmunk v%s (Debug Enabled)\n", cpVersionString);
		printf("Compile with -DNDEBUG defined to disable debug mode and runtime assertion checks\n");
		done = cpTrue;
	}
#endif

	space->iterations = 10;
	
	space->gravity = cpvzero;
	space->damping = 1.0f;
	
	space->collisionSlop = 0.1f;
	space->collisionBias = cpfpow(1.0f - 0.1f, 60.0f);
	space->collisionPersistence = 3;
	
	space->locked = 0;
	space->stamp = 0;
	
	space->shapeIDCounter = 0;
	space->staticShapes = cpBBTreeNew((cpSpatialIndexBBFunc)cpShapeGetBB, NULL);
	space->activeShapes = cpBBTreeNew((cpSpatialIndexBBFunc)cpShapeGetBB, space->staticShapes);
	cpBBTreeSetVelocityFunc(space->activeShapes, (cpBBTreeVelocityFunc)ShapeVelocityFunc);
	
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
	
	space->usesWildcards = cpFalse;
	space->defaultHandler = cpCollisionHandlerDoNothing;
	space->collisionHandlers = cpHashSetNew(0, (cpHashSetEqlFunc)handlerSetEql);
	
	space->postStepCallbacks = cpArrayNew(0);
	space->skipPostStep = cpFalse;
	
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
	cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)cpBodyActivate, NULL);
	
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
		cpArrayFreeEach(space->postStepCallbacks, cpfree);
		cpArrayFree(space->postStepCallbacks);
	}
	
	if(space->collisionHandlers) cpHashSetEach(space->collisionHandlers, FreeWrap, NULL);
	cpHashSetFree(space->collisionHandlers);
}

void
cpSpaceFree(cpSpace *space)
{
	if(space){
		cpSpaceDestroy(space);
		cpfree(space);
	}
}

#define cpAssertSpaceUnlocked(space) \
	cpAssertHard(!space->locked, \
		"This operation cannot be done safely during a call to cpSpaceStep() or during a query. " \
		"Put these calls into a post-step callback." \
	);

//MARK: Collision Handler Function Management

static void
cpSpaceUseWildcardDefaultHandler(cpSpace *space)
{
	// Spaces default to using the slightly faster "do nothing" default handler until wildcards are potentially needed.
	if(!space->usesWildcards){
		space->usesWildcards = cpTrue;
		space->defaultHandler = cpCollisionHandlerDefault;
	}
}

cpCollisionHandler *cpSpaceAddDefaultCollisionHandler(cpSpace *space)
{
	cpSpaceUseWildcardDefaultHandler(space);
	return &space->defaultHandler;
}

cpCollisionHandler *cpSpaceAddCollisionHandler(cpSpace *space, cpCollisionType a, cpCollisionType b)
{
	cpHashValue hash = CP_HASH_PAIR(a, b);
	// TODO should use space->defaultHandler values instead?
	cpCollisionHandler temp = {a, b, DefaultBegin, DefaultPreSolve, DefaultPostSolve, DefaultSeparate, NULL};
	
	cpHashSet *handlers = space->collisionHandlers;
	cpCollisionHandler *handler = cpHashSetFind(handlers, hash, &temp);
	return (handler ? handler : cpHashSetInsert(handlers, hash, &temp, (cpHashSetTransFunc)handlerSetTrans, NULL));
}

cpCollisionHandler *
cpSpaceAddWildcardHandler(cpSpace *space, cpCollisionType type)
{
	cpSpaceUseWildcardDefaultHandler(space);
	
	cpHashValue hash = CP_HASH_PAIR(type, CP_WILDCARD_COLLISION_TYPE);
	cpCollisionHandler temp = {type, CP_WILDCARD_COLLISION_TYPE, AlwaysCollide, AlwaysCollide, DoNothing, DoNothing, NULL};
	
	cpHashSet *handlers = space->collisionHandlers;
	cpCollisionHandler *handler = cpHashSetFind(handlers, hash, &temp);
	return (handler ? handler : cpHashSetInsert(handlers, hash, &temp, (cpHashSetTransFunc)handlerSetTrans, NULL));
}


//MARK: Body, Shape, and Joint Management
static cpShape *
cpSpaceAddStaticShape(cpSpace *space, cpShape *shape)
{
	cpAssertHard(shape->space != space, "You have already added this shape to this space. You must not add it a second time.");
	cpAssertHard(!shape->space, "You have already added this shape to another space. You cannot add it to a second.");
	cpAssertHard(cpBodyIsRogue(shape->body), "You are adding a static shape to a dynamic body. Did you mean to attach it to a static or rogue body? See the documentation for more information.");
	cpAssertSpaceUnlocked(space);
	
	cpBody *body = shape->body;
	cpBodyAddShape(body, shape);
	
	shape->hashid = space->shapeIDCounter++;
	cpShapeUpdate(shape, body->transform);
	cpSpatialIndexInsert(space->staticShapes, shape, shape->hashid);
	shape->space = space;
	
	return shape;
}

cpShape *
cpSpaceAddShape(cpSpace *space, cpShape *shape)
{
	cpBody *body = shape->body;
	if(cpBodyIsStatic(body)) return cpSpaceAddStaticShape(space, shape);
	
	cpAssertHard(shape->space != space, "You have already added this shape to this space. You must not add it a second time.");
	cpAssertHard(!shape->space, "You have already added this shape to another space. You cannot add it to a second.");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(body);
	cpBodyAddShape(body, shape);
	
	shape->hashid = space->shapeIDCounter++;
	cpShapeUpdate(shape, body->transform);
	cpSpatialIndexInsert(space->activeShapes, shape, shape->hashid);
	shape->space = space;
		
	return shape;
}

cpBody *
cpSpaceAddBody(cpSpace *space, cpBody *body)
{
	cpAssertHard(!cpBodyIsStatic(body), "Do not add static bodies to a space. Static bodies do not move and should not be simulated.");
	cpAssertHard(body->space != space, "You have already added this body to this space. You must not add it a second time.");
	cpAssertHard(!body->space, "You have already added this body to another space. You cannot add it to a second.");
	cpAssertSpaceUnlocked(space);
	
	cpArrayPush(space->bodies, body);
	body->space = space;
	
	return body;
}

cpConstraint *
cpSpaceAddConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpAssertHard(constraint->space != space, "You have already added this constraint to this space. You must not add it a second time.");
	cpAssertHard(!constraint->space, "You have already added this constraint to another space. You cannot add it to a second.");
	cpAssertHard(constraint->a && constraint->b, "Constraint is attached to a NULL body.");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(constraint->a);
	cpBodyActivate(constraint->b);
	cpArrayPush(space->constraints, constraint);
	
	// Push onto the heads of the bodies' constraint lists
	cpBody *a = constraint->a, *b = constraint->b;
	constraint->next_a = a->constraintList; a->constraintList = constraint;
	constraint->next_b = b->constraintList; b->constraintList = constraint;
	constraint->space = space;
	
	return constraint;
}

struct arbiterFilterContext {
	cpSpace *space;
	cpBody *body;
	cpShape *shape;
};

static cpBool
cachedArbitersFilter(cpArbiter *arb, struct arbiterFilterContext *context)
{
	cpShape *shape = context->shape;
	cpBody *body = context->body;
	
	
	// Match on the filter shape, or if it's NULL the filter body
	if(
		(body == arb->body_a && (shape == arb->a || shape == NULL)) ||
		(body == arb->body_b && (shape == arb->b || shape == NULL))
	){
		// Call separate when removing shapes.
		if(shape && arb->state != CP_ARBITER_STATE_CACHED){
			cpCollisionHandler *handler = arb->handler;
			handler->separateFunc(arb, context->space, handler->userData);
		}
		
		cpArbiterUnthread(arb);
		cpArrayDeleteObj(context->space->arbiters, arb);
		cpArrayPush(context->space->pooledArbiters, arb);
		
		return cpFalse;
	}
	
	return cpTrue;
}

void
cpSpaceFilterArbiters(cpSpace *space, cpBody *body, cpShape *filter)
{
	cpSpaceLock(space); {
		struct arbiterFilterContext context = {space, body, filter};
		cpHashSetFilter(space->cachedArbiters, (cpHashSetFilterFunc)cachedArbitersFilter, &context);
	} cpSpaceUnlock(space, cpTrue);
}

static void
cpSpaceRemoveStaticShape(cpSpace *space, cpShape *shape)
{
	cpAssertHard(cpSpaceContainsShape(space, shape), "Cannot remove a static or sleeping shape that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);
	
	cpBody *body = shape->body;
	if(cpBodyIsStatic(body)) cpBodyActivateStatic(body, shape);
	cpBodyRemoveShape(body, shape);
	cpSpaceFilterArbiters(space, body, shape);
	cpSpatialIndexRemove(space->staticShapes, shape, shape->hashid);
	shape->space = NULL;
	shape->hashid = 0;
}

void
cpSpaceRemoveShape(cpSpace *space, cpShape *shape)
{
	cpBody *body = shape->body;
	if(cpBodyIsStatic(body)){
		cpSpaceRemoveStaticShape(space, shape);
	} else {
		cpAssertHard(cpSpaceContainsShape(space, shape), "Cannot remove a shape that was not added to the space. (Removed twice maybe?)");
		cpAssertSpaceUnlocked(space);
		
		cpBodyActivate(body);
		cpBodyRemoveShape(body, shape);
		cpSpaceFilterArbiters(space, body, shape);
		cpSpatialIndexRemove(space->activeShapes, shape, shape->hashid);
		shape->space = NULL;
		shape->hashid = 0;
	}
}

void
cpSpaceRemoveBody(cpSpace *space, cpBody *body)
{
	cpAssertHard(cpSpaceContainsBody(space, body), "Cannot remove a body that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(body);
//	cpSpaceFilterArbiters(space, body, NULL);
	cpArrayDeleteObj(space->bodies, body);
	body->space = NULL;
}

void
cpSpaceRemoveConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpAssertHard(cpSpaceContainsConstraint(space, constraint), "Cannot remove a constraint that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivate(constraint->a);
	cpBodyActivate(constraint->b);
	cpArrayDeleteObj(space->constraints, constraint);
	
	cpBodyRemoveConstraint(constraint->a, constraint);
	cpBodyRemoveConstraint(constraint->b, constraint);
	constraint->space = NULL;
}

cpBool cpSpaceContainsShape(cpSpace *space, cpShape *shape)
{
	return (shape->space == space);
}

cpBool cpSpaceContainsBody(cpSpace *space, cpBody *body)
{
	return (body->space == space);
}

cpBool cpSpaceContainsConstraint(cpSpace *space, cpConstraint *constraint)
{
	return (constraint->space == space);
}

//MARK: Static/rogue body conversion.

void
cpSpaceConvertBodyToStatic(cpSpace *space, cpBody *body)
{
	cpAssertHard(!cpBodyIsStatic(body), "Body is already static.");
	cpAssertHard(cpBodyIsRogue(body), "Remove the body from the space before calling this function.");
	cpAssertSpaceUnlocked(space);
	
	cpBodySetMass(body, INFINITY);
	cpBodySetMoment(body, INFINITY);
	
	cpBodySetVelocity(body, cpvzero);
	cpBodySetAngularVelocity(body, 0.0f);
	
	body->node.idleTime = INFINITY;
	CP_BODY_FOREACH_SHAPE(body, shape){
		cpSpatialIndexRemove(space->activeShapes, shape, shape->hashid);
		cpSpatialIndexInsert(space->staticShapes, shape, shape->hashid);
	}
}

void
cpSpaceConvertBodyToDynamic(cpSpace *space, cpBody *body, cpFloat m, cpFloat i)
{
	cpAssertHard(cpBodyIsStatic(body), "Body is already dynamic.");
	cpAssertSpaceUnlocked(space);
	
	cpBodyActivateStatic(body, NULL);
	
	cpBodySetMass(body, m);
	cpBodySetMoment(body, i);
	
	body->node.idleTime = 0.0f;
	CP_BODY_FOREACH_SHAPE(body, shape){
		cpSpatialIndexRemove(space->staticShapes, shape, shape->hashid);
		cpSpatialIndexInsert(space->activeShapes, shape, shape->hashid);
	}
}

//MARK: Iteration

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
			
			cpBody *body = root;
			while(body){
				cpBody *next = body->node.next;
				func(body, data);
				body = next;
			}
		}
	} cpSpaceUnlock(space, cpTrue);
}

typedef struct spaceShapeContext {
	cpSpaceShapeIteratorFunc func;
	void *data;
} spaceShapeContext;

static void
spaceEachShapeIterator(cpShape *shape, spaceShapeContext *context)
{
	context->func(shape, context->data);
}

void
cpSpaceEachShape(cpSpace *space, cpSpaceShapeIteratorFunc func, void *data)
{
	cpSpaceLock(space); {
		spaceShapeContext context = {func, data};
		cpSpatialIndexEach(space->activeShapes, (cpSpatialIndexIteratorFunc)spaceEachShapeIterator, &context);
		cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)spaceEachShapeIterator, &context);
	} cpSpaceUnlock(space, cpTrue);
}

void
cpSpaceEachConstraint(cpSpace *space, cpSpaceConstraintIteratorFunc func, void *data)
{
	cpSpaceLock(space); {
		cpArray *constraints = space->constraints;
		
		for(int i=0; i<constraints->num; i++){
			func((cpConstraint *)constraints->arr[i], data);
		}
	} cpSpaceUnlock(space, cpTrue);
}

//MARK: Spatial Index Management

void 
cpSpaceReindexStatic(cpSpace *space)
{
	cpAssertHard(!space->locked, "You cannot manually reindex objects while the space is locked. Wait until the current query or step is complete.");
	
	cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)&cpShapeUpdateFunc, NULL);
	cpSpatialIndexReindex(space->staticShapes);
}

void
cpSpaceReindexShape(cpSpace *space, cpShape *shape)
{
	cpAssertHard(!space->locked, "You cannot manually reindex objects while the space is locked. Wait until the current query or step is complete.");
	
	cpShapeCacheBB(shape);
	
	// attempt to rehash the shape in both hashes
	cpSpatialIndexReindexObject(space->activeShapes, shape, shape->hashid);
	cpSpatialIndexReindexObject(space->staticShapes, shape, shape->hashid);
}

void
cpSpaceReindexShapesForBody(cpSpace *space, cpBody *body)
{
	CP_BODY_FOREACH_SHAPE(body, shape) cpSpaceReindexShape(space, shape);
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
