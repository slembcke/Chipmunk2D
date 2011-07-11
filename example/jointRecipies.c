// Faked top down friction.

// A pivot joint configured this way will calculate friction against the ground for games with a top down perspective.
// Because the joint correction is disabled, the joint will not recenter itself and only apply to the velocity.
// The force the joint applies when changing the velocity will be clamped by the max force
// and this causes it to work exactly like friction!
cpConstraint *pivot = cpSpaceAddConstraint(space, cpPivotJointNew2(staticBody, body, cpvzero, cpvzero));
pivot->maxBias = 0.0f; // disable joint correction
pivot->maxForce = 1000.0f;

// The pivot joint doesn't apply rotational forces, use a gear joint with a ratio of 1.0 for that.
cpConstraint *gear = cpSpaceAddConstraint(space, cpGearJointNew(staticBody, body, 0.0f, 1.0f));
gear->maxBias = 0.0f; // disable joint correction
gear->maxForce = 5000.0f;

// Also, instead of connecting the joints to a static body, you can connect them to an infinite mass rogue body.
// You can then use the rogue body as a control body to the connected body. See the Tank demo as an example.