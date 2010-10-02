#include "stdlib.h"
#include "stdio.h"

#include "chipmunk.h"

#pragma mark Node Functions

static inline cpBool
cpBBTreeNodeIsLeaf(cpBBTreeNode *node)
{
	return (node->obj != NULL);
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
	
	node->a = node->b = NULL;
	
	return node;
}

static cpBBTreeNode *
cpBBTreeNodeInit(cpBBTreeNode *node, cpBBTreeNode *a, cpBBTreeNode *b)
{
	node->obj = NULL;
	node->bb = cpBBmerge(a->bb, b->bb);
	
	node->a = a;
	node->b = b;
	
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

static void
cpBBTreeNodeInsert(cpBBTreeNode *node, cpBBTreeNode *insert)
{
	if(cpBBTreeNodeIsLeaf(node)){
		cpBBTreeNodeInit(node, cpBBTreeNodeNewLeaf(node->obj, node->bb), insert);
	} else {
		cpFloat area_a = cpBBArea(node->a->bb) + cpBBMergedArea(node->b->bb, insert->bb);
		cpFloat area_b = cpBBArea(node->b->bb) + cpBBMergedArea(node->a->bb, insert->bb);
		cpFloat area_i = cpBBArea(insert->bb) + cpBBArea(node->bb);
		
		if(area_a < area_b && area_a < area_i){
			cpBBTreeNodeInsert(node->b, insert);
		} else if(area_b < area_i){
			cpBBTreeNodeInsert(node->a, insert);
		} else {
			cpBBTreeNodeInsert(node->a, node->b);
			node->b = insert;
		}
	}
	
	node->bb = cpBBmerge(node->bb, insert->bb);
}

static int imax(int a, int b){return (a > b ? a : b);}

static int
cpBBTreeNodeDepth(cpBBTreeNode *node)
{
	return (cpBBTreeNodeIsLeaf(node) ? 1 : 1 + imax(cpBBTreeNodeDepth(node->a), cpBBTreeNodeDepth(node->a)));
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
cpBBTreeDestroy(cpBBTree *tree)
{
	// do nothing so far
}

#pragma mark Insert/Remove

static void
cpBBTreeInsert(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	cpBBTreeNode *node = cpBBTreeNodeNewLeaf(obj, tree->bbfunc(obj));
	
	if(tree->root == NULL){
		tree->root = node;
	} else {
		cpBBTreeNodeInsert(tree->root, node);
	}
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
cpBBTreeReindexFunc(cpBBTree *tree)
{
	cpAssertWarn(cpFalse, "TODO Not implemented");
}

static int
cpBBTreeReindexObjectFunc(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	cpAssert(cpFalse, "TODO Not implemented");
	return cpTrue;
}

#pragma mark Query

static void
cpBBTreePointQueryFunc(cpBBTree *tree, cpVect point, cpSpatialIndexQueryCallback func, void *data)
{
	cpAssert(cpFalse, "TODO Not implemented");
}

static void
cpBBTreeSegmentQueryFunc(cpBBTree *tree, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryCallback func, void *data)
{
	cpAssert(cpFalse, "TODO Not implemented");
}

static void
cpBBTreeQueryFunc(cpBBTree *tree, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	cpAssert(cpFalse, "TODO Not implemented");
}

static void
cpBBTreeReindexQueryFunc(cpBBTree *tree, cpSpatialIndexQueryCallback func, void *data)
{
	cpAssert(cpFalse, "TODO Not implemented");
}

#pragma mark Misc

static int
cpBBTreeDepth(cpBBTree *tree)
{
	return (tree->root ? cpBBTreeNodeDepth(tree->root) : 0);
}

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
	
	(cpSpatialIndexReindexFunc)cpBBTreeReindexFunc,
	(cpSpatialIndexReindexObjectFunc)cpBBTreeReindexObjectFunc,
	
	(cpSpatialIndexPointQueryFunc)cpBBTreePointQueryFunc,
	(cpSpatialIndexSegmentQueryFunc)cpBBTreeSegmentQueryFunc,
	(cpSpatialIndexQueryFunc)cpBBTreeQueryFunc,
	(cpSpatialIndexReindexQueryFunc)cpBBTreeReindexQueryFunc,
};
