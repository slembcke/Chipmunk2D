/' Copyright (c) 2010 Scott Lembcke
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
 '/



/'*
	@defgroup cpSpatialIndex cpSpatialIndex
	
	Spatial indexes are data structures that are used to accelerate collision detection
	and spatial queries. Chipmunk provides a number of spatial index algorithms to pick from
	and they are programmed in a generic way so that you can use them for holding more than
	just cpShape structs.
	
	It works by using @c void pointers to the objects you add and using a callback to ask your code
	for bounding boxes when it needs them. Several types of queries can be performed an index as well
	as reindexing and full collision information. All communication to the spatial indexes is performed
	through callback functions.
	
	Spatial indexes should be treated as opaque structs.
	This meanns you shouldn't be reading any of the struct fields.
	@{
'/

#ifndef CHIPMUNK_SPACIALINDEX
#define CHIPMUNK_SPACIALINDEX

extern "c"

''MARK: Spatial Index



''/ Spatial index bounding box callback function type.

''/ The spatial index calls this function and passes you a pointer to an object you added

''/ when it needs to get the bounding box associated with that object.

type cpSpatialIndexBBFunc as function(byval obj as any ptr) as cpBB

''/ Spatial index/object iterator callback function type.

type cpSpatialIndexIteratorFunc as sub(byval obj as any ptr, byval data as any ptr)

''/ Spatial query callback function type.

type cpSpatialIndexQueryFunc as sub(byval obj1 as any ptr, byval obj2 as any ptr, byval data as any ptr)

''/ Spatial segment query callback function type.

type cpSpatialIndexSegmentQueryFunc as function(byval obj1 as any ptr, byval obj2 as any ptr, byval data as any ptr) as cpFloat


type cpSpatialIndexClassPTR as cpSpatialIndexClass ptr
type cpSpatialIndexPTR as cpSpatialIndex ptr


''/ @private

type cpSpatialIndex

	as cpSpatialIndexClassPTR klass

	

	as cpSpatialIndexBBFunc bbfunc

	

	as cpSpatialIndexPTR staticIndex, dynamicIndex

end type





''MARK: Spatial Hash



type cpSpaceHash as cpSpaceHash



''/ Allocate a spatial hash.

declare function cpSpaceHashAlloc() as cpSpaceHash ptr

''/ Initialize a spatial hash. 

declare function cpSpaceHashInit(byval hash as cpSpaceHash ptr, byval celldim as cpFloat, byval numcells as integer, byval bbfunc as cpSpatialIndexBBFunc, byval staticIndex as cpSpatialIndex ptr) as cpSpatialIndex ptr

''/ Allocate and initialize a spatial hash.

declare function cpSpaceHashNew(byval celldim as cpFloat, byval cells as integer, byval bbfunc as cpSpatialIndexBBFunc, byval staticIndex as cpSpatialIndex ptr) as cpSpatialIndex ptr



''/ Change the cell dimensions and table size of the spatial hash to tune it.

''/ The cell dimensions should roughly match the average size of your objects

''/ and the table size should be ~10 larger than the number of objects inserted.

''/ Some trial and error is required to find the optimum numbers for efficiency.

declare sub cpSpaceHashResize(byval hash as cpSpaceHash ptr, byval celldim as cpFloat, byval numcells as integer)



''MARK: AABB Tree



type as cpBBTree cpBBTree



''/ Allocate a bounding box tree.

declare function cpBBTreeAlloc() as cpBBTree ptr

''/ Initialize a bounding box tree.

declare function cpBBTreeInit(byval tree as cpBBTree ptr, byval bbfunc as cpSpatialIndexBBFunc, byval staticIndex as cpSpatialIndex ptr) as cpSpatialIndex ptr

''/ Allocate and initialize a bounding box tree.

declare function cpBBTreeNew(byval bbfunc as cpSpatialIndexBBFunc, byval staticIndex as cpSpatialIndex ptr) as cpSpatialIndex ptr



''/ Perform a static top down optimization of the tree.

declare sub cpBBTreeOptimize(byval index as cpSpatialIndex ptr)



''/ Bounding box tree velocity callback function.

''/ This function should return an estimate for the object's velocity.

type cpBBTreeVelocityFunc as function(byval obj as any ptr) as cpVect

''/ Set the velocity function for the bounding box tree to enable temporal coherence.

declare sub cpBBTreeSetVelocityFunc(byval index as cpSpatialIndex ptr, byval func as cpBBTreeVelocityFunc)



''MARK: Single Axis Sweep



type as cpSweep1D cpSweep1D



''/ Allocate a 1D sort and sweep broadphase.

declare function cpSweep1DAlloc() as cpSweep1D ptr

''/ Initialize a 1D sort and sweep broadphase.

declare function cpSweep1DInit(byval sweep as cpSweep1D ptr, byval bbfunc as cpSpatialIndexBBFunc, byval staticIndex as cpSpatialIndex ptr) as cpSpatialIndex ptr

''/ Allocate and initialize a 1D sort and sweep broadphase.

declare function cpSweep1DNew(byval bbfunc as cpSpatialIndexBBFunc, byval staticIndex as cpSpatialIndex ptr) as cpSpatialIndex ptr



''MARK: Spatial Index Implementation



type cpSpatialIndexDestroyImpl as sub(byval index as cpSpatialIndex ptr)



type cpSpatialIndexCountImpl as function(byval index as cpSpatialIndex ptr) as integer

type cpSpatialIndexEachImpl as sub(byval index as cpSpatialIndex ptr, byval func as cpSpatialIndexIteratorFunc, byval data as any ptr)



type cpSpatialIndexContainsImpl as function(byval index as cpSpatialIndex ptr, byval obj as any ptr, byval hashid as cpHashValue) as cpBool

type cpSpatialIndexInsertImpl as sub(byval index as cpSpatialIndex ptr, byval obj as any ptr, byval hashid as cpHashValue)

type cpSpatialIndexRemoveImpl as sub(byval index as cpSpatialIndex ptr, byval obj as any ptr, byval hashid as cpHashValue)



type cpSpatialIndexReindexImpl as sub(byval index as cpSpatialIndex ptr)

type cpSpatialIndexReindexObjectImpl as sub(byval index as cpSpatialIndex ptr, byval obj as any ptr, byval hashid as cpHashValue)

type cpSpatialIndexReindexQueryImpl as sub(byval index as cpSpatialIndex ptr, byval func as cpSpatialIndexQueryFunc, byval data as any ptr)



type cpSpatialIndexQueryImpl as sub(byval index as cpSpatialIndex ptr, byval obj as any ptr, byval bb as cpBB, byval func as cpSpatialIndexQueryFunc, byval data as any ptr)

type cpSpatialIndexSegmentQueryImpl as sub(byval index as cpSpatialIndex ptr, byval obj as any ptr, byval a as cpVect, byval b as cpVect, byval t_exit as cpFloat, byval func as cpSpatialIndexSegmentQueryFunc, byval data as any ptr)



type cpSpatialIndexClass

	as cpSpatialIndexDestroyImpl destroy

	

	as cpSpatialIndexCountImpl count

	as cpSpatialIndexEachImpl each

	

	as cpSpatialIndexContainsImpl contains

	as cpSpatialIndexInsertImpl insert

	as cpSpatialIndexRemoveImpl remove

	

	as cpSpatialIndexReindexImpl reindex

	as cpSpatialIndexReindexObjectImpl reindexObject

	as cpSpatialIndexReindexQueryImpl reindexQuery

	

	as cpSpatialIndexQueryImpl query

	as cpSpatialIndexSegmentQueryImpl segmentQuery

end type



''/ Destroy and free a spatial index.

declare sub cpSpatialIndexFree(byval index as cpSpatialIndex ptr)

''/ Collide the objects in @c dynamicIndex against the objects in @c staticIndex using the query callback function.

declare sub cpSpatialIndexCollideStatic(byval dynamicIndex as cpSpatialIndex ptr, byval staticIndex as cpSpatialIndex ptr, byval func as cpSpatialIndexQueryFunc, byval data as any ptr)



''/ Destroy a spatial index.
#ifndef cpSpactialIndexDestroy
#define cpSpactialIndexDestroy( index )	if (index->klass) index->klass->destroy(index)
#endif


''/ Get the number of objects in the spatial index.
#ifndef cpSpatialIndexCount
#define cpSpatialIndexCount( index )	(index)->klass->count( index )
#endif


''/ Iterate the objects in the spatial index. @c func will be called once for each object.
#ifndef cpSpatialIndexEach
#define cpSpatialIndexEach( index, func, _data )	(index)->klass->each( index, func, _data )
#endif

''/ Returns true if the spatial index contains the given object.
''/ Most spatial indexes use hashed storage, so you must provide a hash value too.
#ifndef cpSpatialIndexContains
#define cpSpatialIndexContains( index, obj, hashid )	(index)->klass->contains( index, obj, hashid )
#endif


''/ Add an object to a spatial index.
''/ Most spatial indexes use hashed storage, so you must provide a hash value too.
#ifndef cpSpatialIndexInsert
#define cpSpatialIndexInsert( index, obj, hashid )	(index)->klass->insert( index, obj, hashid )
#endif


''/ Remove an object from a spatial index.
''/ Most spatial indexes use hashed storage, so you must provide a hash value too.
#ifndef cpSpatialIndexRemove
#define cpSpatialIndexRemove( index, obj, hashid )	(index)->klass->remove( index, obj, hashid )
#endif


''/ Perform a full reindex of a spatial index.
#ifndef cpSpatialIndexReindex
#define cpSpatialIndexReindex( index )	index->klass->reindex( index )
#endif


''/ Reindex a single object in the spatial index.
#ifndef cpSpatialIndeReindexObject
#define cpSpatialIndexReindexObject( index, obj, hashid )	(index)->klass->reindexObject( index, obj, hashid )
#endif


''/ Perform a rectangle query against the spatial index, calling @c func for each potential match.
#ifndef cpSpatialIndexQuery
#define cpSpatialIndexQuery( index, obj, bb, func, _data )	(index)->klass->query( index, obj, bb, func, _data )
#endif


''/ Perform a segment query against the spatial index, calling @c func for each potential match.
#ifndef cpSpatialIndexSegmentQuery
#define cpSpatialIndexSegmentQuery( index, obj, a, b, t_exit, func, _data )	(index)->klass->segmentQuery( index, obj, a, b, t_exit, func, _data )
#endif


''/ Simultaneously reindex and find all colliding objects.

''/ @c func will be called once for each potentially overlapping pair of objects found.

''/ If the spatial index was initialized with a static index, it will collide it's objects against that as well.
#ifndef cpSpatialIndexReindexQuery
#define cpSpatialIndexReindexQuery( index, func, _data )	(index)->klass->reindexQuery( index, func, _data )
#endif


''/@}

end extern

#endif