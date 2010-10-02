#include <stdlib.h>

#include "chipmunk.h"

void
cpSpatialIndexFree(cpSpatialIndex *index)
{
	cpSpatialIndexDestroy(index);
	cpfree(index);
}

