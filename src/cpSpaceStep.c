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

#include "chipmunk_private.h"

#pragma mark Post Step Callback Functions

typedef struct postStepCallback {
	cpPostStepFunc func;
	void *obj;
	void *data;
} postStepCallback;

static cpBool
postStepFuncSetEql(postStepCallback *a, postStepCallback *b){
	return a->obj == b->obj;
}

static void *
postStepFuncSetTrans(postStepCallback *callback, void *ignored)
{
	postStepCallback *value = (postStepCallback *)cpmalloc(sizeof(postStepCallback));
	(*value) = (*callback);
	
	return value;
}

void
cpSpaceAddPostStepCallback(cpSpace *space, cpPostStepFunc func, void *obj, void *data)
{
	if(!space->postStepCallbacks){
		space->postStepCallbacks = cpHashSetNew(0, (cpHashSetEqlFunc)postStepFuncSetEql, NULL);
	}
	
	postStepCallback callback = {func, obj, data};
	cpHashSetInsert(space->postStepCallbacks, (cpHashValue)(size_t)obj, &callback, NULL, (cpHashSetTransFunc)postStepFuncSetTrans);
}

#pragma mark Contact Buffer Functions

struct cpContactBufferHeader {
	cpTimestamp stamp;
	cpContactBufferHeader *next;
	unsigned int numContacts;
};

#define CP_CONTACTS_BUFFER_SIZE ((CP_BUFFER_BYTES - sizeof(cpContactBufferHeader))/sizeof(cpContact))
typedef struct cpContactBuffer {
	cpContactBufferHeader header;
	cpContact contacts[CP_CONTACTS_BUFFER_SIZE];
} cpContactBuffer;

static cpContactBufferHeader *
cpSpaceAllocContactBuffer(cpSpace *space)
{
	cpContactBuffer *buffer = (cpContactBuffer *)cpmalloc(sizeof(cpContactBuffer));
	cpArrayPush(space->allocatedBuffers, buffer);
	return (cpContactBufferHeader *)buffer;
}

static cpContactBufferHeader *
cpContactBufferHeaderInit(cpContactBufferHeader *header, cpTimestamp stamp, cpContactBufferHeader *splice)
{
	header->stamp = stamp;
	header->next = (splice ? splice->next : header);
	header->numContacts = 0;
	
	return header;
}

static void
cpSpacePushFreshContactBuffer(cpSpace *space)
{
	cpTimestamp stamp = space->stamp;
	
	cpContactBufferHeader *head = space->contactBuffersHead;
	
	if(!head){
		// No buffers have been allocated, make one
		space->contactBuffersHead = cpContactBufferHeaderInit(cpSpaceAllocContactBuffer(space), stamp, NULL);
	} else if(stamp - head->next->stamp > cp_contact_persistence){
		// The tail buffer is available, rotate the ring
	cpContactBufferHeader *tail = head->next;
		space->contactBuffersHead = cpContactBufferHeaderInit(tail, stamp, tail);
	} else {
		// Allocate a new buffer and push it into the ring
		cpContactBufferHeader *buffer = cpContactBufferHeaderInit(cpSpaceAllocContactBuffer(space), stamp, head);
		space->contactBuffersHead = head->next = buffer;
	}
}


cpContact *
cpContactBufferGetArray(cpSpace *space)
{
	if(space->contactBuffersHead->numContacts + CP_MAX_CONTACTS_PER_ARBITER > CP_CONTACTS_BUFFER_SIZE){
		// contact buffer could overflow on the next collision, push a fresh one.
		cpSpacePushFreshContactBuffer(space);
	}
	
	cpContactBufferHeader *head = space->contactBuffersHead;
	return ((cpContactBuffer *)head)->contacts + head->numContacts;
}

void
cpSpacePushContacts(cpSpace *space, int count)
{
	cpAssert(count <= CP_MAX_CONTACTS_PER_ARBITER, "Internal Error:contact buffer overflow!");
	space->contactBuffersHead->numContacts += count;
}

static inline void
cpSpacePopContacts(cpSpace *space, int count){
	space->contactBuffersHead->numContacts -= count;
}

#pragma mark Collision Detection Functions

static void *
contactSetTrans(cpShape **shapes, cpSpace *space)
{
	if(space->pooledArbiters->num == 0){
		// arbiter pool is exhausted, make more
		int count = CP_BUFFER_BYTES/sizeof(cpArbiter);
		cpAssert(count, "Buffer size too small.");
		
		cpArbiter *buffer = (cpArbiter *)cpmalloc(CP_BUFFER_BYTES);
		cpArrayPush(space->allocatedBuffers, buffer);
		
		for(int i=0; i<count; i++) cpArrayPush(space->pooledArbiters, buffer + i);
	}
	
	return cpArbiterInit((cpArbiter *)cpArrayPop(space->pooledArbiters), shapes[0], shapes[1]);
}

static inline cpBool
queryReject(cpShape *a, cpShape *b)
{
	return (
		// BBoxes must overlap
		!cpBBintersects(a->bb, b->bb)
		// Don't collide shapes attached to the same body.
		|| a->body == b->body
		// Don't collide objects in the same non-zero group
		|| (a->group && a->group == b->group)
		// Don't collide objects that don't share at least on layer.
		|| !(a->layers & b->layers)
	);
}

// Callback from the spatial hash.
void
cpSpaceCollideShapes(cpShape *a, cpShape *b, cpSpace *space)
{
	// Reject any of the simple cases
	if(queryReject(a,b)) return;
	
	// Find the collision pair function for the shapes.
	cpCollisionType types[] = {a->collision_type, b->collision_type};
	cpHashValue collHashID = CP_HASH_PAIR(a->collision_type, b->collision_type);
	cpCollisionHandler *handler = (cpCollisionHandler *)cpHashSetFind(space->collFuncSet, collHashID, types);
	
	cpBool sensor = a->sensor || b->sensor;
	if(sensor && handler == &space->defaultHandler) return;
	
	// Shape 'a' should have the lower shape type. (required by cpCollideShapes() )
	if(a->klass->type > b->klass->type){
		cpShape *temp = a;
		a = b;
		b = temp;
	}
	
	// Narrow-phase collision detection.
	cpContact *contacts = cpContactBufferGetArray(space);
	int numContacts = cpCollideShapes(a, b, contacts);
	if(!numContacts) return; // Shapes are not colliding.
	cpSpacePushContacts(space, numContacts);
	
	// Get an arbiter from space->contactSet for the two shapes.
	// This is where the persistant contact magic comes from.
	cpShape *shape_pair[] = {a, b};
	cpHashValue arbHashID = CP_HASH_PAIR((size_t)a, (size_t)b);
	cpArbiter *arb = (cpArbiter *)cpHashSetInsert(space->contactSet, arbHashID, shape_pair, space, (cpHashSetTransFunc)contactSetTrans);
	cpArbiterUpdate(arb, contacts, numContacts, handler, a, b);
	
	// Call the begin function first if it's the first step
	if(arb->state == cpArbiterStateFirstColl && !handler->begin(arb, space, handler->data)){
		cpArbiterIgnore(arb); // permanently ignore the collision until separation
	}
	
	if(
		// Ignore the arbiter if it has been flagged
		(arb->state != cpArbiterStateIgnore) && 
		// Call preSolve
		handler->preSolve(arb, space, handler->data) &&
		// Process, but don't add collisions for sensors.
		!sensor
	){
		cpArrayPush(space->arbiters, arb);
	} else {
		cpSpacePopContacts(space, numContacts);
		
		arb->contacts = NULL;
		arb->numContacts = 0;
		
		// Normally arbiters are set as used after calling the post-step callback.
		// However, post-step callbacks are not called for sensors or arbiters rejected from pre-solve.
		if(arb->state != cpArbiterStateIgnore) arb->state = cpArbiterStateNormal;
	}
	
	// Time stamp the arbiter so we know it was used recently.
	arb->stamp = space->stamp;
}

// Hashset filter func to throw away old arbiters.
static cpBool
contactSetFilter(cpArbiter *arb, cpSpace *space)
{
	cpTimestamp ticks = space->stamp - arb->stamp;
	
	// was used last frame, but not this one
	if(ticks >= 1 && arb->state != cpArbiterStateCached){
		arb->handler->separate(arb, space, arb->handler->data);
		arb->state = cpArbiterStateCached;
	}
	
	if(ticks >= cp_contact_persistence){
		arb->contacts = NULL;
		arb->numContacts = 0;
		
		cpArrayPush(space->pooledArbiters, arb);
		return cpFalse;
	}
	
	return cpTrue;
}

// Hashset filter func to call and throw away post step callbacks.
static void
postStepCallbackSetIter(postStepCallback *callback, cpSpace *space)
{
	callback->func(space, callback->obj, callback->data);
	cpfree(callback);
}

#pragma mark All Important cpSpaceStep() Function

void cpSpaceProcessComponents(cpSpace *space, cpFloat dt);

static void
updateBBCache(cpShape *shape, void *unused)
{
	cpBody *body = shape->body;
	cpShapeUpdate(shape, body->p, body->rot);
}

void
cpSpaceStep(cpSpace *space, cpFloat dt)
{
	if(!dt) return; // don't step if the timestep is 0!
	cpFloat dt_inv = 1.0f/dt;

	cpArray *bodies = space->bodies;
	cpArray *constraints = space->constraints;
	
	// Empty the arbiter list.
	space->arbiters->num = 0;

	// Integrate positions
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody *)bodies->arr[i];
		body->position_func(body, dt);
	}
	
	// Pre-cache BBoxes and shape data.
	cpSpatialIndexEach(space->activeShapes, (cpSpatialIndexIterator)updateBBCache, NULL);
	
	
	// Collide!
	cpSpaceLock(space);
	cpSpacePushFreshContactBuffer(space);
	cpSpatialIndexReindexQuery(space->activeShapes, (cpSpatialIndexQueryCallback)cpSpaceCollideShapes, space);
	cpSpaceUnlock(space);
	
	// If body sleeping is enabled, do that now.
	if(space->sleepTimeThreshold != INFINITY){
		cpSpaceProcessComponents(space, dt);
//		bodies = space->bodies; // rebuilt by processContactComponents()
	}
	
	// Clear out old cached arbiters and call separate callbacks
	cpHashSetFilter(space->contactSet, (cpHashSetFilterFunc)contactSetFilter, space);

	// Prestep the arbiters.
	cpArray *arbiters = space->arbiters;
	for(int i=0; i<arbiters->num; i++)
		cpArbiterPreStep((cpArbiter *)arbiters->arr[i], dt_inv);

	// Prestep the constraints.
	for(int i=0; i<constraints->num; i++){
		cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
		constraint->klass->preStep(constraint, dt, dt_inv);
	}

	// Integrate velocities.
	cpFloat damping = cpfpow(space->damping, dt);
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody *)bodies->arr[i];
		body->velocity_func(body, space->gravity, damping, dt);
	}

	for(int i=0; i<arbiters->num; i++)
		cpArbiterApplyCachedImpulse((cpArbiter *)arbiters->arr[i]);
	
	// Run the impulse solver.
	for(int i=0; i<space->iterations; i++){
		for(int j=0; j<arbiters->num; j++)
			cpArbiterApplyImpulse((cpArbiter *)arbiters->arr[j]);
			
		for(int j=0; j<constraints->num; j++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[j];
			constraint->klass->applyImpulse(constraint);
		}
	}
	
	
	// run the post solve callbacks
	cpSpaceLock(space);
	for(int i=0; i<arbiters->num; i++){
		cpArbiter *arb = (cpArbiter *) arbiters->arr[i];
		
		cpCollisionHandler *handler = arb->handler;
		handler->postSolve(arb, space, handler->data);
		
		arb->state = cpArbiterStateNormal;
	}
	cpSpaceUnlock(space);
	
	// Run the post step callbacks
	// Loop because post step callbacks may create more post step callbacks
	while(space->postStepCallbacks){
		cpHashSet *callbacks = space->postStepCallbacks;
		space->postStepCallbacks = NULL;
		
		cpHashSetEach(callbacks, (cpHashSetIterFunc)postStepCallbackSetIter, space);
		cpHashSetFree(callbacks);
	}
	
	// Increment the stamp.
	space->stamp++;
}
