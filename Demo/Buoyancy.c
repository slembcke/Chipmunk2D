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

#include "chipmunk.h"
#include "ChipmunkDemo.h"

static cpSpace *space;

static void
update(int ticks)
{
	int steps = 3;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

//static cpBool
//waterPreSolve(cpArbiter *arb, cpSpace *space, Boat *self)
//{
//	CHIPMUNK_ARBITER_GET_SHAPES(arb, object, water);
//	
//	cpFloat level = water.bb.t;
//	cpPolyShape *pshape = (cpPolyShape *)object.shape;
//	
//	int count = pshape->numVerts;
//	cpVect *verts = pshape->tVerts;
//	
//	int clippedCount = 0;
//	cpVect clipped[count + 1];
//	for(int i=0, j=count-1; i<count; j=i, i++){
//		cpVect a = verts[j];
//		cpVect b = verts[i];
//		
//		if(a.y < level){
//			clipped[clippedCount] = a;
//			clippedCount++;
//		}
//		
//		cpFloat a_level = a.y - level;
//		cpFloat b_level = b.y - level;
//		
//		if(a_level*b_level < 0.0f){
//			cpFloat t = cpfabs(a_level)/(cpfabs(a_level) + cpfabs(b_level));
//			
//			clipped[clippedCount] = cpvlerp(a, b, t);
//			clippedCount++;
//		}
//	}
//	
//	cpBody *body = object.body.body;
//	cpFloat area = cpfabs(cpAreaForPoly(count, verts));
//	cpFloat clippedArea = cpfabs(cpAreaForPoly(clippedCount, clipped));
//	cpVect r = cpvsub(cpCentroidForPoly(clippedCount, clipped), body->p);
//	
//	cpVect bouyancy = cpvmult(space->gravity, -clippedArea*FLUID_DENSITY);
//	
//	cpBodyResetForces(body);
//	cpBodyApplyForce(body, bouyancy, r);
//	
//	cpFloat v_coef = cpfpow(0.97f, clippedArea/area);
//	cpVect v_centroid = cpvadd(body->v, cpvmult(cpvperp(r), body->w));
//	
//	cpBodyApplyImpulse(body, cpvmult(v_centroid, v_coef - 1.0f), r);
//	body->w *= v_coef*v_coef;
//	
//	return TRUE;
//}

static cpSpace *
init(void)
{
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -100));
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
	
	// Add lots of boxes.
	for(int i=0; i<14; i++){
		for(int j=0; j<=i; j++){
			body = cpSpaceAddBody(space, cpBodyNew(1.0f, cpMomentForBox(1.0f, 30.0f, 30.0f)));
			cpBodySetPos(body, cpv(j*32 - i*16, 300 - i*32));
			
			shape = cpSpaceAddShape(space, cpBoxShapeNew(body, 30.0f, 30.0f));
			cpShapeSetElasticity(shape, 0.0f);
			cpShapeSetFriction(shape, 0.8f);
		}
	}
	
	// Add a ball to make things more interesting
	cpFloat radius = 15.0f;
	body = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
	cpBodySetPos(body, cpv(0, -240 + radius+5));

	shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.9f);
	
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Buoyancy = {
	"Pyramid Stack",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
