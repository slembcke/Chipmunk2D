#include <stdio.h>
#include <chipmunk.h>

int main(void){
	// cpVect is a 2D vector and cpv() is a shortcut for initializing them.
	cpVect gravity = cpv(0, -100);
	
	// Create an empty space.
	cpSpace *space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	
	// Add a static line segment shape for the ground.
	// We'll make it slightly tilted so the ball will roll off.
	// We attach it to space->staticBody to tell Chipmunk it shouldn't be movable.
	cpShape *ground = cpSegmentShapeNew(space->staticBody, cpv(-20, 5), cpv(20, -5), 0);
	cpShapeSetFriction(ground, 1);
	cpSpaceAddShape(space, ground);
	
	// Now let's make a ball that falls onto the line and rolls off.
	// First we need to make a cpBody to hold the physical properties of the object.
	// These include the mass, position, velocity, angle, etc. of the object.
	// Then we attach collision shapes to the cpBody to give it a size and shape.
	
	cpFloat radius = 5;
	cpFloat mass = 1;
	
	// The moment of inertia is like mass for rotation
	// Use the cpMomentFor*() functions to help you approximate it.
	cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
	
	// The cpSpaceAdd*() functions return the thing that you are adding.
	// It's convenient to create and add an object in one line.
	cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(ballBody, cpv(0, 15));
	
	// Now we create the collision shape for the ball.
	// You can create multiple collision shapes that point to the same body.
	// They will all be attached to the body and move around to follow it.
	cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
	cpShapeSetFriction(ballShape, 0.7);
	
	// Now that it's all set up, we simulate all the objects in the space by
	// stepping forward through time in small increments called steps.
	// It is *highly* recommended to use a fixed size time step.
	cpFloat timeStep = 1.0/60.0;
	for(cpFloat time = 0; time < 2; time += timeStep){
		cpVect pos = cpBodyGetPos(ballBody);
		cpVect vel = cpBodyGetVel(ballBody);
		printf(
			"Time is %5.2f. ballBody is at (%5.2f, %5.2f). It's velocity is (%5.2f, %5.2f)\n",
			time, pos.x, pos.y, vel.x, vel.y
		);
		
		cpSpaceStep(space, timeStep);
	}
	
	// Clean up our objects and exit!
	cpShapeFree(ballShape);
	cpBodyFree(ballBody);
	cpShapeFree(ground);
	cpSpaceFree(space);
	
	return 0;
}