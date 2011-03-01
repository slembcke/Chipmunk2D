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

#pragma mark cpArray

struct cpArray {
	int num, max;
	void **arr;
};

// TODO get rid of reference versions?
cpArray *cpArrayAlloc(void);
cpArray *cpArrayInit(cpArray *arr, int size);
cpArray *cpArrayNew(int size);

void cpArrayDestroy(cpArray *arr);
void cpArrayFree(cpArray *arr);

void cpArrayPush(cpArray *arr, void *object);
void *cpArrayPop(cpArray *arr);
void cpArrayDeleteObj(cpArray *arr, void *obj);
cpBool cpArrayContains(cpArray *arr, void *ptr);

void cpArrayFreeEach(cpArray *arr, void (freeFunc)(void*));

#pragma mark Foreach loops

#define CP_BODY_FOREACH_CONSTRAINT(body, var)\
	for(cpConstraint *var = body->constraintList; var; var = (var->a == body ? var->nextA : var->nextB))

#define CP_BODY_FOREACH_ARBITER(bdy, var)\
	for(cpArbiter *var = bdy->arbiterList; var; var = (var->a->body == bdy ? var->nextA : var->nextB))

#define CP_BODY_FOREACH_SHAPE(body, var)\
	for(cpShape *var = body->shapeList; var; var = var->next)

#define CP_BODY_FOREACH_COMPONENT(root, var)\
	for(cpBody *var = root; var; var = var->node.next)

#pragma mark cpHashSet

typedef cpBool (*cpHashSetEqlFunc)(void *ptr, void *elt);
typedef void *(*cpHashSetTransFunc)(void *ptr, void *data);

// TODO get rid of reference versions?
cpHashSet *cpHashSetAlloc(void);
cpHashSet *cpHashSetInit(cpHashSet *set, int size, cpHashSetEqlFunc eqlFunc, void *defaultValue);
cpHashSet *cpHashSetNew(int size, cpHashSetEqlFunc eqlFunc, void *defaultValue);

void cpHashSetDestroy(cpHashSet *set);
void cpHashSetFree(cpHashSet *set);

int cpHashSetCount(cpHashSet *set);
void *cpHashSetInsert(cpHashSet *set, cpHashValue hash, void *ptr, void *data, cpHashSetTransFunc trans);
void *cpHashSetRemove(cpHashSet *set, cpHashValue hash, void *ptr);
void *cpHashSetFind(cpHashSet *set, cpHashValue hash, void *ptr);

typedef void (*cpHashSetIterFunc)(void *elt, void *data);
void cpHashSetEach(cpHashSet *set, cpHashSetIterFunc func, void *data);

typedef cpBool (*cpHashSetFilterFunc)(void *elt, void *data);
void cpHashSetFilter(cpHashSet *set, cpHashSetFilterFunc func, void *data);

#pragma mark Arbiters

cpContact* cpContactInit(cpContact *con, cpVect p, cpVect n, cpFloat dist, cpHashValue hash);
cpArbiter* cpArbiterInit(cpArbiter *arb, cpShape *a, cpShape *b);

void cpArbiterUpdate(cpArbiter *arb, cpContact *contacts, int numContacts, struct cpCollisionHandler *handler, cpShape *a, cpShape *b);
void cpArbiterPreStep(cpArbiter *arb, cpFloat dt_inv, cpFloat bias, cpFloat slop);
void cpArbiterApplyCachedImpulse(cpArbiter *arb, cpFloat dt_coef);
void cpArbiterApplyImpulse(cpArbiter *arb);

#pragma mark Collision Functions

int cpCollideShapes(const cpShape *a, const cpShape *b, cpContact *arr);

static inline cpFloat
cpPolyShapeValueOnAxis(const cpPolyShape *poly, const cpVect n, const cpFloat d)
{
	cpVect *verts = poly->CP_PRIVATE(tVerts);
	cpFloat min = cpvdot(n, verts[0]);
	
	for(int i=1; i<poly->CP_PRIVATE(numVerts); i++){
		min = cpfmin(min, cpvdot(n, verts[i]));
	}
	
	return min - d;
}

static inline cpBool
cpPolyShapeContainsVert(const cpPolyShape *poly, const cpVect v)
{
	cpPolyShapeAxis *axes = poly->CP_PRIVATE(tAxes);
	
	for(int i=0; i<poly->CP_PRIVATE(numVerts); i++){
		cpFloat dist = cpvdot(axes[i].n, v) - axes[i].d;
		if(dist > 0.0f) return cpFalse;
	}
	
	return cpTrue;
}

static inline cpBool
cpPolyShapeContainsVertPartial(const cpPolyShape *poly, const cpVect v, const cpVect n)
{
	cpPolyShapeAxis *axes = poly->CP_PRIVATE(tAxes);
	
	for(int i=0; i<poly->CP_PRIVATE(numVerts); i++){
		if(cpvdot(axes[i].n, n) < 0.0f) continue;
		cpFloat dist = cpvdot(axes[i].n, v) - axes[i].d;
		if(dist > 0.0f) return cpFalse;
	}
	
	return cpTrue;
}

#pragma mark Spatial Index Functions

cpSpatialIndex *cpSpatialIndexInit(cpSpatialIndex *index, cpSpatialIndexClass *klass, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

#pragma mark Space Functions

cpContact *cpContactBufferGetArray(cpSpace *space);
void cpSpacePushContacts(cpSpace *space, int count);
void cpSpaceCollideShapes(cpShape *a, cpShape *b, cpSpace *space);
void cpSpaceActivateBody(cpSpace *space, cpBody *body);
void cpSpaceLock(cpSpace *space);
void cpSpaceUnlock(cpSpace *space);
