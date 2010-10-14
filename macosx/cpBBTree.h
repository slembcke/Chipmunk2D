struct cpBBTreeNode;

typedef struct cpBBTree {
	cpSpatialIndex spatialIndex;
	
	// TODO move to spatial index
	cpSpatialIndexBBFunc bbfunc;
	
	cpHashSet *leaves;
	struct cpBBTreeNode *root;
} cpBBTree;

cpBBTree *cpBBTreeAlloc(void);
cpBBTree *cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc);
cpBBTree *cpBBTreeNew(cpSpatialIndexBBFunc bbfunc);
