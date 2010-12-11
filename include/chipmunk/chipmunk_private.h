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


#pragma mark cpHashSet

typedef cpBool (*cpHashSetEqlFunc)(void *ptr, void *elt);
typedef void *(*cpHashSetTransFunc)(void *ptr, void *data);

// TODO get rid of reference versions?
cpHashSet *cpHashSetAlloc(void);
cpHashSet *cpHashSetInit(cpHashSet *set, int size, cpHashSetEqlFunc eqlFunc, cpHashSetTransFunc trans, void *defaultValue);
cpHashSet *cpHashSetNew(int size, cpHashSetEqlFunc eqlFunc, cpHashSetTransFunc trans, void *defaultValue);

void cpHashSetDestroy(cpHashSet *set);
void cpHashSetFree(cpHashSet *set);

int cpHashSetCount(cpHashSet *set);
void *cpHashSetInsert(cpHashSet *set, cpHashValue hash, void *ptr, void *data);
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
void cpArbiterPreStep(cpArbiter *arb, cpFloat dt_inv);
void cpArbiterApplyCachedImpulse(cpArbiter *arb);
void cpArbiterApplyImpulse(cpArbiter *arb);

#pragma mark Collision Functions

int cpCollideShapes(const cpShape *a, const cpShape *b, cpContact *arr);

static inline cpFloat
cpPolyShapeValueOnAxis(const cpPolyShape *poly, const cpVect n, const cpFloat d)
{
	cpVect *verts = poly->CP_PRIVATE(tVerts);
	cpFloat min = cpvdot(n, verts[0]);
	
	int i;
	for(i=1; i<poly->CP_PRIVATE(numVerts); i++)
		min = cpfmin(min, cpvdot(n, verts[i]));
	
	return min - d;
}

static inline cpBool
cpPolyShapeContainsVert(const cpPolyShape *poly, const cpVect v)
{
	cpPolyShapeAxis *axes = poly->CP_PRIVATE(tAxes);
	
	int i;
	for(i=0; i<poly->CP_PRIVATE(numVerts); i++){
		cpFloat dist = cpvdot(axes[i].n, v) - axes[i].d;
		if(dist > 0.0f) return cpFalse;
	}
	
	return cpTrue;
}

static inline cpBool
cpPolyShapeContainsVertPartial(const cpPolyShape *poly, const cpVect v, const cpVect n)
{
	cpPolyShapeAxis *axes = poly->CP_PRIVATE(tAxes);
	
	int i;
	for(i=0; i<poly->CP_PRIVATE(numVerts); i++){
		if(cpvdot(axes[i].n, n) < 0.0f) continue;
		cpFloat dist = cpvdot(axes[i].n, v) - axes[i].d;
		if(dist > 0.0f) return cpFalse;
	}
	
	return cpTrue;
}

#pragma mark Space Functions

void cpSpaceActivateBody(cpSpace *space, cpBody *body);

static inline void
cpSpaceLock(cpSpace *space)
{
	space->locked++;
}

static inline void
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
