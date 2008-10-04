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

#include "chipmunk.h"
#include "util.h"

static void
slideJointPreStep(cpConstraint *joint, cpFloat dt_inv)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	cpSlideJoint *jnt = (cpSlideJoint *)joint;
	
	jnt->r1 = cpvrotate(jnt->anchr1, a->rot);
	jnt->r2 = cpvrotate(jnt->anchr2, b->rot);
	
	cpVect delta = cpvsub(cpvadd(b->p, jnt->r2), cpvadd(a->p, jnt->r1));
	cpFloat dist = cpvlength(delta);
	cpFloat pdist = 0.0;
	if(dist > jnt->max) {
		pdist = dist - jnt->max;
	} else if(dist < jnt->min) {
		pdist = jnt->min - dist;
		dist = -dist;
	}
	jnt->n = cpvmult(delta, 1.0f/(dist ? dist : INFINITY));
	
	// calculate mass normal
	jnt->nMass = 1.0f/k_scalar(a, b, jnt->r1, jnt->r2, jnt->n);
	
	// calculate bias velocity
	jnt->bias = -cp_constraint_bias_coef*dt_inv*(pdist);
	jnt->jBias = 0.0f;
	
	// apply accumulated impulse
	if(!jnt->bias) //{
		// if bias is 0, then the joint is not at a limit.
		jnt->jnAcc = 0.0f;
//	} else {
		cpVect j = cpvmult(jnt->n, jnt->jnAcc);
		apply_impulses(a, b, jnt->r1, jnt->r2, j);
//	}
}

static void
slideJointApplyImpulse(cpConstraint *joint)
{
	cpSlideJoint *jnt = (cpSlideJoint *)joint;
	if(!jnt->bias) return;  // early exit

	cpBody *a = joint->a;
	cpBody *b = joint->b;
	
	cpVect n = jnt->n;
	cpVect r1 = jnt->r1;
	cpVect r2 = jnt->r2;
	
	//calculate bias impulse
	cpVect vbr = relative_velocity(r1, a->v_bias, a->w_bias, r2, b->v_bias, b->w_bias);
	cpFloat vbn = cpvdot(vbr, n);
	
	cpFloat jbn = (jnt->bias - vbn)*jnt->nMass;
	cpFloat jbnOld = jnt->jBias;
	jnt->jBias = cpfmin(jbnOld + jbn, 0.0f);
	jbn = jnt->jBias - jbnOld;
	
	cpVect jb = cpvmult(n, jbn);
	apply_bias_impulses(a, b, jnt->r1, jnt->r2, jb);
	
	// compute relative velocity
	cpVect vr = relative_velocity(r1, a->v, a->w, r2, b->v, b->w);
	cpFloat vrn = cpvdot(vr, n);
	
	// compute normal impulse
	cpFloat jn = -vrn*jnt->nMass;
	cpFloat jnOld = jnt->jnAcc;
	jnt->jnAcc = cpfmin(jnOld + jn, 0.0f);
	jn = jnt->jnAcc - jnOld;
	
	// apply impulse
	cpVect j = cpvmult(n, jn);
	apply_impulses(a, b, jnt->r1, jnt->r2, j);
}

static const cpConstraintClass slideJointClass = {
	slideJointPreStep,
	slideJointApplyImpulse,
};

cpSlideJoint *
cpSlideJointAlloc(void)
{
	return (cpSlideJoint *)malloc(sizeof(cpSlideJoint));
}

cpSlideJoint *
cpSlideJointInit(cpSlideJoint *joint, cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat min, cpFloat max)
{
	cpConstraintInit((cpConstraint *)joint, &slideJointClass, a, b);
	
	joint->anchr1 = anchr1;
	joint->anchr2 = anchr2;
	joint->min = min;
	joint->max = max;
	
	joint->jnAcc = 0.0;
	
	return joint;
}

cpConstraint *
cpSlideJointNew(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat min, cpFloat max)
{
	return (cpConstraint *)cpSlideJointInit(cpSlideJointAlloc(), a, b, anchr1, anchr2, min, max);
}
