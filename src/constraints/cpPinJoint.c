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
pinJointPreStep(cpConstraint *joint, cpFloat dt_inv)
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
	jnt->bias = -cp_constraint_bias_coef*dt_inv*(dist - jnt->dist);
	jnt->jBias = 0.0f;
	
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
	cpVect r1 = jnt->r1;
	cpVect r2 = jnt->r2;

	//calculate bias impulse
	cpVect vbr = relative_velocity(r1, a->v_bias, a->w_bias, r2, b->v_bias, b->w_bias);
	cpFloat vbn = cpvdot(vbr, n);
	
	cpFloat jbn = (jnt->bias - vbn)*jnt->nMass;
	jnt->jBias += jbn;
	
	cpVect jb = cpvmult(n, jbn);
	apply_bias_impulses(a, b, jnt->r1, jnt->r2, jb);
	
	// compute relative velocity
	cpVect vr = relative_velocity(r1, a->v, a->w, r2, b->v, b->w);
	cpFloat vrn = cpvdot(vr, n);
	
	// compute normal impulse
	cpFloat jn = -vrn*jnt->nMass;
	jnt->jnAcc =+ jn;
	
	// apply impulse
	cpVect j = cpvmult(n, jn);
	apply_impulses(a, b, jnt->r1, jnt->r2, j);
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
