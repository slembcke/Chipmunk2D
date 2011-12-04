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
#include <math.h>
#include <stdio.h>

#include "chipmunk.h"
#include "constraints/util.h"

#include "ChipmunkDemo.h"

#define DENSITY (1.0/10000.0)
#define CHUNK_SIZE 5.0
#define CHUNK_JITTER 0.5

static cpSpace *space;

static void 
CrumbleShape(cpSpace *space, cpShape *shape)
{
	cpBB bb = cpShapeGetBB(shape);
	
	// TODO need better ignore criteria
	if(cpBBArea(bb) < 4.0*CHUNK_SIZE*CHUNK_SIZE) return;
	
	cpBody *body = cpShapeGetBody(shape);
	cpSpaceRemoveShape(space, shape);
	cpSpaceRemoveBody(space, body);
	
	for(cpFloat x=bb.l; x<bb.r; x+=CHUNK_SIZE){
		for(cpFloat y=bb.b; y<bb.t; y+=CHUNK_SIZE){
			cpVect p = cpv(x, y);
			
			cpPointQueryExtendedInfo info;
			cpShapePointQueryExtended(shape, p, &info);
			
			if(info.d > CHUNK_SIZE/2.0){
				cpBody *chunkBody = cpSpaceAddBody(space, cpBodyNew(CHUNK_SIZE*CHUNK_SIZE*DENSITY, INFINITY));
				cpBodySetPos(chunkBody, cpvadd(p, cpv(CHUNK_JITTER*frand(), CHUNK_JITTER*frand())));
				cpBodySetVel(chunkBody, cpBodyGetVelAtWorldPoint(body, p));
				
				cpShape *chunkShape = cpSpaceAddShape(space, cpCircleShapeNew(chunkBody, CHUNK_SIZE/2.0, cpvzero));
				cpShapeSetFriction(chunkShape, 0.2);
			}
		}
	}
	
	cpShapeFree(shape);
	cpBodyFree(body);
}

static void
update(int ticks)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
	
	if(ChipmunkDemoRightClick){
		cpShape *shape = cpSpacePointQueryFirst(space, ChipmunkDemoMouse, GRABABLE_MASK_BIT, CP_NO_GROUP);
		if(shape) CrumbleShape(space, shape);
	}
	
	// TODO remove offscreen bodies
}

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "Right click to crumble a shape.";
	
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -500));
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	cpSpaceSetCollisionSlop(space, 0.5f);
	
	cpBody *staticBody = cpSpaceGetStaticBody(space);
	
	// Create segments around the edge of the screen.
	cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	{
		cpFloat width = 100.0f;
		cpFloat height = 200.0f;
		cpFloat mass = width*height*DENSITY;
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
		cpBodySetPos(body, cpv(0, -120));
		
		cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
		cpShapeSetFriction(shape, 0.7f);
	}
		
	{
		cpFloat radius = 50.0f;
		cpFloat mass = cpAreaForCircle(0.0, radius)*DENSITY;
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0, radius, cpvzero)));
		cpBodySetPos(body, cpv(0, 80));
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
		cpShapeSetFriction(shape, 0.7f);
	}
		
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Crumble = {
	"Crumble.",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
