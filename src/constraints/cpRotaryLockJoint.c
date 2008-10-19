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
#include "util.h"

static void
preStep(cpRotaryLockJoint *joint, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	// calculate moment of inertia coefficient.
	joint->iSum = 1.0f/(a->i_inv + b->i_inv);
	
	// calculate bias velocity
	joint->bias = -joint->constraint.biasCoef*dt_inv*(b->a - a->a - joint->offset);
	
	// compute max impulse
	joint->jMax = J_MAX(joint, dt);

	// apply joint torque
	a->w -= joint->jAcc*a->i_inv;
	b->w += joint->jAcc*b->i_inv;
}

static void
applyImpulse(cpRotaryLockJoint *joint)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	// compute relative rotational velocity
	cpFloat wr = b->w - a->w;
	
	// compute normal impulse	
	cpFloat j = (joint->bias - wr)*joint->iSum;
	cpFloat jOld = joint->jAcc;
	joint->jAcc = cpfclamp(jOld + j, -joint->jMax, joint->jMax);
	j = joint->jAcc - jOld;
	
	// apply impulse
	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static cpFloat
getImpulse(cpRotaryLockJoint *joint)
{
	return fabs(joint->jAcc);
}

const cpConstraintClass cpRotaryLockJointClass = {
	(cpConstraintPreStepFunction)preStep,
	(cpConstraintApplyImpulseFunction)applyImpulse,
	(cpConstraintGetImpulseFunction)getImpulse,
};

cpRotaryLockJoint *
cpRotaryLockJointAlloc(void)
{
	return (cpRotaryLockJoint *)malloc(sizeof(cpRotaryLockJoint));
}

cpRotaryLockJoint *
cpRotaryLockJointInit(cpRotaryLockJoint *joint, cpBody *a, cpBody *b, cpFloat offset)
{
	cpConstraintInit((cpConstraint *)joint, &cpRotaryLockJointClass, a, b);
	
	joint->offset = offset;
	
	return joint;
}

cpConstraint *
cpRotaryLockJointNew(cpBody *a, cpBody *b, cpFloat offset)
{
	return (cpConstraint *)cpRotaryLockJointInit(cpRotaryLockJointAlloc(), a, b, offset);
}
