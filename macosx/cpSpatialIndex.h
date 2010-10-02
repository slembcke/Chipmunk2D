// TODO define types
//typedef enum cpSpatialIndexType {
//	
//} cpSpatialIndexType;

struct cpSpatialIndexClass;

typedef struct cpSpatialIndex {
	struct cpSpatialIndexClass *klass;
} cpSpatialIndex;

typedef void (*cpSpatialIndexIterator)(void *obj, void *data);
typedef void (*cpSpatialIndexQueryCallback)(void *obj1, void *obj2, void *data);

typedef void (*cpSpatialIndexDestroyFunc)(cpSpatialIndex *index);

typedef int (*cpSpatialIndexCountFunc)(cpSpatialIndex *index);
typedef void (*cpSpatialIndexEachFunc)(cpSpatialIndex *index, cpSpatialIndexIterator func, void *data);

typedef cpBool (*cpSpatialIndexContainsFunc)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexInsertFunc)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexRemoveFunc)(cpSpatialIndex *index, void *obj, cpHashValue hashid);

typedef	void (*cpSpatialIndexPointQueryFunc)(cpSpatialIndex *index, cpVect point, cpSpatialIndexQueryCallback func, void *data);
typedef void (*cpSpatialIndexQueryFunc)(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data);
typedef void (*cpSpatialIndexReindexQueryFunc)(cpSpatialIndex *index, cpSpatialIndexQueryCallback func, void *data);

typedef struct cpSpatialIndexClass {
	cpSpatialIndexDestroyFunc destroy;
	
	cpSpatialIndexCountFunc count;
	cpSpatialIndexEachFunc each;
	
	cpSpatialIndexContainsFunc contains;
	cpSpatialIndexInsertFunc insert;
	cpSpatialIndexRemoveFunc remove;
	
	cpSpatialIndexPointQueryFunc pointQuery;
	cpSpatialIndexQueryFunc query;
	cpSpatialIndexReindexQueryFunc reindexQuery;
} cpSpatialIndexClass;


void cpSpatialIndexFree(cpSpatialIndex *index);

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

static inline	void
cpSpatialIndexPointQuery(cpSpatialIndex *index, cpVect point, cpSpatialIndexQueryCallback func, void *data)
{
	index->klass->pointQuery(index, point, func, data);
}

static inline void
cpSpatialIndexQuery(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	index->klass->query(index, obj, bb, func, data);
}

static inline void
cpSpatialIndexReindexQuery(cpSpatialIndex *index, cpSpatialIndexQueryCallback func, void *data)
{
	index->klass->reindexQuery(index, func, data);
}
