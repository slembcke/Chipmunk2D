/* Copyright (c) 2007 Scott Lembcke
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
 
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "chipmunk.h"
#include "drawSpace.h"
#include "ChipmunkDemo.h"

#include "chipmunk_unsafe.h"

extern cpSpace *space;
extern cpBody *staticBody;
extern cpVect mousePoint;
char messageString[1024];

cpShape *querySeg = NULL;
cpShape *queryShape = NULL;

extern int cpShapeSegmentQuery(cpShape *shape, cpVect a, cpVect b, unsigned int layers, unsigned int group, cpSegmentQueryInfo *info);
#define NUM_VERTS 5

static void
update(int ticks)
{
	messageString[0] = '\0';
	
	cpVect start = cpvzero;
	cpVect end = mousePoint;
	
	{
		char infoString[1024];
		sprintf(infoString, "Query: Dist(%f) Point%s, ", cpvdist(start, end), cpvstr(end));
		strcat(messageString, infoString);
	}
	
	cpSegmentQueryInfo info = {};
	if(cpShapeSegmentQuery(queryShape, start, end, -1, 0, &info)){
		cpVect point = cpvlerp(start, end, info.t);
//		end = cpvsub(info->point, cpvmult(cpvnormalize(info->point), 4.0f));
		end = cpvadd(point, cpvmult(info.n, 4.0f));
		
		char infoString[1024];
		sprintf(infoString, "Segment Query: Dist(%f) Normal%s", cpvdist(start, mousePoint)*info.t, cpvstr(info.n));
		strcat(messageString, infoString);
	} else {
		strcat(messageString, "Segment Query (None)");
	}
	
	cpSegmentShapeSetEndpoints(querySeg, cpvzero, end);
}

static cpSpace *
init(void)
{
	staticBody = cpBodyNew(INFINITY, INFINITY);
	
	cpResetShapeIdCounter();
	
	space = cpSpaceNew();
	space->iterations = 5;
	space->gravity = cpv(0, -100);
	
	cpSpaceResizeStaticHash(space, 40.0, 999);
	cpSpaceResizeActiveHash(space, 30.0, 2999);
	
	cpBody *body;
	cpShape *shape;
	
	// add a non-collidable segment as a quick and dirty way to draw the query line
	shape = cpSegmentShapeNew(staticBody, cpvzero, cpv(100.0f, 0.0f), 0.0f);
	cpSpaceAddStaticShape(space, shape);
	shape->layers = 0;
	querySeg = shape;
	
	cpVect verts[NUM_VERTS];
	for(int i=0; i<NUM_VERTS; i++){
		cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
		verts[i] = cpv(30*cos(angle), 30*sin(angle));
	}
	
	// Make a shape to query against
	body = cpBodyNew(1.0f, 1.0f);
	shape = cpCircleShapeNew(body, 20.0f, cpv(100, 100));
//	shape = cpSegmentShapeNew(body, cpv(-10, 100), cpv(10, 200), 20.0f);
//	shape = cpPolyShapeNew(body, NUM_VERTS, verts, cpv(100,100));
	cpSpaceAddStaticShape(space, shape);
	queryShape = shape;
	
	return space;
}

static void
destroy(void)
{
	cpBodyFree(staticBody);
	cpSpaceFreeChildren(space);
	cpSpaceFree(space);
}

const chipmunkDemo Query = {
	"Query",
	NULL,
	init,
	update,
	destroy,
};
