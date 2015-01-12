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
 
#include "chipmunk/chipmunk.h"
#include "ChipmunkDemo.h"

static cpFloat pentagon_mass = 0.0f;
static cpFloat pentagon_moment = 0.0f;

// Iterate over all of the bodies and reset the ones that have fallen offscreen.
static void
eachBody(cpBody *body, void *unused)
{
	cpVect pos = cpBodyGetPosition(body);
	if(pos.y < -260 || cpfabs(pos.x) > 340){
		cpFloat x = rand()/(cpFloat)RAND_MAX*640 - 320;
		cpBodySetPosition(body, cpv(x, 260));
	}
}

static void
update(cpSpace *space, double dt)
{
	if(ChipmunkDemoRightDown){
		cpShape *nearest = cpSpacePointQueryNearest(space, ChipmunkDemoMouse, 0.0, GRAB_FILTER, NULL);
		if(nearest){
			cpBody *body = cpShapeGetBody(nearest);
			if(cpBodyGetType(body) == CP_BODY_TYPE_STATIC){
				cpBodySetType(body, CP_BODY_TYPE_DYNAMIC);
				cpBodySetMass(body, pentagon_mass);
				cpBodySetMoment(body, pentagon_moment);
			} else if(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC) {
				cpBodySetType(body, CP_BODY_TYPE_STATIC);
			}
		}
	}
	
	cpSpaceEachBody(space, &eachBody, NULL);
	cpSpaceStep(space, dt);
}

#define NUM_VERTS 5

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "Right click to make pentagons static/dynamic.";
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 5);
	cpSpaceSetGravity(space, cpv(0, -100));
		
	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	// Vertexes for a triangle shape.
	cpVect tris[] = {
		cpv(-15,-15),
		cpv(  0, 10),
		cpv( 15,-15),
	};

	// Create the static triangles.
	for(int i=0; i<9; i++){
		for(int j=0; j<6; j++){
			cpFloat stagger = (j%2)*40;
			cpVect offset = cpv(i*80 - 320 + stagger, j*70 - 240);
			shape = cpSpaceAddShape(space, cpPolyShapeNew(staticBody, 3, tris, cpTransformTranslate(offset), 0.0));
			cpShapeSetElasticity(shape, 1.0f);
			cpShapeSetFriction(shape, 1.0f);
			cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
		}
	}
	
	// Create vertexes for a pentagon shape.
	cpVect verts[NUM_VERTS];
	for(int i=0; i<NUM_VERTS; i++){
		cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
		verts[i] = cpv(10*cos(angle), 10*sin(angle));
	}
	
	pentagon_mass = 1.0;
	pentagon_moment = cpMomentForPoly(1.0f, NUM_VERTS, verts, cpvzero, 0.0f);
	
	// Add lots of pentagons.
	for(int i=0; i<300; i++){
		body = cpSpaceAddBody(space, cpBodyNew(pentagon_mass, pentagon_moment));
		cpFloat x = rand()/(cpFloat)RAND_MAX*640 - 320;
		cpBodySetPosition(body, cpv(x, 350));
		
		shape = cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpTransformIdentity, 0.0));
		cpShapeSetElasticity(shape, 0.0f);
		cpShapeSetFriction(shape, 0.4f);
	}
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Plink = {
	"Plink",
	1.0/60.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
