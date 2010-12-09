#pragma mark Spatial Index

typedef cpBB (*cpSpatialIndexBBFunc)(void *obj);
typedef void (*cpSpatialIndexIterator)(void *obj, void *data);
typedef void (*cpSpatialIndexQueryCallback)(void *obj1, void *obj2, void *data);
typedef cpFloat (*cpSpatialIndexSegmentQueryCallback)(void *obj1, void *obj2, void *data);


struct cpSpatialIndexClass;

typedef struct cpSpatialIndex {
	struct cpSpatialIndexClass *klass;
	
	cpSpatialIndexBBFunc bbfunc;
} cpSpatialIndex;


#pragma mark Spatial Hash

typedef struct cpSpaceHash cpSpaceHash;

cpSpaceHash *cpSpaceHashAlloc(void);
cpSpaceHash *cpSpaceHashInit(cpSpaceHash *hash, cpFloat celldim, int numcells, cpSpatialIndexBBFunc bbfunc);
cpSpaceHash *cpSpaceHashNew(cpFloat celldim, int cells, cpSpatialIndexBBFunc bbfunc);

void cpSpaceHashResize(cpSpaceHash *hash, cpFloat celldim, int numcells);

#pragma mark AABB Tree

typedef struct cpBBTree cpBBTree;

cpBBTree *cpBBTreeAlloc(void);
cpBBTree *cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc);
cpBBTree *cpBBTreeNew(cpSpatialIndexBBFunc bbfunc);

void cpBBTreeOptimize(cpSpatialIndex *index);

typedef cpVect (*cpBBTreeVelocityFunc)(void *obj);
void cpBBTreeSetVelocityFunc(cpSpatialIndex *index, cpBBTreeVelocityFunc func);

#pragma mark Spatial Index Implementation

typedef void (*cpSpatialIndexDestroyFunc)(cpSpatialIndex *index);

typedef int (*cpSpatialIndexCountFunc)(cpSpatialIndex *index);
typedef void (*cpSpatialIndexEachFunc)(cpSpatialIndex *index, cpSpatialIndexIterator func, void *data);

typedef cpBool (*cpSpatialIndexContainsFunc)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexInsertFunc)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexRemoveFunc)(cpSpatialIndex *index, void *obj, cpHashValue hashid);

typedef void (*cpSpatialIndexReindexFunc)(cpSpatialIndex *index);
typedef void (*cpSpatialIndexReindexObjectFunc)(cpSpatialIndex *index, void *obj, cpHashValue hashid);

typedef void (*cpSpatialIndexPointQueryFunc)(cpSpatialIndex *index, cpVect point, cpSpatialIndexQueryCallback func, void *data);
typedef void (*cpSpatialIndexSegmentQueryFunc)(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryCallback func, void *data);
typedef void (*cpSpatialIndexQueryFunc)(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data);
typedef void (*cpSpatialIndexReindexCollideFunc)(cpSpatialIndex *dynamicIndex, cpSpatialIndex *staticIndex, cpSpatialIndexQueryCallback func, void *data);

typedef struct cpSpatialIndexClass {
	cpSpatialIndexDestroyFunc destroy;
	
	cpSpatialIndexCountFunc count;
	cpSpatialIndexEachFunc each;
	
	cpSpatialIndexContainsFunc contains;
	cpSpatialIndexInsertFunc insert;
	cpSpatialIndexRemoveFunc remove;
	
	cpSpatialIndexReindexFunc reindex;
	cpSpatialIndexReindexObjectFunc reindexObject;
	
	cpSpatialIndexPointQueryFunc pointQuery;
	cpSpatialIndexSegmentQueryFunc segmentQuery;
	cpSpatialIndexQueryFunc query;
	cpSpatialIndexReindexCollideFunc reindexCollide;
} cpSpatialIndexClass;


void cpSpatialIndexFree(cpSpatialIndex *index);
void cpSpatialIndexCollideStatic(cpSpatialIndex *dynamicIndex, cpSpatialIndex *staticIndex, cpSpatialIndexQueryCallback func, void *data);

static inline void
cpSpatialIndexDestroy(cpSpatialIndex *index)
{
	index->klass->destroy(index);
}

static inline int
cpSpatialIndexCount(cpSpatialIndex *index)
{
	return index->klass->count(index);
}

static inline void
cpSpatialIndexEach(cpSpatialIndex *index, cpSpatialIndexIterator func, void *data)
{
	index->klass->each(index, func, data);
}

static inline cpBool
cpSpatialIndexContains(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	return index->klass->contains(index, obj, hashid);
}

static inline void
cpSpatialIndexInsert(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	index->klass->insert(index, obj, hashid);
}

static inline void
cpSpatialIndexRemove(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	index->klass->remove(index, obj, hashid);
}

static inline void
cpSpatialIndexReindex(cpSpatialIndex *index)
{
	index->klass->reindex(index);
}

static inline void
cpSpatialIndexReindexObject(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	index->klass->reindexObject(index, obj, hashid);
}

// TODO make sure to doc the callback type, a pointer to the point is passed as obj1
static inline	void
cpSpatialIndexPointQuery(cpSpatialIndex *index, cpVect point, cpSpatialIndexQueryCallback func, void *data)
{
	index->klass->pointQuery(index, point, func, data);
}

// TODO make sure to doc the callback type
static inline void
cpSpatialIndexSegmentQuery(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryCallback func, void *data)
{
	index->klass->segmentQuery(index, obj, a, b, t_exit, func, data);
}


static inline void
cpSpatialIndexQuery(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	index->klass->query(index, obj, bb, func, data);
}

static inline void
cpSpatialIndexReindexCollide(cpSpatialIndex *dynamicIndex, cpSpatialIndex *staticIndex, cpSpatialIndexQueryCallback func, void *data)
{
	dynamicIndex->klass->reindexCollide(dynamicIndex, staticIndex, func, data);
}
