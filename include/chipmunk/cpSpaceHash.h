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

// The spatial hash is Chipmunk's default (and currently only) spatial index type.
// Based on a chained hash table.

// Used internally to track objects added to the hash
typedef struct cpHandle{
	// Pointer to the object
	void *obj;
	// Retain count
	int retain;
	// Query stamp. Used to make sure two objects
	// aren't identified twice in the same query.
	cpTimestamp stamp;
} cpHandle;

// Linked list element for in the chains.
typedef struct cpSpaceHashBin{
	cpHandle *handle;
	struct cpSpaceHashBin *next;
} cpSpaceHashBin;

typedef struct cpSpaceHash{
	cpSpatialIndex spatialIndex;
	
	// Number of cells in the table.
	CP_PRIVATE(int numcells);
	// Dimentions of the cells.
	CP_PRIVATE(cpFloat celldim);
	
	// Hashset of the handles and the recycled ones.
	CP_PRIVATE(cpHashSet *handleSet);
	CP_PRIVATE(cpArray *pooledHandles);
	
	// The table and the recycled bins.
	CP_PRIVATE(cpSpaceHashBin **table);
	CP_PRIVATE(cpSpaceHashBin *pooledBins);
	
	// list of buffers to free on destruction.
	CP_PRIVATE(cpArray *allocatedBuffers);
	
	// Incremented on each query. See cpHandle.stamp.
	CP_PRIVATE(cpTimestamp stamp);
} cpSpaceHash;

//Basic allocation/destruction functions.
cpSpaceHash *cpSpaceHashAlloc(void);
cpSpaceHash *cpSpaceHashInit(cpSpaceHash *hash, cpFloat celldim, int cells, cpSpatialIndexBBFunc bbfunc);
cpSpaceHash *cpSpaceHashNew(cpFloat celldim, int cells, cpSpatialIndexBBFunc bbfunc);

// Resize the hashtable. (Does not rehash! You must call cpSpaceHashRehash() if needed.)
void cpSpaceHashResize(cpSpaceHash *hash, cpFloat celldim, int numcells);
