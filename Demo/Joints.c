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

static cpBody *
addBall(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat radius = 15.0f;
	cpFloat mass = 1.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPosition(body, cpvadd(pos, boxOffset));
	
	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
	
	return body;
}

static cpBody *
addLever(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat mass = 1.0f;
	cpVect a = cpv(0,  15);
	cpVect b = cpv(0, -15);
	
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForSegment(mass, a, b, 0.0f)));
	cpBodySetPosition(body, cpvadd(pos, cpvadd(boxOffset, cpv(0, -15))));
	
	cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(body, a, b, 5.0f));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
	
	return body;
}

static cpBody *
addBar(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat mass = 2.0f;
	cpVect a = cpv(0,  30);
	cpVect b = cpv(0, -30);
	
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForSegment(mass, a, b, 0.0f)));
	cpBodySetPosition(body, cpvadd(pos, boxOffset));
	
	cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(body, a, b, 5.0f));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
	cpShapeSetFilter(shape, cpShapeFilterNew(1, CP_ALL_CATEGORIES, CP_ALL_CATEGORIES));
	
	return body;
}

static cpBody *
addWheel(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat radius = 15.0f;
	cpFloat mass = 1.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	cpBodySetPosition(body, cpvadd(pos, boxOffset));
	
	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
	cpShapeSetFilter(shape, cpShapeFilterNew(1, CP_ALL_CATEGORIES, CP_ALL_CATEGORIES));
	
	return body;
}

static cpBody *
addChassis(cpSpace *space, cpVect pos, cpVect boxOffset)
{
	cpFloat mass = 5.0f;
	cpFloat width = 80;
	cpFloat height = 30;
	
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, width, height)));
	cpBodySetPosition(body, cpvadd(pos, boxOffset));
	
	cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height, 0.0));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.7f);
	cpShapeSetFilter(shape, cpShapeFilterNew(1, CP_ALL_CATEGORIES, CP_ALL_CATEGORIES));
	
	return body;
}

static cpSpace *
init(void)
{
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 10);
	cpSpaceSetGravity(space, cpv(0, -100));
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	
	cpBody *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,120), cpv(320,120), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,0), cpv(320,0), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-120), cpv(320,-120), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-160,-240), cpv(-160,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(0,-240), cpv(0,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(160,-240), cpv(160,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetFilter(shape, NOT_GRABBABLE_FILTER);
	
	cpVect boxOffset;
	cpBody *body1, *body2;
	
	cpVect posA = cpv( 50, 60);
	cpVect posB = cpv(110, 60);
	
	#define POS_A cpvadd(boxOffset, posA)
	#define POS_B cpvadd(boxOffset, posB)
	
	// Pin Joints - Link shapes with a solid bar or pin.
	// Keeps the anchor points the same distance apart from when the joint was created.
	boxOffset = cpv(-320, -240);
	body1 = addBall(space, posA, boxOffset);
	body2 = addBall(space, posB, boxOffset);
	cpSpaceAddConstraint(space, cpPinJointNew(body1, body2, cpv(15,0), cpv(-15,0)));
	
	// Slide Joints - Like pin joints but with a min/max distance.
	// Can be used for a cheap approximation of a rope.
	boxOffset = cpv(-160, -240);
	body1 = addBall(space, posA, boxOffset);
	body2 = addBall(space, posB, boxOffset);
	cpSpaceAddConstraint(space, cpSlideJointNew(body1, body2, cpv(15,0), cpv(-15,0), 20.0f, 40.0f));
	
	// Pivot Joints - Holds the two anchor points together. Like a swivel.
	boxOffset = cpv(0, -240);
	body1 = addBall(space, posA, boxOffset);
	body2 = addBall(space, posB, boxOffset);
	cpSpaceAddConstraint(space, cpPivotJointNew(body1, body2, cpvadd(boxOffset, cpv(80,60))));
	// cpPivotJointNew() takes it's anchor parameter in world coordinates. The anchors are calculated from that
	// cpPivotJointNew2() lets you specify the two anchor points explicitly
	
	// Groove Joints - Like a pivot joint, but one of the anchors is a line segment that the pivot can slide in
	boxOffset = cpv(160, -240);
	body1 = addBall(space, posA, boxOffset);
	body2 = addBall(space, posB, boxOffset);
	cpSpaceAddConstraint(space, cpGrooveJointNew(body1, body2, cpv(30,30), cpv(30,-30), cpv(-30,0)));
	
	// Damped Springs
	boxOffset = cpv(-320, -120);
	body1 = addBall(space, posA, boxOffset);
	body2 = addBall(space, posB, boxOffset);
	cpSpaceAddConstraint(space, cpDampedSpringNew(body1, body2, cpv(15,0), cpv(-15,0), 20.0f, 5.0f, 0.3f));
	
	// Damped Rotary Springs
	boxOffset = cpv(-160, -120);
	body1 = addBar(space, posA, boxOffset);
	body2 = addBar(space, posB, boxOffset);
	// Add some pin joints to hold the circles in place.
	cpSpaceAddConstraint(space, cpPivotJointNew(body1, staticBody, POS_A));
	cpSpaceAddConstraint(space, cpPivotJointNew(body2, staticBody, POS_B));
	cpSpaceAddConstraint(space, cpDampedRotarySpringNew(body1, body2, 0.0f, 3000.0f, 60.0f));
	
	// Rotary Limit Joint
	boxOffset = cpv(0, -120);
	body1 = addLever(space, posA, boxOffset);
	body2 = addLever(space, posB, boxOffset);
	// Add some pin joints to hold the circles in place.
	cpSpaceAddConstraint(space, cpPivotJointNew(body1, staticBody, POS_A));
	cpSpaceAddConstraint(space, cpPivotJointNew(body2, staticBody, POS_B));
	// Hold their rotation within 90 degrees of each other.
	cpSpaceAddConstraint(space, cpRotaryLimitJointNew(body1, body2, -M_PI_2, M_PI_2));
	
	// Ratchet Joint - A rotary ratchet, like a socket wrench
	boxOffset = cpv(160, -120);
	body1 = addLever(space, posA, boxOffset);
	body2 = addLever(space, posB, boxOffset);
	// Add some pin joints to hold the circles in place.
	cpSpaceAddConstraint(space, cpPivotJointNew(body1, staticBody, POS_A));
	cpSpaceAddConstraint(space, cpPivotJointNew(body2, staticBody, POS_B));
	// Ratchet every 90 degrees
	cpSpaceAddConstraint(space, cpRatchetJointNew(body1, body2, 0.0f, M_PI_2));
	
	// Gear Joint - Maintain a specific angular velocity ratio
	boxOffset = cpv(-320, 0);
	body1 = addBar(space, posA, boxOffset);
	body2 = addBar(space, posB, boxOffset);
	// Add some pin joints to hold the circles in place.
	cpSpaceAddConstraint(space, cpPivotJointNew(body1, staticBody, POS_A));
	cpSpaceAddConstraint(space, cpPivotJointNew(body2, staticBody, POS_B));
	// Force one to sping 2x as fast as the other
	cpSpaceAddConstraint(space, cpGearJointNew(body1, body2, 0.0f, 2.0f));
	
	// Simple Motor - Maintain a specific angular relative velocity
	boxOffset = cpv(-160, 0);
	body1 = addBar(space, posA, boxOffset);
	body2 = addBar(space, posB, boxOffset);
	// Add some pin joints to hold the circles in place.
	cpSpaceAddConstraint(space, cpPivotJointNew(body1, staticBody, POS_A));
	cpSpaceAddConstraint(space, cpPivotJointNew(body2, staticBody, POS_B));
	// Make them spin at 1/2 revolution per second in relation to each other.
	cpSpaceAddConstraint(space, cpSimpleMotorNew(body1, body2, M_PI));
	
	// Make a car with some nice soft suspension
	boxOffset = cpv(0, 0);
	cpBody *wheel1 = addWheel(space, posA, boxOffset);
	cpBody *wheel2 = addWheel(space, posB, boxOffset);
	cpBody *chassis = addChassis(space, cpv(80, 100), boxOffset);
	
	cpSpaceAddConstraint(space, cpGrooveJointNew(chassis, wheel1, cpv(-30, -10), cpv(-30, -40), cpvzero));
	cpSpaceAddConstraint(space, cpGrooveJointNew(chassis, wheel2, cpv( 30, -10), cpv( 30, -40), cpvzero));
	
	cpSpaceAddConstraint(space, cpDampedSpringNew(chassis, wheel1, cpv(-30, 0), cpvzero, 50.0f, 20.0f, 10.0f));
	cpSpaceAddConstraint(space, cpDampedSpringNew(chassis, wheel2, cpv( 30, 0), cpvzero, 50.0f, 20.0f, 10.0f));
	
	return space;
}

static void
update(cpSpace *space, double dt)
{
	cpSpaceStep(space, dt);
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Joints = {
	"Joints and Constraints",
	1.0/60.0,
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
