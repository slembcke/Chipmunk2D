#pragma mark Spatial Index

typedef cpBB (*cpSpatialIndexBBFunc)(void *obj);
typedef void (*cpSpatialIndexIteratorFunc)(void *obj, void *data);
typedef void (*cpSpatialIndexQueryFunc)(void *obj1, void *obj2, void *data);
typedef cpFloat (*cpSpatialIndexSegmentQueryFunc)(void *obj1, void *obj2, void *data);


typedef struct cpSpatialIndexClass cpSpatialIndexClass;
typedef struct cpSpatialIndex cpSpatialIndex;

struct cpSpatialIndex {
	cpSpatialIndexClass *klass;
	
	cpSpatialIndexBBFunc bbfunc;
	
	cpSpatialIndex *staticIndex, *dynamicIndex;
};


#pragma mark Spatial Hash

typedef struct cpSpaceHash cpSpaceHash;

cpSpaceHash *cpSpaceHashAlloc(void);
cpSpatialIndex *cpSpaceHashInit(cpSpaceHash *hash, cpFloat celldim, int numcells, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);
cpSpatialIndex *cpSpaceHashNew(cpFloat celldim, int cells, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

void cpSpaceHashResize(cpSpaceHash *hash, cpFloat celldim, int numcells);

#pragma mark AABB Tree

typedef struct cpBBTree cpBBTree;

cpBBTree *cpBBTreeAlloc(void);
cpSpatialIndex *cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);
cpSpatialIndex *cpBBTreeNew(cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

void cpBBTreeOptimize(cpSpatialIndex *index);

typedef cpVect (*cpBBTreeVelocityFunc)(void *obj);
void cpBBTreeSetVelocityFunc(cpSpatialIndex *index, cpBBTreeVelocityFunc func);

#pragma mark Spatial Index Implementation

typedef void (*cpSpatialIndexDestroyImpl)(cpSpatialIndex *index);

typedef int (*cpSpatialIndexCountImpl)(cpSpatialIndex *index);
typedef void (*cpSpatialIndexEachImpl)(cpSpatialIndex *index, cpSpatialIndexIteratorFunc func, void *data);

typedef cpBool (*cpSpatialIndexContainsImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexInsertImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexRemoveImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);

typedef void (*cpSpatialIndexReindexImpl)(cpSpatialIndex *index);
typedef void (*cpSpatialIndexReindexObjectImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexReindexQueryImpl)(cpSpatialIndex *index, cpSpatialIndexQueryFunc func, void *data);

typedef void (*cpSpatialIndexPointQueryImpl)(cpSpatialIndex *index, cpVect point, cpSpatialIndexQueryFunc func, void *data);
typedef void (*cpSpatialIndexSegmentQueryImpl)(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data);
typedef void (*cpSpatialIndexQueryImpl)(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data);

struct cpSpatialIndexClass {
	cpSpatialIndexDestroyImpl destroy;
	
	cpSpatialIndexCountImpl count;
	cpSpatialIndexEachImpl each;
	
	cpSpatialIndexContainsImpl contains;
	cpSpatialIndexInsertImpl insert;
	cpSpatialIndexRemoveImpl remove;
	
	cpSpatialIndexReindexImpl reindex;
	cpSpatialIndexReindexObjectImpl reindexObject;
	cpSpatialIndexReindexQueryImpl reindexQuery;
	
	cpSpatialIndexPointQueryImpl pointQuery;
	cpSpatialIndexSegmentQueryImpl segmentQuery;
	cpSpatialIndexQueryImpl query;
};


void cpSpatialIndexFree(cpSpatialIndex *index);
void cpSpatialIndexCollideStatic(cpSpatialIndex *dynamicIndex, cpSpatialIndex *staticIndex, cpSpatialIndexQueryFunc func, void *data);

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
cpSpatialIndexEach(cpSpatialIndex *index, cpSpatialIndexIteratorFunc func, void *data)
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
cpSpatialIndexPointQuery(cpSpatialIndex *index, cpVect point, cpSpatialIndexQueryFunc func, void *data)
{
	index->klass->pointQuery(index, point, func, data);
}

// TODO make sure to doc the callback type
static inline void
cpSpatialIndexSegmentQuery(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	index->klass->segmentQuery(index, obj, a, b, t_exit, func, data);
}


static inline void
cpSpatialIndexQuery(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data)
{
	index->klass->query(index, obj, bb, func, data);
}

static inline void
cpSpatialIndexReindexQuery(cpSpatialIndex *index, cpSpatialIndexQueryFunc func, void *data)
{
	index->klass->reindexQuery(index, func, data);
}
