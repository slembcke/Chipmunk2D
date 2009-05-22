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

extern int cpShapeSegmentQuery(cpShape *shape, cpVect a, cpVect b, unsigned int layers, unsigned int group, cpSegmentQueryInfo *info);


typedef void (*cpSpaceSegmentQueryFunc)(cpShape *shape, cpFloat t, cpVect n, void *data);

typedef struct segQueryContext {
	cpVect start, end;
	unsigned int layers, group;
	cpSpaceSegmentQueryFunc func;
	int anyCollision;
} segQueryContext;

static void
segQueryFunc(segQueryContext *context, cpShape *shape, void *data)
{
	cpSegmentQueryInfo info = {};
	if(cpShapeSegmentQuery(shape, context->start, context->end, context->layers, context->group, &info)){
		if(context->func)
			context->func(shape, info.t, info.n, data);
		
		context->anyCollision = context->anyCollision || 1;
	}
}

extern void raytrace(cpSpaceHash *hash, void *obj, cpVect a, cpVect b, cpSpaceHashQueryFunc func, void *data);

static int
cpSpaceShapeSegmentQuery(cpSpace *space, cpVect start, cpVect end, unsigned int layers, unsigned int group, cpSpaceSegmentQueryFunc func, void *data)
{
	segQueryContext context = {
		start, end,
		layers, group,
		func,
		0,
	};
	
	raytrace(space->staticShapes, &context, start, end, (cpSpaceHashQueryFunc)segQueryFunc, data);
	raytrace(space->activeShapes, &context, start, end, (cpSpaceHashQueryFunc)segQueryFunc, data);
	
	return context.anyCollision;
}

static void
storeFirstCollision(cpShape *shape, cpFloat t, cpVect n, cpSegmentQueryInfo *info)
{
	if(t > info->t) return;
	
	info->shape = shape;
	info->t = t;
	info->n = n;
}

static int
cpSpaceShapeSegmentQueryFirst(cpSpace *space, cpVect start, cpVect end, unsigned int layers, unsigned int group, cpSegmentQueryInfo *out)
{
	cpSegmentQueryInfo info;
	info.t = INFINITY;
	
	if(cpSpaceShapeSegmentQuery(space, start, end, -1, 0, (cpSpaceSegmentQueryFunc)storeFirstCollision, &info)){
		(*out) = info;
		return 1;
	} else {
		return 0;
	}
}



static void
update(int ticks)
{
	messageString[0] = '\0';
	
	cpVect start = cpvzero;
	cpVect end = mousePoint;
	cpVect lineEnd = end;
	
	{
		char infoString[1024];
		sprintf(infoString, "Query: Dist(%f) Point%s, ", cpvdist(start, end), cpvstr(end));
		strcat(messageString, infoString);
	}
	
	cpSegmentQueryInfo info = {};
	if(cpSpaceShapeSegmentQueryFirst(space, start, end, -1, 0, &info)){
		cpVect point = cpvlerp(start, end, info.t);
		lineEnd = cpvadd(point, cpvzero);//cpvmult(info.n, 4.0f));
		
		char infoString[1024];
		sprintf(infoString, "Segment Query: Dist(%f) Normal%s", cpvdist(start, end)*info.t, cpvstr(info.n));
		strcat(messageString, infoString);
	} else {
		strcat(messageString, "Segment Query (None)");
	}
	
	cpSegmentShapeSetEndpoints(querySeg, cpvzero, lineEnd);
	
	// normal other stuff.
	int steps = 1;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
	staticBody = cpBodyNew(INFINITY, INFINITY);
	
	cpResetShapeIdCounter();
	
	space = cpSpaceNew();
	space->elasticIterations = 0;
	space->iterations = 5;
	
	cpSpaceResizeStaticHash(space, 40.0, 999);
	cpSpaceResizeActiveHash(space, 30.0, 2999);
	
	cpShape *shape;
	
	// add a non-collidable segment as a quick and dirty way to draw the query line
	shape = cpSegmentShapeNew(staticBody, cpvzero, cpv(100.0f, 0.0f), 4.0f);
	cpSpaceAddStaticShape(space, shape);
	shape->layers = 0;
	querySeg = shape;
	
	{ // add a fat segment
		cpFloat mass = 1.0f;
		cpFloat length = 100.0f;
		cpVect a = cpv(-length/2.0f, 0.0f), b = cpv(length/2.0f, 0.0f);
		
		cpBody *body = cpBodyNew(mass, cpMomentForSegment(mass, a, b));
		cpSpaceAddBody(space, body);
		body->p = cpv(0.0f, 100.0f);
		
		cpSpaceAddShape(space, cpSegmentShapeNew(body, a, b, 20.0f));
	}
	
	{ // add a pentagon
		cpFloat mass = 1.0f;
		int NUM_VERTS = 5;
		
		cpVect verts[NUM_VERTS];
		for(int i=0; i<NUM_VERTS; i++){
			cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
			verts[i] = cpv(30*cos(angle), 30*sin(angle));
		}
		
		cpBody *body = cpBodyNew(mass, cpMomentForPoly(mass, NUM_VERTS, verts, cpvzero));
		cpSpaceAddBody(space, body);
		body->p = cpv(50.0f, 50.0f);
		
		cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpvzero));
	}
	
	{ // add a circle
		cpFloat mass = 1.0f;
		cpFloat r = 20.0f;
		
		cpBody *body = cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, r, cpvzero));
		cpSpaceAddBody(space, body);
		body->p = cpv(100.0f, 100.0f);
		
		cpSpaceAddShape(space, cpCircleShapeNew(body, r, cpvzero));
	}
	
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
