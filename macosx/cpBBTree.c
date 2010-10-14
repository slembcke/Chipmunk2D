#include "stdlib.h"
#include "stdio.h"

#include "chipmunk.h"

#pragma mark Node Functions

static inline cpBool
cpBBTreeNodeIsLeaf(cpBBTreeNode *node)
{
	return (node->obj != NULL);
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
cpBBTreeNodeAlloc(void)
{
	return (cpBBTreeNode *)cpcalloc(1, sizeof(cpBBTreeNode));
}

static cpBBTreeNode *
cpBBTreeNodeNewLeaf(void *obj, cpBB bb)
{
	cpBBTreeNode *node = cpBBTreeNodeAlloc();
	node->obj = obj;
	node->bb = bb;
	
	node->a = node->b = node->parent = NULL;
	
	return node;
}

static cpBBTreeNode *
cpBBTreeNodeInit(cpBBTreeNode *node, cpBBTreeNode *a, cpBBTreeNode *b)
{
	node->obj = NULL;
	node->bb = cpBBmerge(a->bb, b->bb);
	
	cpBBTreeNodeSetA(node, a);
	cpBBTreeNodeSetB(node, b);
	
	return node;
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
cpBBTreeNodeInsert(cpBBTreeNode *node, cpBBTreeNode *insert)
{
	if(cpBBTreeNodeIsLeaf(node)){
		return cpBBTreeNodeInit(cpBBTreeNodeAlloc(), insert, node);
	} else {
		cpFloat area_a = cpBBArea(node->a->bb) + cpBBMergedArea(node->b->bb, insert->bb);
		cpFloat area_b = cpBBArea(node->b->bb) + cpBBMergedArea(node->a->bb, insert->bb);
		
		if(area_a < area_b){
			cpBBTreeNodeSetB(node, cpBBTreeNodeInsert(node->b, insert));
		} else {
			cpBBTreeNodeSetA(node, cpBBTreeNodeInsert(node->a, insert));
		}
		
		node->bb = cpBBmerge(node->bb, insert->bb);
		return node;
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

static int imax(int a, int b){return (a > b ? a : b);}

static int
cpBBTreeNodeDepth(cpBBTreeNode *node)
{
	return (cpBBTreeNodeIsLeaf(node) ? 1 : 1 + imax(cpBBTreeNodeDepth(node->a), cpBBTreeNodeDepth(node->b)));
}

#pragma mark Memory Management Functions

cpBBTree *
cpBBTreeAlloc(void)
{
	return (cpBBTree *)cpcalloc(1, sizeof(cpBBTree));
}

static cpSpatialIndexClass klass;

cpBBTree *
cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc)
{
	tree->spatialIndex.klass = &klass;
	
	tree->bbfunc = bbfunc;
	tree->objs = cpArrayNew(0);
	tree->root = NULL;
	
	return tree;
}

cpBBTree *
cpBBTreeNew(cpSpatialIndexBBFunc bbfunc)
{
	return cpBBTreeInit(cpBBTreeAlloc(), bbfunc);
}

static void
freeSubTree(cpBBTreeNode *node)
{
	if(!cpBBTreeNodeIsLeaf(node)){
		freeSubTree(node->a);
		freeSubTree(node->b);
	}
	
	cpfree(node);
}

static void
cpBBTreeDestroy(cpBBTree *tree)
{
	cpArrayFree(tree->objs);
	if(tree->root) freeSubTree(tree->root);
}

#pragma mark Insert/Remove

static void
cpBBTreeInsert(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	cpArrayPush(tree->objs, obj);
	cpBBTreeNode *node = cpBBTreeNodeNewLeaf(obj, tree->bbfunc(obj));
	
	tree->root = (tree->root ? cpBBTreeNodeInsert(tree->root, node) : node);
}

static void
cpBBTreeRemove(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	cpAssert(cpFalse, "TODO Not implemented");
}

static int
cpBBTreeContains(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	return cpArrayContains(tree->objs, obj);
}

#pragma mark Reindex

static void
cpBBTreeReindex(cpBBTree *tree)
{
	if(tree->root) freeSubTree(tree->root);
	tree->root = NULL;
	
	cpArray *objs = tree->objs;
	for(int i=0; i<objs->num; i++){
		void *obj = objs->arr[i];
		
		cpBBTreeNode *node = cpBBTreeNodeNewLeaf(obj, tree->bbfunc(obj));
		tree->root = (tree->root ? cpBBTreeNodeInsert(tree->root, node) : node);
	}
	
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
	cpAssert(cpFalse, "TODO Not implemented");
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

static int
cpBBTreeDepth(cpBBTree *tree)
{
	return (tree->root ? cpBBTreeNodeDepth(tree->root) : 0);
}

static void
cpBBTreeReindexQuery(cpBBTree *tree, cpSpatialIndexQueryCallback func, void *data)
{
	if(tree->root) freeSubTree(tree->root);
	tree->root = NULL;
	
	cpArray *objs = tree->objs;
	for(int i=0; i<objs->num; i++){
		void *obj = objs->arr[i];
		
		cpBBTreeQuery(tree, obj, tree->bbfunc(obj), func, data);
		
		cpBBTreeNode *node = cpBBTreeNodeNewLeaf(obj, tree->bbfunc(obj));
		tree->root = (tree->root ? cpBBTreeNodeInsert(tree->root, node) : node);
	}
	
//	printf("tree depth % 5d for % 5d objects\n", cpBBTreeDepth(tree), tree->objs->num);
}

#pragma mark Misc

static int
cpBBTreeCount(cpBBTree *tree)
{
	return tree->objs->num;
}

static void
cpBBTreeEach(cpBBTree *tree, cpSpatialIndexIterator func, void *data)
{
	cpArray *objs = tree->objs;
	for(int i=0; i<objs->num; i++) func(objs->arr[i], data);
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
