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

#include "chipmunk_private.h"
#include "ChipmunkDemo.h"

static cpSpace *space;
static cpShape *shape1, *shape2;

static void
update(int ticks)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static void
draw(void)
{
	ChipmunkDemoDefaultDrawImpl();
	
	// draw the minkowski difference origin
	cpVect origin = cpvzero;
	ChipmunkDebugDrawPoints(5.0, 1, &origin, RGBAColor(1,0,0,1));
	
	// draw the minkowski difference
	int shape1Count = cpPolyShapeGetNumVerts(shape1);
	int shape2Count = cpPolyShapeGetNumVerts(shape2);
	
	int mdiffCount = shape1Count*shape2Count;
	cpVect *mdiffVerts = alloca(mdiffCount*sizeof(cpVect));
	
	for(int i=0; i<shape1Count; i++){
		for(int j=0; j<shape2Count; j++){
			cpVect v1 = cpBodyLocal2World(cpShapeGetBody(shape1), cpPolyShapeGetVert(shape1, i));
			cpVect v2 = cpBodyLocal2World(cpShapeGetBody(shape2), cpPolyShapeGetVert(shape2, j));
			mdiffVerts[i*shape2Count + j] = cpvsub(v2, v1);
		}
	}
	
	cpVect *hullVerts = alloca(mdiffCount*sizeof(cpVect));
	int hullCount = cpConvexHull(mdiffCount, mdiffVerts, hullVerts, NULL, 0.0);
	
	ChipmunkDebugDrawPolygon(hullCount, hullVerts, RGBAColor(1, 0, 0, 1), RGBAColor(1, 0, 0, 0.25));
	ChipmunkDebugDrawPoints(2.0, mdiffCount, mdiffVerts, RGBAColor(1, 0, 0, 1));
}

static cpSpace *
init(void)
{
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 5);
	
	{
		cpFloat width = 50.0;
		cpFloat height = 70.0;
		
		cpFloat mass = 1.0f;
		cpFloat moment = cpMomentForBox(mass, width, height);
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
		cpBodySetPos(body, cpv(50.0f, 0.0f));
		
		shape1 = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
		shape1->group = 1;
	}
	
	{
		cpVect verts[] = {{-25, -25}, {-25, 25}, {50, -25}};
		
		cpFloat mass = 1.0f;
		cpFloat moment = cpMomentForPoly(mass, 3, verts, cpvzero);
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
		cpBodySetPos(body, cpv(-50.0f, 0.0f));
		
		shape2 = cpSpaceAddShape(space, cpPolyShapeNew(body, 3, verts, cpvzero));
		shape2->group = 1;
	}
	
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo GJK = {
	"GJK",
	init,
	update,
	draw,
	destroy,
};
