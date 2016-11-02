/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "chipmunk/chipmunk_private.h"

void
cpSpatialIndexFree(cpSpatialIndex *index)
{
	if(index){
		cpSpatialIndexDestroy(index);
		cpfree(index);
	}
}

cpSpatialIndex *
cpSpatialIndexInit(cpSpatialIndex *index, cpSpatialIndexClass *klass, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	index->klass = klass;
	index->bbfunc = bbfunc;
	index->staticIndex = staticIndex;
	
	if(staticIndex){
		cpAssertHard(!staticIndex->dynamicIndex, "This static index is already associated with a dynamic index.");
		staticIndex->dynamicIndex = index;
	}
	
	return index;
}

typedef struct dynamicToStaticContext {
	cpSpatialIndexBBFunc bbfunc;
	cpSpatialIndex *staticIndex;
	cpSpatialIndexQueryFunc queryFunc;
	void *data;
} dynamicToStaticContext;

static void
dynamicToStaticIter(void *obj, dynamicToStaticContext *context)
{
	cpSpatialIndexQuery(context->staticIndex, obj, context->bbfunc(obj), context->queryFunc, context->data);
}

void
cpSpatialIndexCollideStatic(cpSpatialIndex *dynamicIndex, cpSpatialIndex *staticIndex, cpSpatialIndexQueryFunc func, void *data)
{
	if(staticIndex && cpSpatialIndexCount(staticIndex) > 0){
		dynamicToStaticContext context = {dynamicIndex->bbfunc, staticIndex, func, data};
		cpSpatialIndexEach(dynamicIndex, (cpSpatialIndexIteratorFunc)dynamicToStaticIter, &context);
	}
}

// declarations of inline functions in "cpSpatialIndex.h"

void cpSpatialIndexDestroy(cpSpatialIndex *index);
int cpSpatialIndexCount(cpSpatialIndex *index);
void cpSpatialIndexEach(cpSpatialIndex *index, cpSpatialIndexIteratorFunc func, void *data);
cpBool cpSpatialIndexContains(cpSpatialIndex *index, void *obj, cpHashValue hashid);
void cpSpatialIndexInsert(cpSpatialIndex *index, void *obj, cpHashValue hashid);
void cpSpatialIndexRemove(cpSpatialIndex *index, void *obj, cpHashValue hashid);
void cpSpatialIndexReindex(cpSpatialIndex *index);
void cpSpatialIndexReindexObject(cpSpatialIndex *index, void *obj, cpHashValue hashid);
void cpSpatialIndexQuery(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data);
void cpSpatialIndexSegmentQuery(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data);
void cpSpatialIndexReindexQuery(cpSpatialIndex *index, cpSpatialIndexQueryFunc func, void *data);

