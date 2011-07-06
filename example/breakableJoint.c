// Create the joint and set it's max force property.
breakableJoint = cpSpaceAddConstraint(space, cpPinJointNew(body1, body2, cpv(15,0), cpv(-15,0)));
cpConstraintSetMaxForce(breakableJoint, 4000);


// In your update function:
// Step your space normally...
cpFloat dt = 1.0/60.0;
cpSpaceStep(space, dt);

if(breakableJoint){
	// Convert the impulse to a force by dividing it by the timestep.
	cpFloat force = cpConstraintGetImpulse(breakableJoint)/dt;
	cpFloat maxForce = cpConstraintGetMaxForce(breakableJoint);

	// If the force is almost as big as the joint's max force, break it.
	if(force > 0.9*maxForce){
		cpSpaceRemoveConstraint(space, breakableJoint);
		breakableJoint = NULL;
	}
}
