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

/*
	A simple but extremely inefficient broadphase.
	This is meant as an example of writting a broadphase algorithm for Chipmunk.
	Unless your game has < 10 shapes, you do NOT want to use this.
*/

#include <math.h>
#include <stdlib.h>

#include "chipmunk_private.h"

static cpSpatialIndexClass klass;

#pragma mark Memory Management Functions

/*
	Standard set of Chipmunk memory functions.
	All are optional except for the destroy function.
*/

typedef struct cpBruteIndex
{
	cpSpatialIndex spatialIndex;
	cpArray *arr;
} cpBruteIndex;

cpBruteIndex *
cpBruteIndexAlloc(void)
{
	return (cpBruteIndex *)cpcalloc(1, sizeof(cpBruteIndex));
}

cpSpatialIndex *
cpBruteIndexInit(cpBruteIndex *brute, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	cpSpatialIndexInit((cpSpatialIndex *)brute, &klass, bbfunc, staticIndex);
	
	brute->arr = cpArrayNew(0);
	
	return (cpSpatialIndex *)brute;
}

cpSpatialIndex *
cpBruteIndexNew(cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	return cpBruteIndexInit(cpBruteIndexAlloc(), bbfunc, staticIndex);
}

// This function is required.
// It should free any memory allocated in your init function.
// Do not free the broadphase object itself, that is done by cpSpatialIndexFree().
static void
cpBruteIndexDestroy(cpBruteIndex *brute)
{
	cpArrayFree(brute->arr);
}

#pragma mark Misc

// This function is required.
static int
cpBruteIndexCount(cpBruteIndex *brute)
{
	return brute->arr->num;
}

// This function is required.
static void
cpBruteIndexEach(cpBruteIndex *brute, cpSpatialIndexIteratorFunc func, void *data)
{
	void **arr = brute->arr->arr;
	for(int i=0, count=brute->arr->num; i<count; i++) func(arr[i], data);
}

// This function is required.
static int
cpBruteIndexContains(cpBruteIndex *brute, void *obj, cpHashValue hashid)
{
	return cpArrayContains(brute->arr, obj);
}

#pragma mark Basic Operations

// This function is required.
static void
cpBruteIndexInsert(cpBruteIndex *brute, void *obj, cpHashValue hashid)
{
	cpArrayPush(brute->arr, obj);
}

// This function is required.
static void
cpBruteIndexRemove(cpBruteIndex *brute, void *obj, cpHashValue hashid)
{
	cpArrayDeleteObj(brute->arr, obj);
}

#pragma mark Reindexing Functions

// This function is optional if you don't explicitly reindex anything.
static void
cpBruteIndexReindexObject(cpBruteIndex *brute, void *obj, cpHashValue hashid)
{
	// Nothing to do here
}

// This function is optional if you don't explicity reindex anything.
static void
cpBruteIndexReindex(cpBruteIndex *brute)
{
	// Nothing to do here
}

#pragma mark Query Functions

static void
cpBruteIndexQuery(cpBruteIndex *brute, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data)
{
	void **arr = brute->arr->arr;
	for(int i=0, count=brute->arr->num; i<count; i++){
		void *other = arr[i];
		if(obj != other) func(obj, other, data);
	}
}

static void
cpBruteIndexPointQuery(cpBruteIndex *brute, cpVect point, cpSpatialIndexQueryFunc func, void *data)
{
	// It's often easy to just implement point queries on top of rect queries.
	cpBruteIndexQuery(brute, &point, cpBBNew(point.x, point.y, point.x, point.y), func, data);
}

void
cpBruteIndexSegmentQuery(cpBruteIndex *brute, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	void **arr = brute->arr->arr;
	for(int i=0, count=brute->arr->num; i<count; i++){
		void *other = arr[i];
		
		
		func(obj, other, data);
	}
}

#pragma mark Reindex/Query

// This function is required
// It is called to reindex and collide the dynamic objects.
// It is also responsible for processing the static-dynamic collisions.
static void
cpBruteIndexReindexQuery(cpBruteIndex *brute, cpSpatialIndexQueryFunc func, void *data)
{
	void **arr = brute->arr->arr;
	int count = count=brute->arr->num;
	
	for(int i=1; i<count; i++){
		for(int j=0; j<i; j++){
			// Do not return a collision pair more than once.
			// There is no debug assertion to check for that and it
			// will cause an infinite loop in the contact graph code.
			func(arr[i], arr[j], data);
		}
	}
	
	// You MUST call this to process dynamic-static collisions.
	cpSpatialIndexCollideStatic((cpSpatialIndex *)brute, brute->spatialIndex.staticIndex, func, data);
}

static cpSpatialIndexClass klass = {
	(cpSpatialIndexDestroyImpl)cpBruteIndexDestroy,
	
	(cpSpatialIndexCountImpl)cpBruteIndexCount,
	(cpSpatialIndexEachImpl)cpBruteIndexEach,
	(cpSpatialIndexContainsImpl)cpBruteIndexContains,
	
	(cpSpatialIndexInsertImpl)cpBruteIndexInsert,
	(cpSpatialIndexRemoveImpl)cpBruteIndexRemove,
	
	(cpSpatialIndexReindexImpl)cpBruteIndexReindex,
	(cpSpatialIndexReindexObjectImpl)cpBruteIndexReindexObject,
	(cpSpatialIndexReindexQueryImpl)cpBruteIndexReindexQuery,
	
	(cpSpatialIndexPointQueryImpl)cpBruteIndexPointQuery,
	(cpSpatialIndexSegmentQueryImpl)cpBruteIndexSegmentQuery,
	(cpSpatialIndexQueryImpl)cpBruteIndexQuery,
};
