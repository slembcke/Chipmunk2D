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

#include "../chipmunk.h"
#include "util.h"

static void
preStep(cpDampedRotarySpring *spring, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;
	
	spring->iSum = 1.0f/(a->moment_inv + b->moment_inv);

	spring->dt = dt;
	spring->target_wrn = 0.0f;

	// apply spring torque
	cpFloat j_spring = (a->angle - b->angle - spring->restAngle)*spring->stiffness*dt;
	a->ang_vel -= j_spring*a->moment_inv;
	b->ang_vel += j_spring*b->moment_inv;
}

static void
applyImpulse(cpDampedRotarySpring *spring)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;
	
	// compute relative velocity
	cpFloat wrn = a->ang_vel - b->ang_vel;//normal_relative_velocity(a, b, r1, r2, n) - spring->target_vrn;
	
	// compute velocity loss from drag
	// not 100% certain this is derived correctly, though it makes sense
	cpFloat w_damp = wrn*(1.0f - cpfexp(-spring->damping*spring->dt/spring->iSum));
	spring->target_wrn = wrn - w_damp;
	
	//apply_impulses(a, b, spring->r1, spring->r2, cpvmult(spring->n, v_damp*spring->nMass));
	cpFloat j_damp = w_damp*spring->iSum;
	a->ang_vel -= j_damp*a->moment_inv;
	b->ang_vel += j_damp*b->moment_inv;
}

static cpFloat
getImpulse(cpConstraint *constraint)
{
	return 0.0f;
}

const cpConstraintClass cpDampedRotarySpringClass = {
	(cpConstraintPreStepFunction)preStep,
	(cpConstraintApplyImpulseFunction)applyImpulse,
	(cpConstraintGetImpulseFunction)getImpulse,
};

cpDampedRotarySpring *
cpDampedRotarySpringAlloc(void)
{
	return (cpDampedRotarySpring *)malloc(sizeof(cpDampedRotarySpring));
}

cpDampedRotarySpring *
cpDampedRotarySpringInit(cpDampedRotarySpring *spring, cpBody *a, cpBody *b, cpFloat restAngle, cpFloat stiffness, cpFloat damping)
{
	cpConstraintInit((cpConstraint *)spring, &cpDampedRotarySpringClass, a, b);
	
	spring->restAngle = restAngle;
	spring->stiffness = stiffness;
	spring->damping = damping;
	
	return spring;
}

cpConstraint *
cpDampedRotarySpringNew(cpBody *a, cpBody *b, cpFloat restAngle, cpFloat stiffness, cpFloat damping)
{
	return (cpConstraint *)cpDampedRotarySpringInit(cpDampedRotarySpringAlloc(), a, b, restAngle, stiffness, damping);
}
