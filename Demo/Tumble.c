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

static cpBody *KinematicBoxBody;

static void
update(cpSpace *space, double dt)
{
	// Manually update the position of the box body so that the box rotates.
	// Normally Chipmunk calls this and cpBodyUpdateVelocity() for you,
	// but we wanted to control the angular velocity explicitly.
	cpBodyUpdatePosition(KinematicBoxBody, dt);
	
	cpSpaceStep(space, dt);
}

static void
AddBox(cpSpace *space, cpVect pos, cpFloat mass, cpFloat width, cpFloat height)
{
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPosition(body, pos);
	
	cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height, 0.0));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
}

static void
AddSegment(cpSpace *space, cpVect pos, cpFloat mass, cpFloat width, cpFloat height)
{
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPosition(body, pos);
	
	cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(body, cpv(0.0, (height - width)/2.0), cpv(0.0, (width - height)/2.0), width/2.0));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
}

static void
AddCircle(cpSpace *space, cpVect pos, cpFloat mass, cpFloat radius)
{
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0, radius, cpvzero)));
	cpBodySetPosition(body, pos);
	
	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
}

static cpSpace *
init(void)
{
	cpSpace *space = cpSpaceNew();
	cpSpaceSetGravity(space, cpv(0, -600));
	
	cpShape *shape;
	
	// We create an infinite mass rogue body to attach the line segments too
	// This way we can control the rotation however we want.
	KinematicBoxBody = cpSpaceAddBody(space, cpBodyNewKinematic());
	cpBodySetAngularVelocity(KinematicBoxBody, 0.4f);
	
	// Set up the static box.
	cpVect a = cpv(-200, -200);
	cpVect b = cpv(-200,  200);
	cpVect c = cpv( 200,  200);
	cpVect d = cpv( 200, -200);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(KinematicBoxBody, a, b, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(KinematicBoxBody, b, c, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(KinematicBoxBody, c, d, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(KinematicBoxBody, d, a, 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	cpFloat mass = 1;
	cpFloat width = 30;
	cpFloat height = width*2;
	
	// Add the bricks.
	for(int i=0; i<7; i++){
		for(int j=0; j<3; j++){
			cpVect pos = cpv(i*width - 150, j*height - 150);
			
			int type = (rand()%3000)/1000;
			if(type ==0){
				AddBox(space, pos, mass, width, height);
			} else if(type == 1){
				AddSegment(space, pos, mass, width, height);
			} else {
				AddCircle(space, cpvadd(pos, cpv(0.0, (height - width)/2.0)), mass, width/2.0);
				AddCircle(space, cpvadd(pos, cpv(0.0, (width - height)/2.0)), mass, width/2.0);
			}
		}
	}
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Tumble = {
	"Tumble",
	1.0/180.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
