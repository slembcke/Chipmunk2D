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
 
#include "chipmunk.h"
#include "constraints/util.h"

#include "ChipmunkDemo.h"

static void
update(cpSpace *space)
{
	int steps = 3;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

#define FLUID_DENSITY 0.00014
#define FLUID_DRAG 2.0

char messageBuffer[1024] = {};

static cpBool
waterPreSolve(cpArbiter *arb, cpSpace *space, void *ptr)
{
	CP_ARBITER_GET_SHAPES(arb, water, poly);
	cpBody *body = cpShapeGetBody(poly);
	
	// Get the top of the water sensor bounding box to use as the water level.
	cpFloat level = cpShapeGetBB(water).t;
	
	// Clip the polygon against the water level
	int count = cpPolyShapeGetNumVerts(poly);
	int clippedCount = 0;
#ifdef _MSC_VER
	// MSVC is pretty much the only compiler in existence that doesn't support variable sized arrays.
	cpVect clipped[10];
#else
	cpVect clipped[count + 1];
#endif

	for(int i=0, j=count-1; i<count; j=i, i++){
		cpVect a = cpBodyLocal2World(body, cpPolyShapeGetVert(poly, j));
		cpVect b = cpBodyLocal2World(body, cpPolyShapeGetVert(poly, i));
		
		if(a.y < level){
			clipped[clippedCount] = a;
			clippedCount++;
		}
		
		cpFloat a_level = a.y - level;
		cpFloat b_level = b.y - level;
		
		if(a_level*b_level < 0.0f){
			cpFloat t = cpfabs(a_level)/(cpfabs(a_level) + cpfabs(b_level));
			
			clipped[clippedCount] = cpvlerp(a, b, t);
			clippedCount++;
		}
	}
	
	// Calculate buoyancy from the clipped polygon area
	cpFloat clippedArea = cpAreaForPoly(clippedCount, clipped);
	cpFloat displacedMass = clippedArea*FLUID_DENSITY;
	cpVect centroid = cpCentroidForPoly(clippedCount, clipped);
	cpVect r = cpvsub(centroid, cpBodyGetPos(body));
	
	ChipmunkDebugDrawPolygon(clippedCount, clipped, RGBAColor(0, 0, 1, 1), RGBAColor(0, 0, 1, 0.1f));
	ChipmunkDebugDrawPoints(5, 1, &centroid, RGBAColor(0, 0, 1, 1));
	
	cpFloat dt = cpSpaceGetCurrentTimeStep(space);
	cpVect g = cpSpaceGetGravity(space);
	
	// Apply the buoyancy force as an impulse.
	apply_impulse(body, cpvmult(g, -displacedMass*dt), r);
	
	// Apply linear damping for the fluid drag.
	cpVect v_centroid = cpvadd(body->v, cpvmult(cpvperp(r), body->w));
	cpFloat k = k_scalar_body(body, r, cpvnormalize_safe(v_centroid));
	cpFloat damping = clippedArea*FLUID_DRAG*FLUID_DENSITY;
	cpFloat v_coef = cpfexp(-damping*dt*k); // linear drag
//	cpFloat v_coef = 1.0/(1.0 + damping*dt*cpvlength(v_centroid)*k); // quadratic drag
	apply_impulse(body, cpvmult(cpvsub(cpvmult(v_centroid, v_coef), v_centroid), 1.0/k), r);
	
	// Apply angular damping for the fluid drag.
	cpFloat w_damping = cpMomentForPoly(FLUID_DRAG*FLUID_DENSITY*clippedArea, clippedCount, clipped, cpvneg(body->p));
	body->w *= cpfexp(-w_damping*dt*body->i_inv);
	
	return cpTrue;
}

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = messageBuffer;
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -500));
//	cpSpaceSetDamping(space, 0.5);
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	cpSpaceSetCollisionSlop(space, 0.5f);
	
	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	{
		// Add the edges of the bucket
		cpBB bb = cpBBNew(-300, -200, 100, 0);
		cpFloat radius = 5.0f;
		
		shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(bb.l, bb.b), cpv(bb.l, bb.t), radius));
		cpShapeSetElasticity(shape, 1.0f);
		cpShapeSetFriction(shape, 1.0f);
		cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

		shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(bb.r, bb.b), cpv(bb.r, bb.t), radius));
		cpShapeSetElasticity(shape, 1.0f);
		cpShapeSetFriction(shape, 1.0f);
		cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

		shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(bb.l, bb.b), cpv(bb.r, bb.b), radius));
		cpShapeSetElasticity(shape, 1.0f);
		cpShapeSetFriction(shape, 1.0f);
		cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
		
		// Add the sensor for the water.
		shape = cpSpaceAddShape(space, cpBoxShapeNew2(staticBody, bb));
		cpShapeSetSensor(shape, cpTrue);
		cpShapeSetCollisionType(shape, 1);
	}


	{
		cpFloat width = 200.0f;
		cpFloat height = 50.0f;
		cpFloat mass = 0.3*FLUID_DENSITY*width*height;
		cpFloat moment = cpMomentForBox(mass, width, height);
		
		body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
		cpBodySetPos(body, cpv(-50, -100));
		cpBodySetVel(body, cpv(0, -100));
		cpBodySetAngVel(body, 1);
		
		shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
		cpShapeSetFriction(shape, 0.8f);
	}
	
	{
		cpFloat width = 40.0f;
		cpFloat height = width*2;
		cpFloat mass = 0.3*FLUID_DENSITY*width*height;
		cpFloat moment = cpMomentForBox(mass, width, height);
		
		body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
		cpBodySetPos(body, cpv(-200, -50));
		cpBodySetVel(body, cpv(0, -100));
		cpBodySetAngVel(body, 1);
		
		shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
		cpShapeSetFriction(shape, 0.8f);
	}
	
	cpSpaceAddCollisionHandler(space, 1, 0, NULL, (cpCollisionPreSolveFunc)waterPreSolve, NULL, NULL, NULL);
		
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Buoyancy = {
	"Simple Sensor based fluids.",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
