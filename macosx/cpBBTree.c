#include "stdlib.h"
#include "stdio.h"

#include "chipmunk.h"

static cpSpatialIndexClass klass;

#pragma mark Node Functions

typedef struct cpBBTreeNode {
	void *obj;
	cpBB bb;
	struct cpBBTreeNode *a, *b, *parent;
} cpBBTreeNode;

static inline void
recycleNode(cpBBTree *tree, cpBBTreeNode *node)
{
	node->parent = tree->pooledNodes;
	tree->pooledNodes = node;
}

static cpBBTreeNode *
getFreeNode(cpBBTree *tree)
{
	cpBBTreeNode *node = tree->pooledNodes;
	
	if(node){
		tree->pooledNodes = node->parent;
		return node;
	} else {
		// Pool is exhausted, make more
		int count = CP_BUFFER_BYTES/sizeof(cpBBTreeNode);
		cpAssert(count, "Buffer size is too small.");
		
		cpBBTreeNode *buffer = (cpBBTreeNode *)cpmalloc(CP_BUFFER_BYTES);
		cpArrayPush(tree->allocatedBuffers, buffer);
		
		// push all but the first one, return the first instead
		for(int i=1; i<count; i++) recycleNode(tree, buffer + i);
		return buffer;
	}
}

static inline cpBB
cpBBinflate(cpBB bb)
{
	cpFloat coef = 0.1f;
	cpFloat x = (bb.r - bb.l)*coef;
	cpFloat y = (bb.t - bb.b)*coef;
	
	return cpBBNew(bb.l - x, bb.b - y, bb.r + x, bb.t + y);
}

static inline cpBB
cpBBinflateV(cpBB bb, cpVect v)
{
	cpFloat coef = 0.1f;
	cpFloat x = (bb.r - bb.l)*coef;
	cpFloat y = (bb.t - bb.b)*coef;
	
	v = cpvmult(v, 0.1f);
	
	return cpBBNew(bb.l + cpfmin(-x, v.x), bb.b + cpfmin(-y, v.y), bb.r + cpfmax(x, v.x), bb.t + cpfmax(y, v.y));
}

static cpBBTreeNode *
cpBBTreeNodeNewLeaf(cpBBTree *tree, void *obj, cpBB bb)
{
	cpBBTreeNode *node = getFreeNode(tree);
	node->obj = obj;
	node->bb = bb;//cpBBinflate(bb);
	
	node->a = node->b = node->parent = NULL;
	
	return node;
}

static inline void
cpBBTreeNodeSetA(cpBBTreeNode *node, cpBBTreeNode *value)
{
	node->a = value;
	value->parent = node;
}

static inline void
cpBBTreeNodeSetB(cpBBTreeNode *node, cpBBTreeNode *value)
{
	node->b = value;
	value->parent = node;
}

static cpBBTreeNode *
cpBBTreeNodeInit(cpBBTreeNode *node, cpBBTreeNode *a, cpBBTreeNode *b)
{
	node->obj = NULL;
	node->bb = cpBBmerge(a->bb, b->bb);
	node->parent = NULL;
	
	cpBBTreeNodeSetA(node, a);
	cpBBTreeNodeSetB(node, b);
	
	return node;
}

static inline cpBool
cpBBTreeNodeIsLeaf(cpBBTreeNode *node)
{
	return (node->obj != NULL);
}

static cpFloat inline
cpBBMergedArea(cpBB a, cpBB b)
{
	return (cpfmax(a.r, b.r) - cpfmin(a.l, b.l))*(cpfmax(a.t, b.t) - cpfmin(a.b, b.b));
}

static cpFloat inline
cpBBArea(cpBB bb)
{
	return (bb.r - bb.l)*(bb.t - bb.b);
}

static cpBBTreeNode *
subtreeInsert(cpBBTreeNode *subtree, cpBBTreeNode *leaf, cpBBTree *tree)
{
	if(subtree == NULL){
		return leaf;
	} else if(cpBBTreeNodeIsLeaf(subtree)){
		return cpBBTreeNodeInit(getFreeNode(tree), leaf, subtree);
	} else {
		cpFloat area_a = cpBBArea(subtree->a->bb) + cpBBMergedArea(subtree->b->bb, leaf->bb);
		cpFloat area_b = cpBBArea(subtree->b->bb) + cpBBMergedArea(subtree->a->bb, leaf->bb);
		
		if(area_a < area_b){
			cpBBTreeNodeSetB(subtree, subtreeInsert(subtree->b, leaf, tree));
		} else {
			cpBBTreeNodeSetA(subtree, subtreeInsert(subtree->a, leaf, tree));
		}
		
		subtree->bb = cpBBmerge(subtree->bb, leaf->bb);
		return subtree;
	}
}

static void
cpBBTreeNodeQuery(cpBBTreeNode *node, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	if(cpBBintersects(bb, node->bb)){
		if(cpBBTreeNodeIsLeaf(node)){
			func(obj, node->obj, data);
		} else {
			cpBBTreeNodeQuery(node->a, obj, bb, func, data);
			cpBBTreeNodeQuery(node->b, obj, bb, func, data);
		}
	}
}

static inline cpBBTreeNode *
cpBBTreeNodeOther(cpBBTreeNode *node, cpBBTreeNode *child)
{
	return (node->a == child ? node->b : node->a);
}

//static inline void
//subtreeBBPropagate(cpBBTreeNode *subtree, cpBBTreeNode *node)
//{
//	cpBBTreeNode *cond = subtree->parent;
//	for(; node != cond; node = node->parent){
//		node->bb = cpBBmerge(node->a->bb, node->b->bb);
//	}
//	
////	while(TRUE){
////		node->bb = cpBBmerge(node->a->bb, node->b->bb);
////		if(node == subtree) break;
////		
////		node = node->parent;
////	}
//}

static inline void
bbPropagate(cpBBTreeNode *node)
{
	for(; node; node = node->parent){
		node->bb = cpBBmerge(node->a->bb, node->b->bb);
	}
}

static inline void
nodeReplaceChild(cpBBTreeNode *parent, cpBBTreeNode *child, cpBBTreeNode *value)
{
	cpAssert(!cpBBTreeNodeIsLeaf(parent), "Cannot replace child of a leaf.");
	cpAssert(child == parent->a || child == parent->b, "Node is not a child of parent.");
	
	if(parent->a == child){
		cpBBTreeNodeSetA(parent, value);
	} else {
		cpBBTreeNodeSetB(parent, value);
	}
	
//	subtreeBBPropagate(subtree, parent);
	bbPropagate(parent);
}

//static int imax(int a, int b){return (a > b ? a : b);}
//
//static int
//cpBBTreeNodeDepth(cpBBTreeNode *node)
//{
//	return (cpBBTreeNodeIsLeaf(node) ? 1 : 1 + imax(cpBBTreeNodeDepth(node->a), cpBBTreeNodeDepth(node->b)));
//}

#pragma mark Memory Management Functions

cpBBTree *
cpBBTreeAlloc(void)
{
	return (cpBBTree *)cpcalloc(1, sizeof(cpBBTree));
}

static int leafSetEql(void *obj, cpBBTreeNode *node){return (obj == node->obj);}

static void *
leafSetTrans(void *obj, cpBBTree *tree)
{
	return cpBBTreeNodeNewLeaf(tree, obj, tree->bbfunc(obj));
}



cpBBTree *
cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc)
{
	tree->spatialIndex.klass = &klass;
	
	tree->bbfunc = bbfunc;
	tree->leaves = cpHashSetNew(0, (cpHashSetEqlFunc)leafSetEql, (cpHashSetTransFunc)leafSetTrans);
	tree->root = NULL;
	
	tree->pooledNodes = NULL;
	tree->allocatedBuffers = cpArrayNew(0);
	
	return tree;
}

cpBBTree *
cpBBTreeNew(cpSpatialIndexBBFunc bbfunc)
{
	return cpBBTreeInit(cpBBTreeAlloc(), bbfunc);
}

static void
freeSubTree(cpBBTree *tree, cpBBTreeNode *node)
{
	if(!cpBBTreeNodeIsLeaf(node)){
		freeSubTree(tree, node->a);
		freeSubTree(tree, node->b);
		recycleNode(tree, node);
	}
}

static void freeWrap(void *ptr, void *unused){cpfree(ptr);}

static void
cpBBTreeDestroy(cpBBTree *tree)
{
	cpHashSetFree(tree->leaves);
	cpArrayEach(tree->allocatedBuffers, freeWrap, NULL);
}

#pragma mark Insert/Remove

static inline void
insertLeaf(cpBBTreeNode *leaf, cpBBTree *tree)
{
	cpBBTreeNode *root = tree->root;
	tree->root = (root ? subtreeInsert(root, leaf, tree) : leaf);
}

static void
cpBBTreeInsert(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	cpBBTreeNode *leaf = cpHashSetInsert(tree->leaves, hashid, obj, tree);
	insertLeaf(leaf, tree);
}

static inline cpBBTreeNode *
subtreeRemove(cpBBTreeNode *subtree, cpBBTreeNode *leaf, cpBBTree *tree)
{
	if(leaf == subtree){
		return NULL;
	} else {
		cpBBTreeNode *parent = leaf->parent;
		if(parent == subtree){
			cpBBTreeNode *other = cpBBTreeNodeOther(subtree, leaf);
			other->parent = subtree->parent;
			return other;
		} else {
			nodeReplaceChild(parent->parent, parent, cpBBTreeNodeOther(parent, leaf));
			return subtree;
		}
	}
}

static void
cpBBTreeRemove(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	cpBBTreeNode *leaf = cpHashSetRemove(tree->leaves, hashid, obj);
	
	tree->root = subtreeRemove(tree->root, leaf, tree);
	recycleNode(tree, leaf);
}

static cpBool
cpBBTreeContains(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	return (cpHashSetFind(tree->leaves, hashid, obj) != NULL);
}

#pragma mark Reindex

static void
updateLeaf(cpBBTreeNode *leaf, cpBBTree *tree)
{
	cpBBTreeNode *root = tree->root;
	cpBB bb = tree->bbfunc(leaf->obj);
	
	if(!cpBBcontainsBB(leaf->bb, bb)){
		leaf->bb = cpBBinflateV(bb, ((cpShape *)(leaf->obj))->body->v);
		
		root = subtreeRemove(root, leaf, tree);
		tree->root = subtreeInsert(root, leaf, tree);
	}
}

static void
cpBBTreeReindex(cpBBTree *tree)
{
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)updateLeaf, tree);
	
//	cpAssert(cpFalse, "FAIL");
//	if(tree->root) freeSubTree(tree, tree->root);
//	tree->root = NULL;
//	
//	// TODO must fix this to assign root and update BBs!
//	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)insertLeaf, tree);
	
//	printf("tree depth %d\n", cpBBTreeDepth(tree));
}

static int
cpBBTreeReindexObject(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	cpAssert(cpFalse, "TODO Not implemented");
	return cpTrue;
}

#pragma mark Query

static void
cpBBTreePointQuery(cpBBTree *tree, cpVect point, cpSpatialIndexQueryCallback func, void *data)
{
	if(tree->root) cpBBTreeNodeQuery(tree->root, &point, cpBBNew(point.x, point.y, point.x, point.y), func, data);
}

static void
cpBBTreeSegmentQuery(cpBBTree *tree, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryCallback func, void *data)
{
	cpAssert(cpFalse, "TODO Not implemented");
}

static void
cpBBTreeQuery(cpBBTree *tree, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	if(tree->root) cpBBTreeNodeQuery(tree->root, obj, bb, func, data);
}

//static int
//cpBBTreeDepth(cpBBTree *tree)
//{
//	return (tree->root ? cpBBTreeNodeDepth(tree->root) : 0);
//}

//typedef struct queryInsertContext {
//	cpBBTree *tree;
//	cpSpatialIndexQueryCallback func;
//	void *data;
//} queryInsertContext;

//static void
//selfCollideSubtrees(cpBBTreeNode *a, cpBBTreeNode *b, cpSpatialIndexQueryCallback func, void *data)
//{
//	cpBool a_leaf = cpBBTreeNodeIsLeaf(a), b_leaf = cpBBTreeNodeIsLeaf(b);
//	
//	if(a_leaf && b_leaf){
//		func(a, b, data);
//	} else if(a_leaf){
//		cpBBTreeNodeQuery(b, a->obj, a->bb, func, data);
//	} else if(b_leaf){
//		cpBBTreeNodeQuery(a, b->obj, b->bb, func, data);
//	} else {
//		selfCollideSubtrees(a->a, a->b, func, data);
//		selfCollideSubtrees(b->a, b->b, func, data);
//		
//		if(cpBBintersects(a->bb, b->bb)) {
//			selfCollideSubtrees(b->a, a, func, data);
//			selfCollideSubtrees(b->b, a, func, data);
//		}
//	}
//}

typedef struct queryContext {
	cpSpatialIndexQueryCallback func;
	void *data;
} queryContext;

static void
queryHelper(cpBBTreeNode *leaf, queryContext *context)
{
	cpBB bb = leaf->bb;
	void *obj = leaf->obj;
	
	for(cpBBTreeNode *node=leaf; node->parent; node=node->parent){
		cpBBTreeNode *parent = node->parent;
		if(node == parent->a) cpBBTreeNodeQuery(parent->b, obj, bb, context->func, context->data);
	}
}

static void
cpBBTreeReindexQuery(cpBBTree *tree, cpSpatialIndexQueryCallback func, void *data)
{
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)updateLeaf, tree);
	
	queryContext context = {func, data};
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)queryHelper, &context);
	
//	cpBBTreeNode *root = tree->root;
//	if(root && !cpBBTreeNodeIsLeaf(root)){
//		selfCollideSubtrees(tree->root->a, tree->root->b, func, data);
//	}
	
//	int height = cpBBTreeDepth(tree);
//	int count = tree->leaves->entries;
//	printf("tree depth % 5d for % 5d objects optimal % 5.2f\n", height, count, height/(log(count) + 1.0f));
}

#pragma mark Misc

static int
cpBBTreeCount(cpBBTree *tree)
{
	return tree->leaves->entries;
}

typedef struct eachContext {
	cpSpatialIndexIterator func;
	void *data;
} eachContext;

static void eachHelper(cpBBTreeNode *node, eachContext *context){context->func(node->obj, context->data);}

static void
cpBBTreeEach(cpBBTree *tree, cpSpatialIndexIterator func, void *data)
{
	eachContext context = {func, data};
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)eachHelper, &context);
}

static cpSpatialIndexClass klass = {
	(cpSpatialIndexDestroyFunc)cpBBTreeDestroy,
	
	(cpSpatialIndexCountFunc)cpBBTreeCount,
	(cpSpatialIndexEachFunc)cpBBTreeEach,
	
	(cpSpatialIndexContainsFunc)cpBBTreeContains,
	(cpSpatialIndexInsertFunc)cpBBTreeInsert,
	(cpSpatialIndexRemoveFunc)cpBBTreeRemove,
	
	(cpSpatialIndexReindexFunc)cpBBTreeReindex,
	(cpSpatialIndexReindexObjectFunc)cpBBTreeReindexObject,
	
	(cpSpatialIndexPointQueryFunc)cpBBTreePointQuery,
	(cpSpatialIndexSegmentQueryFunc)cpBBTreeSegmentQuery,
	(cpSpatialIndexQueryFunc)cpBBTreeQuery,
	(cpSpatialIndexReindexQueryFunc)cpBBTreeReindexQuery,
};

static int
cpfcompare(const cpFloat *a, const cpFloat *b){
	if(*a < *b){
		return -1;
	} else if(*b < *a){
		return 1;
	} else {
		return 0;
	}
}

static void
fillNodeArray(cpBBTreeNode *node, cpBBTreeNode ***cursor){
	(**cursor) = node;
	(*cursor)++;
}

static cpBBTreeNode *
partitionNodes(cpBBTree *tree, cpBBTreeNode **nodes, int count)
{
	if(count == 1){
		return nodes[0];
	} else if(count == 2) {
		return cpBBTreeNodeInit(getFreeNode(tree), nodes[0], nodes[1]);
	}
	
	
	// Find the AABB for these nodes
	cpBB bb = nodes[0]->bb;
	for(int i=1; i<count; i++) bb = cpBBmerge(bb, nodes[i]->bb);
	
	// Split it on it's longest axis
	cpBool splitWidth = (bb.r - bb.l > bb.t - bb.b);
	
	// Sort the bounds and use the median as the splitting point
	cpFloat bounds[count*2];
	if(splitWidth){
		for(int i=0; i<count; i++){
			bounds[2*i + 0] = nodes[i]->bb.l;
			bounds[2*i + 1] = nodes[i]->bb.r;
		}
	} else {
		for(int i=0; i<count; i++){
			bounds[2*i + 0] = nodes[i]->bb.b;
			bounds[2*i + 1] = nodes[i]->bb.t;
		}
	}
	
	qsort(bounds, count*2, sizeof(cpFloat), (int (*)(const void *, const void *))cpfcompare);
	cpFloat split = (bounds[count - 1] + bounds[count])*0.5f; // use the medain as the split
	
	// Generate the child BBs
	cpBB a = bb, b = bb;
	if(splitWidth){
		a.r = b.l = split;
	} else {
		a.t = b.b = split;
	}
	
	// Partition the nodes
	int right = count;
	for(int left=0; left < right;){
		cpBBTreeNode *node = nodes[left];
		if(cpBBMergedArea(node->bb, b) < cpBBMergedArea(node->bb, a)){
			right--;
			nodes[left] = nodes[right];
			nodes[right] = node;
		} else {
			left++;
		}
	}
	
	if(right == count){
		cpBBTreeNode *node = NULL;
		for(int i=0; i<count; i++) node = subtreeInsert(node, nodes[i], tree);
		return node;
	}
	
	// Recurse and build the node!
	return cpBBTreeNodeInit(getFreeNode(tree),
		partitionNodes(tree, nodes, right),
		partitionNodes(tree, nodes + right, count - right)
	);
}

void
cpBBTreeOptimize(cpBBTree *tree)
{
	if(tree->spatialIndex.klass != &klass){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeOptimize() call to non-tree spatial index.");
		return;
	}
	
	cpBBTreeNode *root = tree->root;
	if(!root) return;
	
	int count = cpBBTreeCount(tree);
	cpBBTreeNode *nodes[count];
	cpBBTreeNode **cursor = nodes;
	
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)fillNodeArray, &cursor);
	
	freeSubTree(tree, root);
	tree->root = partitionNodes(tree, nodes, count);
}

#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#include <GLUT/glut.h>

static void
cpBBTreeNodeRender(cpBBTreeNode *node, int depth)
{
	if(!cpBBTreeNodeIsLeaf(node) && depth <= 10){
		cpBBTreeNodeRender(node->a, depth + 1);
		cpBBTreeNodeRender(node->b, depth + 1);
	}
	
	cpBB bb = node->bb;
	
	GLfloat v = depth/2.0f;
//	glColor3f(v,v,v);
//	glRectf(bb.l, bb.b, bb.r, bb.t);
	
	glColor3f(1.0f - v, v, 0.0f);
	glBegin(GL_LINES); {
		glVertex2f(bb.l, bb.b);
		glVertex2f(bb.l, bb.t);
		
		glVertex2f(bb.l, bb.t);
		glVertex2f(bb.r, bb.t);
		
		glVertex2f(bb.r, bb.t);
		glVertex2f(bb.r, bb.b);
		
		glVertex2f(bb.r, bb.b);
		glVertex2f(bb.l, bb.b);
	}; glEnd();
}

void
cpBBTreeRenderDebug(cpBBTree *tree){
	if(tree->spatialIndex.klass != &klass){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeRenderDebug() call to non-tree spatial index.");
		return;
	}
	
	if(tree->root) cpBBTreeNodeRender(tree->root, 0);
}
