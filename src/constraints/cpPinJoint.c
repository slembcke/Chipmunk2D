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

//#include <stdlib.h>
//#include <math.h>

#include <stdlib.h>

#include "chipmunk.h"
#include "util.h"

static void
pinJointPreStep(cpConstraint *joint, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	cpPinJoint *jnt = (cpPinJoint *)joint;
	
	jnt->r1 = cpvrotate(jnt->anchr1, a->rot);
	jnt->r2 = cpvrotate(jnt->anchr2, b->rot);
	
	cpVect delta = cpvsub(cpvadd(b->p, jnt->r2), cpvadd(a->p, jnt->r1));
	cpFloat dist = cpvlength(delta);
	jnt->n = cpvmult(delta, 1.0f/(dist ? dist : INFINITY));
	
	// calculate mass normal
	jnt->nMass = 1.0f/k_scalar(a, b, jnt->r1, jnt->r2, jnt->n);
	
	// calculate bias velocity
	jnt->bias = -jnt->constraint.biasCoef*dt_inv*(dist - jnt->dist);
	
	// compute max impulse
	jnt->jnMax = J_MAX(jnt, dt);
	
	// apply accumulated impulse
	cpVect j = cpvmult(jnt->n, jnt->jnAcc);
	apply_impulses(a, b, jnt->r1, jnt->r2, j);
}

static void
pinJointApplyImpulse(cpConstraint *joint)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	
	cpPinJoint *jnt = (cpPinJoint *)joint;
	cpVect n = jnt->n;

	// compute relative velocity
	cpFloat vrn = normal_relative_velocity(a, b, jnt->r1, jnt->r2, n);
	
	// compute normal impulse
	cpFloat jn = (jnt->bias - vrn)*jnt->nMass;
	cpFloat jnOld = jnt->jnAcc;
	jnt->jnAcc = cpfclamp(jnOld + jn, -jnt->jnMax, jnt->jnMax);
	jn = jnt->jnAcc - jnOld;
	
	// apply impulse
	apply_impulses(a, b, jnt->r1, jnt->r2, cpvmult(n, jn));
}

static const cpConstraintClass pinJointClass = {
	pinJointPreStep,
	pinJointApplyImpulse,
};

cpPinJoint *
cpPinJointAlloc(void)
{
	return (cpPinJoint *)malloc(sizeof(cpPinJoint));
}

cpPinJoint *
cpPinJointInit(cpPinJoint *joint, cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2)
{
	cpConstraintInit((cpConstraint *)joint, &pinJointClass, a, b);
	
	joint->anchr1 = anchr1;
	joint->anchr2 = anchr2;
	
	cpVect p1 = cpvadd(a->p, cpvrotate(anchr1, a->rot));
	cpVect p2 = cpvadd(b->p, cpvrotate(anchr2, b->rot));
	joint->dist = cpvlength(cpvsub(p2, p1));

	joint->jnAcc = 0.0;
	
	return joint;
}

cpConstraint *
cpPinJointNew(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2)
{
	return (cpConstraint *)cpPinJointInit(cpPinJointAlloc(), a, b, anchr1, anchr2);
}
