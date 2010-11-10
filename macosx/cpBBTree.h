struct cpBBTreeNode;
struct pairLink;

typedef cpVect (*cpBBTreeVelocityFunc)(void *obj);

typedef struct cpBBTree {
	cpSpatialIndex spatialIndex;
	cpBBTreeVelocityFunc velocityFunc;
	
	cpHashSet *leaves;
	struct cpBBTreeNode *root;
	
	struct cpBBTreeNode *pooledNodes;
	struct pairLink *pooledLinks;
	cpArray *allocatedBuffers;
	
	cpTimestamp stamp;
} cpBBTree;

cpBBTree *cpBBTreeAlloc(void);
cpBBTree *cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc);
cpBBTree *cpBBTreeNew(cpSpatialIndexBBFunc bbfunc);

void cpBBTreeOptimize(cpSpatialIndex *index);
void cpBBTreeSetVelocityFunc(cpSpatialIndex *index, cpBBTreeVelocityFunc func);
