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

#define CP_ALLOW_PRIVATE_ACCESS 1
#include "chipmunk.h"

#define CP_HASH_COEF (3344921057ul)
#define CP_HASH_PAIR(A, B) ((cpHashValue)(A)*CP_HASH_COEF ^ (cpHashValue)(B)*CP_HASH_COEF)

#pragma mark cpArray

struct cpArray {
	int num, max;
	void **arr;
};

cpArray *cpArrayNew(int size);

//void cpArrayDestroy(cpArray *arr);
void cpArrayFree(cpArray *arr);

void cpArrayPush(cpArray *arr, void *object);
void *cpArrayPop(cpArray *arr);
void cpArrayDeleteObj(cpArray *arr, void *obj);
cpBool cpArrayContains(cpArray *arr, void *ptr);

void cpArrayFreeEach(cpArray *arr, void (freeFunc)(void*));

#pragma mark Foreach loops

#define CP_BODY_FOREACH_CONSTRAINT(body, var)\
	for(cpConstraint *var = body->constraintList; var; var = (var->a == body ? var->next_a : var->next_b))

#define CP_BODY_FOREACH_ARBITER(bdy, var)\
	for(cpArbiter *var = bdy->arbiterList; var; var = (var->a->body == bdy ? var->next_a : var->next_b))

#define CP_BODY_FOREACH_SHAPE(body, var)\
	for(cpShape *var = body->shapeList; var; var = var->next)

#define CP_BODY_FOREACH_COMPONENT(root, var)\
	for(cpBody *var = root; var; var = var->node.next)

#pragma mark cpHashSet

typedef cpBool (*cpHashSetEqlFunc)(void *ptr, void *elt);
typedef void *(*cpHashSetTransFunc)(void *ptr, void *data);

cpHashSet *cpHashSetNew(int size, cpHashSetEqlFunc eqlFunc);
void cpHashSetSetDefaultValue(cpHashSet *set, void *default_value);

//void cpHashSetDestroy(cpHashSet *set);
void cpHashSetFree(cpHashSet *set);

int cpHashSetCount(cpHashSet *set);
void *cpHashSetInsert(cpHashSet *set, cpHashValue hash, void *ptr, void *data, cpHashSetTransFunc trans);
void *cpHashSetRemove(cpHashSet *set, cpHashValue hash, void *ptr);
void *cpHashSetFind(cpHashSet *set, cpHashValue hash, void *ptr);

typedef void (*cpHashSetIteratorFunc)(void *elt, void *data);
void cpHashSetEach(cpHashSet *set, cpHashSetIteratorFunc func, void *data);

typedef cpBool (*cpHashSetFilterFunc)(void *elt, void *data);
void cpHashSetFilter(cpHashSet *set, cpHashSetFilterFunc func, void *data);

#pragma mark Arbiters

struct cpContact {
	cpVect p, n;
	cpFloat dist;
	
	cpVect r1, r2;
	cpFloat nMass, tMass, bounce;

	cpFloat jnAcc, jtAcc, jBias;
	cpFloat bias;
	
	cpHashValue hash;
};

cpContact* cpContactInit(cpContact *con, cpVect p, cpVect n, cpFloat dist, cpHashValue hash);
cpArbiter* cpArbiterInit(cpArbiter *arb, cpShape *a, cpShape *b);

void cpArbiterUpdate(cpArbiter *arb, cpContact *contacts, int numContacts, struct cpCollisionHandler *handler, cpShape *a, cpShape *b);
void cpArbiterPreStep(cpArbiter *arb, cpFloat dt, cpFloat bias, cpFloat slop);
void cpArbiterApplyCachedImpulse(cpArbiter *arb, cpFloat dt_coef);
void cpArbiterApplyImpulse(cpArbiter *arb);

#pragma mark Body Functions

void cpBodyAddShape(cpBody *body, cpShape *shape);
void cpBodyRemoveShape(cpBody *body, cpShape *shape);

#pragma mark Shape/Collision Functions

cpShape* cpShapeInit(cpShape *shape, const cpShapeClass *klass, cpBody *body);

int cpCollideShapes(const cpShape *a, const cpShape *b, cpContact *arr);

static inline cpFloat
cpPolyShapeValueOnAxis(const cpPolyShape *poly, const cpVect n, const cpFloat d)
{
	cpVect *verts = poly->tVerts;
	cpFloat min = cpvdot(n, verts[0]);
	
	for(int i=1; i<poly->numVerts; i++){
		min = cpfmin(min, cpvdot(n, verts[i]));
	}
	
	return min - d;
}

static inline cpBool
cpPolyShapeContainsVert(const cpPolyShape *poly, const cpVect v)
{
	cpPolyShapeAxis *axes = poly->tAxes;
	
	for(int i=0; i<poly->numVerts; i++){
		cpFloat dist = cpvdot(axes[i].n, v) - axes[i].d;
		if(dist > 0.0f) return cpFalse;
	}
	
	return cpTrue;
}

static inline cpBool
cpPolyShapeContainsVertPartial(const cpPolyShape *poly, const cpVect v, const cpVect n)
{
	cpPolyShapeAxis *axes = poly->tAxes;
	
	for(int i=0; i<poly->numVerts; i++){
		if(cpvdot(axes[i].n, n) < 0.0f) continue;
		cpFloat dist = cpvdot(axes[i].n, v) - axes[i].d;
		if(dist > 0.0f) return cpFalse;
	}
	
	return cpTrue;
}

#pragma mark Spatial Index Functions

cpSpatialIndex *cpSpatialIndexInit(cpSpatialIndex *index, cpSpatialIndexClass *klass, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

#pragma mark Space Functions

extern cpCollisionHandler cpDefaultCollisionHandler;
void cpSpaceProcessComponents(cpSpace *space, cpFloat dt);

void cpSpacePushFreshContactBuffer(cpSpace *space);
cpContact *cpContactBufferGetArray(cpSpace *space);
void cpSpacePushContacts(cpSpace *space, int count);
void cpSpacePopContacts(cpSpace *space, int count);

void *cpSpaceGetPostStepData(cpSpace *space, void *obj);

typedef struct cpPostStepCallback cpPostStepCallback;
//void cpSpacePostStepCallbackSetIter(cpPostStepCallback *callback, cpSpace *space);

cpBool cpSpaceArbiterSetFilter(cpArbiter *arb, cpSpace *space);
void *cpSpaceArbiterSetTrans(cpShape **shapes, cpSpace *space);
void cpShapeUpdateFunc(cpShape *shape, void *unused);

//void cpSpaceCollideShapes(cpShape *a, cpShape *b, cpSpace *space);
void cpSpaceActivateBody(cpSpace *space, cpBody *body);
void cpSpaceLock(cpSpace *space);
void cpSpaceUnlock(cpSpace *space, cpBool runPostStep);
