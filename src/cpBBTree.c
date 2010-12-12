#include "stdlib.h"
#include "stdio.h"

#include "chipmunk_private.h"

static cpSpatialIndexClass klass;

typedef struct Node Node;
typedef struct Pair Pair;

struct cpBBTree {
	cpSpatialIndex spatialIndex;
	cpBBTreeVelocityFunc velocityFunc;
	
	cpHashSet *leaves;
	Node *root;
	
	Node *pooledNodes;
	Pair *pooledPairs;
	cpArray *allocatedBuffers;
	
	cpTimestamp stamp;
};

struct Node {
	void *obj;
	cpBB bb;
	Node *parent;
	
	union {
		// Internal nodes
		struct { Node *a, *b; };
		
		// Leaves
		struct {
			cpTimestamp stamp;
			Pair *pairs;
		};
	};
};

typedef struct Thread {
	Pair *prev;
	Node *leaf;
	Pair *next;
} Thread;

struct Pair { Thread a, b; };

#pragma mark Misc Functions

static inline cpFloat
cpBBMergedArea(cpBB a, cpBB b)
{
	return (cpfmax(a.r, b.r) - cpfmin(a.l, b.l))*(cpfmax(a.t, b.t) - cpfmin(a.b, b.b));
}

static inline cpFloat
cpBBArea(cpBB bb)
{
	return (bb.r - bb.l)*(bb.t - bb.b);
}

//static inline cpFloat
//cpBBProximity(cpBB a, cpBB b)
//{
//	return cpfabs(a.l + a.r - b.l - b.r) + cpfabs(a.b + b.t - b.b - b.t);
//}

static inline cpBB
get_bb(cpBBTree *tree, void *obj)
{
	cpBB bb = tree->spatialIndex.bbfunc(obj);
	
	cpBBTreeVelocityFunc velocityFunc = tree->velocityFunc;
	if(velocityFunc){
		cpFloat coef = 0.1f;
		cpFloat x = (bb.r - bb.l)*coef;
		cpFloat y = (bb.t - bb.b)*coef;
		
		cpVect v = cpvmult(velocityFunc(obj), 0.1f);
		return cpBBNew(bb.l + cpfmin(-x, v.x), bb.b + cpfmin(-y, v.y), bb.r + cpfmax(x, v.x), bb.t + cpfmax(y, v.y));
	} else {
		return bb;
	}
}

#pragma mark Pair/Thread Functions

static void
PairRecycle(cpBBTree *tree, Pair *pair)
{
	pair->a.next = tree->pooledPairs;
	tree->pooledPairs = pair;
}

static Pair *
PairFromPool(cpBBTree *tree)
{
	Pair *pair = tree->pooledPairs;
	
	if(pair){
		tree->pooledPairs = pair->a.next;
		return pair;
	} else {
		// Pool is exhausted, make more
		int count = CP_BUFFER_BYTES/sizeof(Pair);
		cpAssert(count, "Buffer size is too small.");
		
		Pair *buffer = (Pair *)cpmalloc(CP_BUFFER_BYTES);
		cpArrayPush(tree->allocatedBuffers, buffer);
		
		// push all but the first one, return the first instead
		for(int i=1; i<count; i++) PairRecycle(tree, buffer + i);
		return buffer;
	}
}

static void
ThreadUnlink(Thread *thread)
{
	Node *leaf = thread->leaf;
	Pair *next = thread->next, *prev = thread->prev;
	
	if(next){
		if(next->a.leaf == leaf) next->a.prev = prev; else next->b.prev = prev;
	}
	
	if(prev){
		if(prev->a.leaf == leaf) prev->a.next = next; else prev->b.next = next;
	} else {
		leaf->pairs = next;
	}
}

static void
PairsClear(Node *leaf, cpBBTree *tree)
{
	Pair *pair = leaf->pairs;
	leaf->pairs = NULL;
	
	while(pair){
		if(pair->a.leaf == leaf){
			Pair *next = pair->a.next;
			ThreadUnlink(&pair->b);
			PairRecycle(tree, pair);
			pair = next;
		} else {
			Pair *next = pair->b.next;
			ThreadUnlink(&pair->a);
			PairRecycle(tree, pair);
			pair = next;
		}
	}
}

static void
PairInsert(Node *a, Node *b, cpBBTree *tree)
{
	Pair *pair = PairFromPool(tree);
	Pair *nextA = a->pairs, *nextB = b->pairs;
	
	if(nextA){
		if(nextA->a.leaf == a) nextA->a.prev = pair; else nextA->b.prev = pair;
	}
	
	if(nextB){
		if(nextB->a.leaf == b) nextB->a.prev = pair; else nextB->b.prev = pair;
	}
	
	(*pair) = (Pair){{NULL, a, nextA},{NULL, b, nextB}};
	a->pairs = b->pairs = pair;
}


#pragma mark Node Functions

static void
NodeRecycle(cpBBTree *tree, Node *node)
{
	node->parent = tree->pooledNodes;
	tree->pooledNodes = node;
}

static Node *
NodeFromPool(cpBBTree *tree)
{
	Node *node = tree->pooledNodes;
	
	if(node){
		tree->pooledNodes = node->parent;
		return node;
	} else {
		// Pool is exhausted, make more
		int count = CP_BUFFER_BYTES/sizeof(Node);
		cpAssert(count, "Buffer size is too small.");
		
		Node *buffer = (Node *)cpmalloc(CP_BUFFER_BYTES);
		cpArrayPush(tree->allocatedBuffers, buffer);
		
		// push all but the first one, return the first instead
		for(int i=1; i<count; i++) NodeRecycle(tree, buffer + i);
		return buffer;
	}
}

static inline void
NodeSetA(Node *node, Node *value)
{
	node->a = value;
	value->parent = node;
}

static inline void
NodeSetB(Node *node, Node *value)
{
	node->b = value;
	value->parent = node;
}

static Node *
NodeNew(cpBBTree *tree, Node *a, Node *b)
{
	Node *node = NodeFromPool(tree);
	
	node->obj = NULL;
	node->bb = cpBBmerge(a->bb, b->bb);
	node->parent = NULL;
	
	NodeSetA(node, a);
	NodeSetB(node, b);
	
	return node;
}

static inline cpBool
NodeIsLeaf(Node *node)
{
	return (node->obj != NULL);
}

static inline Node *
NodeOther(Node *node, Node *child)
{
	return (node->a == child ? node->b : node->a);
}

static inline void
NodeReplaceChild(Node *parent, Node *child, Node *value, cpBBTree *tree)
{
	cpAssert(!NodeIsLeaf(parent), "Cannot replace child of a leaf.");
	cpAssert(child == parent->a || child == parent->b, "Node is not a child of parent.");
	
	if(parent->a == child){
		NodeRecycle(tree, parent->a);
		NodeSetA(parent, value);
	} else {
		NodeRecycle(tree, parent->b);
		NodeSetB(parent, value);
	}
	
	for(Node *node=parent; node; node = node->parent){
		node->bb = cpBBmerge(node->a->bb, node->b->bb);
	}
}

#pragma mark Subtree Functions

static Node *
SubtreeInsert(Node *subtree, Node *leaf, cpBBTree *tree)
{
	if(subtree == NULL){
		return leaf;
	} else if(NodeIsLeaf(subtree)){
		return NodeNew(tree, leaf, subtree);
	} else {
		cpFloat cost_a = cpBBArea(subtree->b->bb) + cpBBMergedArea(subtree->a->bb, leaf->bb);
		cpFloat cost_b = cpBBArea(subtree->a->bb) + cpBBMergedArea(subtree->b->bb, leaf->bb);
		
//		cpFloat cost_a = cpBBProximity(subtree->a->bb, leaf->bb);
//		cpFloat cost_b = cpBBProximity(subtree->b->bb, leaf->bb);
		
		if(cost_b < cost_a){
			NodeSetB(subtree, SubtreeInsert(subtree->b, leaf, tree));
		} else {
			NodeSetA(subtree, SubtreeInsert(subtree->a, leaf, tree));
		}
		
		subtree->bb = cpBBmerge(subtree->bb, leaf->bb);
		return subtree;
	}
}

static void
SubtreeQuery(Node *node, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	if(cpBBintersects(bb, node->bb)){
		if(NodeIsLeaf(node)){
			func(obj, node->obj, data);
		} else {
			SubtreeQuery(node->a, obj, bb, func, data);
			SubtreeQuery(node->b, obj, bb, func, data);
		}
	}
}

static void
SubtreeRecycle(cpBBTree *tree, Node *node)
{
	if(!NodeIsLeaf(node)){
		SubtreeRecycle(tree, node->a);
		SubtreeRecycle(tree, node->b);
		NodeRecycle(tree, node);
	}
}

static inline Node *
SubtreeRemove(Node *subtree, Node *leaf, cpBBTree *tree)
{
	if(leaf == subtree){
		return NULL;
	} else {
		Node *parent = leaf->parent;
		if(parent == subtree){
			Node *other = NodeOther(subtree, leaf);
			other->parent = subtree->parent;
			NodeRecycle(tree, subtree);
			return other;
		} else {
			NodeReplaceChild(parent->parent, parent, NodeOther(parent, leaf), tree);
			return subtree;
		}
	}
}

#pragma mark Leaf Functions

static Node *
LeafNew(cpBBTree *tree, void *obj, cpBB bb)
{
	Node *node = NodeFromPool(tree);
	node->obj = obj;
	node->bb = get_bb(tree, obj);
	
	node->parent = NULL;
	node->stamp = 0;
	node->pairs = NULL;
	
	return node;
}

static void
LeafUpdate(Node *leaf, cpBBTree *tree)
{
	Node *root = tree->root;
	cpBB bb = tree->spatialIndex.bbfunc(leaf->obj);
	
	if(!cpBBcontainsBB(leaf->bb, bb)){
		leaf->bb = get_bb(tree, leaf->obj);
		
		root = SubtreeRemove(root, leaf, tree);
		tree->root = SubtreeInsert(root, leaf, tree);
		
		leaf->stamp = tree->stamp;
	}
}

#pragma mark Memory Management Functions

cpBBTree *
cpBBTreeAlloc(void)
{
	return (cpBBTree *)cpcalloc(1, sizeof(cpBBTree));
}

static int
leafSetEql(void *obj, Node *node)
{
	return (obj == node->obj);
}

static void *
leafSetTrans(void *obj, cpBBTree *tree)
{
	return LeafNew(tree, obj, tree->spatialIndex.bbfunc(obj));
}

cpBBTree *
cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc)
{
	tree->spatialIndex.klass = &klass;
	tree->velocityFunc = NULL;
	
	tree->spatialIndex.bbfunc = bbfunc;
	tree->leaves = cpHashSetNew(0, (cpHashSetEqlFunc)leafSetEql, (cpHashSetTransFunc)leafSetTrans, NULL);
	tree->root = NULL;
	
	tree->pooledNodes = NULL;
	tree->allocatedBuffers = cpArrayNew(0);
	
	tree->stamp = 0;
	
	return tree;
}

void
cpBBTreeSetVelocityFunc(cpSpatialIndex *index, cpBBTreeVelocityFunc func)
{
	if(index->klass != &klass){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeSetVelocityFunc() call to non-tree spatial index.");
		return;
	}
	
	((cpBBTree *)index)->velocityFunc = func;
}

cpBBTree *
cpBBTreeNew(cpSpatialIndexBBFunc bbfunc)
{
	return cpBBTreeInit(cpBBTreeAlloc(), bbfunc);
}

static void
cpBBTreeDestroy(cpBBTree *tree)
{
	cpHashSetFree(tree->leaves);
	
	cpArrayFreeEach(tree->allocatedBuffers, cpfree);
	cpArrayFree(tree->allocatedBuffers);
}

#pragma mark Insert/Remove

static void
cpBBTreeInsert(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	Node *leaf = cpHashSetInsert(tree->leaves, hashid, obj, tree);
	
	Node *root = tree->root;
	tree->root = (root ? SubtreeInsert(root, leaf, tree) : leaf);
	
	leaf->stamp = tree->stamp;
}

static void
cpBBTreeRemove(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	Node *leaf = cpHashSetRemove(tree->leaves, hashid, obj);
	
	tree->root = SubtreeRemove(tree->root, leaf, tree);
	PairsClear(leaf, tree);
	NodeRecycle(tree, leaf);
}

static cpBool
cpBBTreeContains(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	return (cpHashSetFind(tree->leaves, hashid, obj) != NULL);
}

#pragma mark Reindex

static void
cpBBTreeReindex(cpBBTree *tree)
{
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)LeafUpdate, tree);
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
	if(tree->root) SubtreeQuery(tree->root, &point, cpBBNew(point.x, point.y, point.x, point.y), func, data);
}

static void
cpBBTreeSegmentQuery(cpBBTree *tree, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryCallback func, void *data)
{
	cpAssert(cpFalse, "TODO Not implemented");
}

static void
cpBBTreeQuery(cpBBTree *tree, void *obj, cpBB bb, cpSpatialIndexQueryCallback func, void *data)
{
	if(tree->root) SubtreeQuery(tree->root, obj, bb, func, data);
}

#pragma mark ReindexCollide and Helper Functions

typedef struct traverseContext {
	cpBBTree *tree;
	Node *staticRoot;
	cpSpatialIndexQueryCallback func;
	void *data;
} traverseContext;

static void
TraverseMarkQuery(Node *subtree, Node *leaf, cpBool left, traverseContext *context)
{
	if(cpBBintersects(leaf->bb, subtree->bb)){
		if(NodeIsLeaf(subtree)){
			if(left){
				PairInsert(leaf, subtree, context->tree);
			} else {
				PairInsert(subtree, leaf, context->tree);
				context->func(leaf->obj, subtree->obj, context->data);
			}
		} else {
			TraverseMarkQuery(subtree->a, leaf, left, context);
			TraverseMarkQuery(subtree->b, leaf, left, context);
		}
	}
}

static void
TraverseMarkLeaf(Node *leaf, traverseContext *context)
{
	cpBBTree *tree = context->tree;
	if(leaf->stamp == tree->stamp){
		PairsClear(leaf, tree);
		
		Node *staticRoot = context->staticRoot;
		if(staticRoot) TraverseMarkQuery(staticRoot, leaf, cpFalse, context);
		
		for(Node *node = leaf; node->parent; node = node->parent){
			if(node == node->parent->a){
				TraverseMarkQuery(node->parent->b, leaf, cpTrue, context);
			} else {
				TraverseMarkQuery(node->parent->a, leaf, cpFalse, context);
			}
		}
	} else {
		Pair *pair = leaf->pairs;
		while(pair){
			if(leaf == pair->b.leaf){
				context->func(leaf->obj, pair->a.leaf->obj, context->data);
				pair = pair->b.next;
			} else {
				pair = pair->a.next;
			}
		}
	}
}

static void
TraverseMark(Node *subtree, traverseContext *context)
{
	if(NodeIsLeaf(subtree)){
		TraverseMarkLeaf(subtree, context);
	} else {
		TraverseMark(subtree->a, context);
		TraverseMark(subtree->b, context);
	}
}

void
cpBBTreeReindexCollide(cpBBTree *tree, cpSpatialIndex *staticIndex, cpSpatialIndexQueryCallback func, void *data)
{
	Node *root = tree->root;
	if(!root) return;
	
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)LeafUpdate, tree);
	
	cpBBTree *staticTree = (staticIndex->klass == &klass ? (cpBBTree *)staticIndex : NULL);
	traverseContext context = {tree, (staticTree ? staticTree->root : NULL), func, data};
	TraverseMark(root, &context);
	tree->stamp++;
	
	if(!staticTree) cpSpatialIndexCollideStatic((cpSpatialIndex *)tree, staticIndex, func, data);
}

#pragma mark Misc

static int
cpBBTreeCount(cpBBTree *tree)
{
	return cpHashSetCount(tree->leaves);
}

typedef struct eachContext {
	cpSpatialIndexIterator func;
	void *data;
} eachContext;

static void each_helper(Node *node, eachContext *context){context->func(node->obj, context->data);}

static void
cpBBTreeEach(cpBBTree *tree, cpSpatialIndexIterator func, void *data)
{
	eachContext context = {func, data};
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)each_helper, &context);
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
	(cpSpatialIndexReindexCollideFunc)cpBBTreeReindexCollide,
};

#pragma mark Tree Optimization

static int
cpfcompare(const cpFloat *a, const cpFloat *b){
	return (*a < *b ? -1 : (*b < *a ? 1 : 0));
}

static void
fillNodeArray(Node *node, Node ***cursor){
	(**cursor) = node;
	(*cursor)++;
}

static Node *
partitionNodes(cpBBTree *tree, Node **nodes, int count)
{
	if(count == 1){
		return nodes[0];
	} else if(count == 2) {
		return NodeNew(tree, nodes[0], nodes[1]);
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
	if(splitWidth) a.r = b.l = split; else a.t = b.b = split;
	
	// Partition the nodes
	int right = count;
	for(int left=0; left < right;){
		Node *node = nodes[left];
		if(cpBBMergedArea(node->bb, b) < cpBBMergedArea(node->bb, a)){
//		if(cpBBProximity(node->bb, b) < cpBBProximity(node->bb, a)){
			right--;
			nodes[left] = nodes[right];
			nodes[right] = node;
		} else {
			left++;
		}
	}
	
	if(right == count){
		Node *node = NULL;
		for(int i=0; i<count; i++) node = SubtreeInsert(node, nodes[i], tree);
		return node;
	}
	
	// Recurse and build the node!
	return NodeNew(tree,
		partitionNodes(tree, nodes, right),
		partitionNodes(tree, nodes + right, count - right)
	);
}

//static void
//cpBBTreeOptimizeIncremental(cpBBTree *tree, int passes)
//{
//	for(int i=0; i<passes; i++){
//		Node *root = tree->root;
//		Node *node = root;
//		int bit = 0;
//		unsigned int path = tree->opath;
//		
//		while(!NodeIsLeaf(node)){
//			node = (path&(1<<bit) ? node->a : node->b);
//			bit = (bit + 1)&(sizeof(unsigned int)*8 - 1);
//		}
//		
//		root = subtreeRemove(root, node, tree);
//		tree->root = subtreeInsert(root, node, tree);
//	}
//}

void
cpBBTreeOptimize(cpSpatialIndex *index)
{
	if(index->klass != &klass){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeOptimize() call to non-tree spatial index.");
		return;
	}
	
	cpBBTree *tree = (cpBBTree *)index;
	Node *root = tree->root;
	if(!root) return;
	
	int count = cpBBTreeCount(tree);
	Node *nodes[count];
	Node **cursor = nodes;
	
	cpHashSetEach(tree->leaves, (cpHashSetIterFunc)fillNodeArray, &cursor);
	
	SubtreeRecycle(tree, root);
	tree->root = partitionNodes(tree, nodes, count);
}

#pragma mark Debug Draw

#define CP_BBTREE_DEBUG_DRAW
#ifdef CP_BBTREE_DEBUG_DRAW
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#include <GLUT/glut.h>

static void
NodeRender(Node *node, int depth)
{
	if(!NodeIsLeaf(node) && depth <= 10){
		NodeRender(node->a, depth + 1);
		NodeRender(node->b, depth + 1);
	}
	
	cpBB bb = node->bb;
	
//	GLfloat v = depth/2.0f;	
//	glColor3f(1.0f - v, v, 0.0f);
	glLineWidth(cpfmax(5.0f - depth, 1.0f));
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
cpBBTreeRenderDebug(cpSpatialIndex *index){
	if(index->klass != &klass){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeRenderDebug() call to non-tree spatial index.");
		return;
	}
	
	cpBBTree *tree = (cpBBTree *)index;
	if(tree->root) NodeRender(tree->root, 0);
}
#endif
