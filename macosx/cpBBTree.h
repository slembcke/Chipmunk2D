struct cpBBTreeNode;
struct pairLink;

typedef struct cpBBTree {
	cpSpatialIndex spatialIndex;
	
	// TODO move to spatial index
	cpSpatialIndexBBFunc bbfunc;
	
	cpHashSet *leaves;
	struct cpBBTreeNode *root;
	
	struct cpBBTreeNode *pooledNodes;
	struct pairLink *pooledLinks;
	cpArray *allocatedBuffers;
	
	cpTimestamp stamp;
	
//	unsigned int opath;
} cpBBTree;

cpBBTree *cpBBTreeAlloc(void);
cpBBTree *cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc);
cpBBTree *cpBBTreeNew(cpSpatialIndexBBFunc bbfunc);

void cpBBTreeRenderDebug(cpBBTree *tree);
