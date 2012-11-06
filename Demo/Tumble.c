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
#include "ChipmunkDemo.h"

static cpBody *rogueBoxBody;

static void
update(cpSpace *space)
{
	int steps = 3;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		// Manually update the position of the box body so that the box rotates.
		// Normally Chipmunk calls this and cpBodyUpdateVelocity() for you,
		// but we wanted to control the angular velocity explicitly.
		cpBodyUpdatePosition(rogueBoxBody, dt);
		
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
	cpSpace *space = cpSpaceNew();
	cpSpaceSetGravity(space, cpv(0, -600));
	
	cpBody *body;
	cpShape *shape;
	
	// We create an infinite mass rogue body to attach the line segments too
	// This way we can control the rotation however we want.
	rogueBoxBody = cpBodyNew(INFINITY, INFINITY);
	cpBodySetAngVel(rogueBoxBody, 0.4f);
	
	// Set up the static box.
	cpVect a = cpv(-200, -200);
	cpVect b = cpv(-200,  200);
	cpVect c = cpv( 200,  200);
	cpVect d = cpv( 200, -200);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(rogueBoxBody, a, b, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(rogueBoxBody, b, c, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(rogueBoxBody, c, d, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(rogueBoxBody, d, a, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	cpFloat mass = 1;
	cpFloat width = 60;
	cpFloat height = 30;
	
	// Add the bricks.
	for(int i=0; i<3; i++){
		for(int j=0; j<7; j++){
			body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
			cpBodySetPos(body, cpv(i*60 - 150, j*30 - 150));
			
			shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
			cpShapeSetElasticity(shape, 0.0f);
			cpShapeSetFriction(shape, 0.7f);
		}
	}
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpBodyFree(rogueBoxBody);
	cpSpaceFree(space);
}

ChipmunkDemo Tumble = {
	"Tumble",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
