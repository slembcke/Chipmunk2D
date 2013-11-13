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

static cpBody *planetBody;

static cpFloat gravityStrength = 5.0e6f;

static void
update(cpSpace *space, double dt)
{
	cpSpaceStep(space, dt);
	
	// Update the static body spin so that it looks like it's rotating.
	cpBodyUpdatePosition(planetBody, dt);
}

static void
planetGravityVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	// Gravitational acceleration is proportional to the inverse square of
	// distance, and directed toward the origin. The central planet is assumed
	// to be massive enough that it affects the satellites but not vice versa.
	cpVect p = cpBodyGetPosition(body);
	cpFloat sqdist = cpvlengthsq(p);
	cpVect g = cpvmult(p, -gravityStrength / (sqdist * cpfsqrt(sqdist)));
	
	cpBodyUpdateVelocity(body, g, damping, dt);
}

static cpVect
rand_pos(cpFloat radius)
{
	cpVect v;
	do {
		v = cpv(frand()*(640 - 2*radius) - (320 - radius), frand()*(480 - 2*radius) - (240 - radius));
	} while(cpvlength(v) < 85.0f);
	
	return v;
}

static void
add_box(cpSpace *space)
{
	const cpFloat size = 10.0f;
	const cpFloat mass = 1.0f;
	
	cpVect verts[] = {
		cpv(-size,-size),
		cpv(-size, size),
		cpv( size, size),
		cpv( size,-size),
	};
	
	cpFloat radius = cpvlength(cpv(size, size));
	cpVect pos = rand_pos(radius);
	
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, 4, verts, cpvzero, 0.0f)));
	cpBodySetVelocityUpdateFunc(body, planetGravityVelocityFunc);
	cpBodySetPosition(body, pos);

	// Set the box's velocity to put it into a circular orbit from its
	// starting position.
	cpFloat r = cpvlength(pos);
	cpFloat v = cpfsqrt(gravityStrength / r) / r;
	cpBodySetVelocity(body, cpvmult(cpvperp(pos), v));

	// Set the box's angular velocity to match its orbital period and
	// align its initial angle with its position.
	cpBodySetAngularVelocity(body, v);
	cpBodySetAngle(body, cpfatan2(pos.y, pos.x));

	cpShape *shape = cpSpaceAddShape(space, cpPolyShapeNew(body, 4, verts, cpTransformIdentity, 0.0));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
}

static cpSpace *
init(void)
{
	// Create a rouge body to control the planet manually.
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 20);
	
	planetBody = cpSpaceAddBody(space, cpBodyNewKinematic());
	cpBodySetAngularVelocity(planetBody, 0.2f);
	
	for(int i=0; i<30; i++){
		add_box(space);
	}
	
	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(planetBody, 70.0f, cpvzero));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Planet = {
	"Planet",
	1.0/60.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
