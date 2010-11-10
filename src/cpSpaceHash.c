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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "chipmunk.h"
#include "prime.h"

#pragma mark Handle Functions

static cpHandle*
cpHandleInit(cpHandle *hand, void *obj)
{
	hand->obj = obj;
	hand->retain = 0;
	hand->stamp = 0;
	
	return hand;
}

static inline void cpHandleRetain(cpHandle *hand){hand->retain++;}

static inline void
cpHandleRelease(cpHandle *hand, cpArray *pooledHandles)
{
	hand->retain--;
	if(hand->retain == 0) cpArrayPush(pooledHandles, hand);
}

static int handleSetEql(void *obj, cpHandle *hand){return (obj == hand->obj);}

static void *
handleSetTrans(void *obj, cpSpaceHash *hash)
{
	if(hash->pooledHandles->num == 0){
		// handle pool is exhausted, make more
		int count = CP_BUFFER_BYTES/sizeof(cpHandle);
		cpAssert(count, "Buffer size is too small.");
		
		cpHandle *buffer = (cpHandle *)cpmalloc(CP_BUFFER_BYTES);
		cpArrayPush(hash->allocatedBuffers, buffer);
		
		for(int i=0; i<count; i++) cpArrayPush(hash->pooledHandles, buffer + i);
	}
	
	cpHandle *hand = cpHandleInit((cpHandle *) cpArrayPop(hash->pooledHandles), obj);
	cpHandleRetain(hand);
	
	return hand;
}

#pragma mark Memory Management Functions

cpSpaceHash *
cpSpaceHashAlloc(void)
{
	return (cpSpaceHash *)cpcalloc(1, sizeof(cpSpaceHash));
}

// Frees the old table, and allocate a new one.
static void
cpSpaceHashAllocTable(cpSpaceHash *hash, int numcells)
{
	cpfree(hash->table);
	
	hash->numcells = numcells;
	hash->table = (cpSpaceHashBin **)cpcalloc(numcells, sizeof(cpSpaceHashBin *));
}

static cpSpatialIndexClass klass;

cpSpaceHash *
cpSpaceHashInit(cpSpaceHash *hash, cpFloat celldim, int numcells, cpSpatialIndexBBFunc bbfunc)
{
	hash->spatialIndex.klass = &klass;
	
	cpSpaceHashAllocTable(hash, next_prime(numcells));
	hash->celldim = celldim;
	hash->spatialIndex.bbfunc = bbfunc;
	
	hash->handleSet = cpHashSetNew(0, (cpHashSetEqlFunc)handleSetEql, (cpHashSetTransFunc)handleSetTrans);
	hash->pooledHandles = cpArrayNew(0);
	
	hash->pooledBins = NULL;
	hash->allocatedBuffers = cpArrayNew(0);
	
	hash->stamp = 1;
	
	return hash;
}

cpSpaceHash *
cpSpaceHashNew(cpFloat celldim, int cells, cpSpatialIndexBBFunc bbfunc)
{
	return cpSpaceHashInit(cpSpaceHashAlloc(), celldim, cells, bbfunc);
}

static inline void
recycleBin(cpSpaceHash *hash, cpSpaceHashBin *bin)
{
	bin->next = hash->pooledBins;
	hash->pooledBins = bin;
}

static inline void
clearHashCell(cpSpaceHash *hash, int idx)
{
	cpSpaceHashBin *bin = hash->table[idx];
	while(bin){
		cpSpaceHashBin *next = bin->next;
		
		cpHandleRelease(bin->handle, hash->pooledHandles);
		recycleBin(hash, bin);
		
		bin = next;
	}
	
	hash->table[idx] = NULL;
}

// Clear all cells in the hashtable.
static void
clearHash(cpSpaceHash *hash)
{
	for(int i=0; i<hash->numcells; i++)
		clearHashCell(hash, i);
}

static void freeWrap(void *ptr, void *unused){cpfree(ptr);}

static void
cpSpaceHashDestroy(cpSpaceHash *hash)
{
	clearHash(hash);
	
	cpHashSetFree(hash->handleSet);
	
	cpArrayEach(hash->allocatedBuffers, freeWrap, NULL);
	cpArrayFree(hash->allocatedBuffers);
	cpArrayFree(hash->pooledHandles);
	
	cpfree(hash->table);
}

#pragma mark TODO

// Return true if the chain contains the handle.
static inline cpBool
containsHandle(cpSpaceHashBin *bin, cpHandle *hand)
{
	while(bin){
		if(bin->handle == hand) return cpTrue;
		bin = bin->next;
	}
	
	return cpFalse;
}

// Get a recycled or new bin.
static inline cpSpaceHashBin *
getEmptyBin(cpSpaceHash *hash)
{
	cpSpaceHashBin *bin = hash->pooledBins;
	
	if(bin){
		hash->pooledBins = bin->next;
		return bin;
	} else {
		// Pool is exhausted, make more
		int count = CP_BUFFER_BYTES/sizeof(cpSpaceHashBin);
		cpAssert(count, "Buffer size is too small.");
		
		cpSpaceHashBin *buffer = (cpSpaceHashBin *)cpmalloc(CP_BUFFER_BYTES);
		cpArrayPush(hash->allocatedBuffers, buffer);
		
		// push all but the first one, return the first instead
		for(int i=1; i<count; i++) recycleBin(hash, buffer + i);
		return buffer;
	}
}

// The hash function itself.
static inline cpHashValue
hash_func(cpHashValue x, cpHashValue y, cpHashValue n)
{
	return (x*1640531513ul ^ y*2654435789ul) % n;
}

// Much faster than (int)floor(f)
// Profiling showed floor() to be a sizable performance hog
static inline int
floor_int(cpFloat f)
{
	int i = (int)f;
	return (f < 0.0f && f != i ? i - 1 : i);
}

static inline void
hashHandle(cpSpaceHash *hash, cpHandle *hand, cpBB bb)
{
	// Find the dimensions in cell coordinates.
	cpFloat dim = hash->celldim;
	int l = floor_int(bb.l/dim); // Fix by ShiftZ
	int r = floor_int(bb.r/dim);
	int b = floor_int(bb.b/dim);
	int t = floor_int(bb.t/dim);
	
	int n = hash->numcells;
	for(int i=l; i<=r; i++){
		for(int j=b; j<=t; j++){
			int idx = hash_func(i,j,n);
			cpSpaceHashBin *bin = hash->table[idx];
			
			// Don't add an object twice to the same cell.
			if(containsHandle(bin, hand)) continue;

			cpHandleRetain(hand);
			// Insert a new bin for the handle in this cell.
			cpSpaceHashBin *newBin = getEmptyBin(hash);
			newBin->handle = hand;
			newBin->next = bin;
			hash->table[idx] = newBin;
		}
	}
}

static void
cpSpaceHashInsert(cpSpaceHash *hash, void *obj, cpHashValue hashid, cpBB _deprecated_unused)
{
	cpHandle *hand = (cpHandle *)cpHashSetInsert(hash->handleSet, hashid, obj, hash);
	hashHandle(hash, hand, hash->spatialIndex.bbfunc(obj));
}

static void
cpSpaceHashRehashObject(cpSpaceHash *hash, void *obj, cpHashValue hashid)
{
	cpHandle *hand = (cpHandle *)cpHashSetRemove(hash->handleSet, hashid, obj);
	
	if(hand){
		hand->obj = NULL;
		cpHandleRelease(hand, hash->pooledHandles);
		
		cpSpaceHashInsert(hash, obj, hashid, cpBBNew(0.0f, 0.0f, 0.0f, 0.0f));
	}
}

static void handleRehashHelper(cpHandle *hand, cpSpaceHash *hash){hashHandle(hash, hand, hash->spatialIndex.bbfunc(hand->obj));}

static void
cpSpaceHashRehash(cpSpaceHash *hash)
{
	clearHash(hash);
	cpHashSetEach(hash->handleSet, (cpHashSetIterFunc)handleRehashHelper, hash);
}

static void
cpSpaceHashRemove(cpSpaceHash *hash, void *obj, cpHashValue hashid)
{
	cpHandle *hand = (cpHandle *)cpHashSetRemove(hash->handleSet, hashid, obj);
	
	if(hand){
		hand->obj = NULL;
		cpHandleRelease(hand, hash->pooledHandles);
	}
}

typedef struct eachContext {
	cpSpatialIndexIterator func;
	void *data;
} eachContext;

static void eachHelper(cpHandle *hand, eachContext *context){context->func(hand->obj, context->data);}

static void
cpSpaceHashEach(cpSpaceHash *hash, cpSpatialIndexIterator func, void *data)
{
	eachContext context = {func, data};
	cpHashSetEach(hash->handleSet, (cpHashSetIterFunc)eachHelper, &context);
}

static void
removeOrphanedHandles(cpSpaceHash *hash, cpSpaceHashBin **bin_ptr)
{
	cpSpaceHashBin *bin = *bin_ptr;
	while(bin){
		cpHandle *hand = bin->handle;
		cpSpaceHashBin *next = bin->next;
		
		if(!hand->obj){
			// orphaned handle, unlink and recycle the bin
			(*bin_ptr) = bin->next;
			recycleBin(hash, bin);
			
			cpHandleRelease(hand, hash->pooledHandles);
		} else {
			bin_ptr = &bin->next;
		}
		
		bin = next;
	}
}

// Calls the callback function for the objects in a given chain.
static inline void
query(cpSpaceHash *hash, cpSpaceHashBin **bin_ptr, void *obj, cpSpatialIndexQueryCallback func, void *data)
{
	restart:
	for(cpSpaceHashBin *bin = *bin_ptr; bin; bin = bin->next){
		cpHandle *hand = bin->handle;
		void *other = hand->obj;
		
		if(hand->stamp == hash->stamp || obj == other){
			continue;
		} else if(other){
			func(obj, other, data);
			hand->stamp = hash->stamp;
		} else {
			// The object for this handle has been removed
			// cleanup this cell and restart the query
			removeOrphanedHandles(hash, bin_ptr);
			goto restart; // GCC not smart enough/able to tail call an inlined function.
		}
	}
}

static void
cpSpaceHashPointQuery(cpSpaceHash *hash, cpVect point, cpSpatialIndexQueryCallback func, void *data)
{
	cpFloat dim = hash->celldim;
	int idx = hash_func(floor_int(point.x/dim), floor_int(point.y/dim), hash->numcells);  // Fix by ShiftZ
	
	query(hash, &hash->table[idx], &point, func, data);
	hash->stamp++;
}

static void
cpSpaceHashQuery(cpSpaceHash *hash, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	// Get the dimensions in cell coordinates.
	cpFloat dim = hash->celldim;
	int l = floor_int(bb.l/dim);  // Fix by ShiftZ
	int r = floor_int(bb.r/dim);
	int b = floor_int(bb.b/dim);
	int t = floor_int(bb.t/dim);
	
	int n = hash->numcells;
	cpSpaceHashBin **table = hash->table;
	
	// Iterate over the cells and query them.
	for(int i=l; i<=r; i++){
		for(int j=b; j<=t; j++){
			query(hash, &table[hash_func(i,j,n)], obj, func, data);
		}
	}
	
	hash->stamp++;
}

// Similar to struct eachPair above.
typedef struct queryRehashContext {
	cpSpaceHash *hash;
	cpSpatialIndexQueryCallback func;
	void *data;
} queryRehashContext;

// Hashset iterator func used with cpSpaceHashQueryRehash().
static void
handleQueryRehashHelper(cpHandle *hand, queryRehashContext *context)
{
	cpSpaceHash *hash = context->hash;
	cpSpatialIndexQueryCallback func = context->func;
	void *data = context->data;

	cpFloat dim = hash->celldim;
	int n = hash->numcells;

	void *obj = hand->obj;
	cpBB bb = hash->spatialIndex.bbfunc(obj);

	int l = floor_int(bb.l/dim);
	int r = floor_int(bb.r/dim);
	int b = floor_int(bb.b/dim);
	int t = floor_int(bb.t/dim);
	
	cpSpaceHashBin **table = hash->table;

	for(int i=l; i<=r; i++){
		for(int j=b; j<=t; j++){
			int idx = hash_func(i,j,n);
			cpSpaceHashBin *bin = table[idx];
			
			if(containsHandle(bin, hand)) continue;
			
			cpHandleRetain(hand); // this MUST be done first in case the object is removed in func()
			query(hash, &bin, obj, func, data);
			
			cpSpaceHashBin *newBin = getEmptyBin(hash);
			newBin->handle = hand;
			newBin->next = bin;
			table[idx] = newBin;
		}
	}
	
	// Increment the stamp for each object hashed.
	hash->stamp++;
}

static void
cpSpaceHashReindexCollide(cpSpaceHash *hash, cpSpatialIndex *staticIndex, cpSpatialIndexQueryCallback func, void *data)
{
	clearHash(hash);
	
	queryRehashContext context = {hash, func, data};
	cpHashSetEach(hash->handleSet, (cpHashSetIterFunc)handleQueryRehashHelper, &context);
	
	cpSpatialIndexCollideStatic((cpSpatialIndex *)hash, staticIndex, func, data);
}

static inline cpFloat
segmentQuery(cpSpaceHash *hash, cpSpaceHashBin **bin_ptr, void *obj, cpSpatialIndexSegmentQueryCallback func, void *data)
{
	cpFloat t = 1.0f;
	 
	restart:
	for(cpSpaceHashBin *bin = *bin_ptr; bin; bin = bin->next){
		cpHandle *hand = bin->handle;
		void *other = hand->obj;
		
		// Skip over certain conditions
		if(hand->stamp == hash->stamp){
			continue;
		} else if(other){
			t = cpfmin(t, func(obj, other, data));
			hand->stamp = hash->stamp;
		} else {
			// The object for this handle has been removed
			// cleanup this cell and restart the query
			removeOrphanedHandles(hash, bin_ptr);
			goto restart; // GCC not smart enough/able to tail call an inlined function.
		}
	}
	
	return t;
}

// modified from http://playtechs.blogspot.com/2007/03/raytracing-on-grid.html
void
cpSpaceHashSegmentQuery(cpSpaceHash *hash, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryCallback func, void *data)
{
	a = cpvmult(a, 1.0f/hash->celldim);
	b = cpvmult(b, 1.0f/hash->celldim);
	
	int cell_x = floor_int(a.x), cell_y = floor_int(a.y);

	cpFloat t = 0;

	int x_inc, y_inc;
	cpFloat temp_v, temp_h;

	if (b.x > a.x){
		x_inc = 1;
		temp_h = (cpffloor(a.x + 1.0f) - a.x);
	} else {
		x_inc = -1;
		temp_h = (a.x - cpffloor(a.x));
	}

	if (b.y > a.y){
		y_inc = 1;
		temp_v = (cpffloor(a.y + 1.0f) - a.y);
	} else {
		y_inc = -1;
		temp_v = (a.y - cpffloor(a.y));
	}
	
	// Division by zero is *very* slow on ARM
	cpFloat dx = cpfabs(b.x - a.x), dy = cpfabs(b.y - a.y);
	cpFloat dt_dx = (dx ? 1.0f/dx : INFINITY), dt_dy = (dy ? 1.0f/dy : INFINITY);
	
	// fix NANs in horizontal directions
	cpFloat next_h = (temp_h ? temp_h*dt_dx : dt_dx);
	cpFloat next_v = (temp_v ? temp_v*dt_dy : dt_dy);
	
	cpSpaceHashBin **table = hash->table;

	int n = hash->numcells;
	while(t < t_exit){
		int idx = hash_func(cell_x, cell_y, n);
		t_exit = cpfmin(t_exit, segmentQuery(hash, &table[idx], obj, func, data));

		if (next_v < next_h){
			cell_y += y_inc;
			t = next_v;
			next_v += dt_dy;
		} else {
			cell_x += x_inc;
			t = next_h;
			next_h += dt_dx;
		}
	}
	
	hash->stamp++;
}

#pragma mark Misc

void
cpSpaceHashResize(cpSpaceHash *hash, cpFloat celldim, int numcells)
{
	if(hash->spatialIndex.klass != &klass){
		cpAssertWarn(cpFalse, "Ignoring cpSpaceHashResize() call to non-cpSpaceHash spatial index.");
		return;
	}
	
	// Clear the hash to release the old handle locks.
	clearHash(hash);
	
	hash->celldim = celldim;
	cpSpaceHashAllocTable(hash, next_prime(numcells));
}

static int
cpSpaceHashCount(cpSpaceHash *hash)
{
	return hash->handleSet->entries;
}

static int
cpSpaceHashContains(cpSpaceHash *hash, void *obj, cpHashValue hashid)
{
	return cpHashSetFind(hash->handleSet, hashid, obj) != NULL;
}

static cpSpatialIndexClass klass = {
	(cpSpatialIndexDestroyFunc)cpSpaceHashDestroy,
	
	(cpSpatialIndexCountFunc)cpSpaceHashCount,
	(cpSpatialIndexEachFunc)cpSpaceHashEach,
	(cpSpatialIndexContainsFunc)cpSpaceHashContains,
	
	(cpSpatialIndexInsertFunc)cpSpaceHashInsert,
	(cpSpatialIndexRemoveFunc)cpSpaceHashRemove,
	
	(cpSpatialIndexReindexFunc)cpSpaceHashRehash,
	(cpSpatialIndexReindexObjectFunc)cpSpaceHashRehashObject,
	
	(cpSpatialIndexPointQueryFunc)cpSpaceHashPointQuery,
	(cpSpatialIndexSegmentQueryFunc)cpSpaceHashSegmentQuery,
	(cpSpatialIndexQueryFunc)cpSpaceHashQuery,
	(cpSpatialIndexReindexCollideFunc)cpSpaceHashReindexCollide,
};
