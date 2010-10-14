typedef struct cpBBTreeNode {
	void *obj;
	cpBB bb;
	struct cpBBTreeNode *a, *b, *parent;
} cpBBTreeNode;

typedef struct cpBBTree {
	cpSpatialIndex spatialIndex;
	
	// TODO move to spatial index
	cpSpatialIndexBBFunc bbfunc;
	
	cpArray *objs;
	cpBBTreeNode *root;
} cpBBTree;

cpBBTree *cpBBTreeAlloc(void);
cpBBTree *cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc);
cpBBTree *cpBBTreeNew(cpSpatialIndexBBFunc bbfunc);
