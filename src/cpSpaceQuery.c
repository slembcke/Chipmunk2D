#include <stdlib.h>

#include "chipmunk.h"

typedef struct pointQueryContext {
	cpLayers layers;
	cpGroup group;
	cpSpacePointQueryFunc func;
	void *data;
} pointQueryContext;

static void 
pointQueryHelper(void *point, cpShape *shape, pointQueryContext *context)
{
	if(cpShapePointQuery(shape, *((cpVect *)point), context->layers, context->group))
		context->func(shape, context->data);
}

void
cpSpacePointQuery(cpSpace *space, cpVect point, cpLayers layers, cpLayers group, cpSpacePointQueryFunc func, void *data)
{
	pointQueryContext context = {layers, group, func, data};
	cpSpaceHashPointQuery(space->activeShapes, point, (cpSpaceHashQueryFunc)pointQueryHelper, &context);
	cpSpaceHashPointQuery(space->staticShapes, point, (cpSpaceHashQueryFunc)pointQueryHelper, &context);
}

static void
rememberLast(cpShape *shape, void **outShape)
{
	(*outShape) = shape;
}

cpShape *
cpSpacePointQueryFirst(cpSpace *space, cpVect point, cpLayers layers, cpLayers group)
{
	cpShape *shape = NULL;
	cpSpacePointQuery(space, point, layers, group, (cpSpacePointQueryFunc)rememberLast, &shape);
	
	return shape;
}
