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
pivotJointPreStep(cpConstraint *joint, cpFloat dt_inv)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	cpPivotJoint *jnt = (cpPivotJoint *)joint;
	
	jnt->r1 = cpvrotate(jnt->anchr1, a->rot);
	jnt->r2 = cpvrotate(jnt->anchr2, b->rot);
	
	// calculate mass matrix
	// If I wasn't lazy, this wouldn't be so gross...
	cpFloat k11, k12, k21, k22;
	
	cpFloat m_sum = a->m_inv + b->m_inv;
	k11 = m_sum; k12 = 0.0f;
	k21 = 0.0f;  k22 = m_sum;
	
	cpFloat r1xsq =  jnt->r1.x * jnt->r1.x * a->i_inv;
	cpFloat r1ysq =  jnt->r1.y * jnt->r1.y * a->i_inv;
	cpFloat r1nxy = -jnt->r1.x * jnt->r1.y * a->i_inv;
	k11 += r1ysq; k12 += r1nxy;
	k21 += r1nxy; k22 += r1xsq;
	
	cpFloat r2xsq =  jnt->r2.x * jnt->r2.x * b->i_inv;
	cpFloat r2ysq =  jnt->r2.y * jnt->r2.y * b->i_inv;
	cpFloat r2nxy = -jnt->r2.x * jnt->r2.y * b->i_inv;
	k11 += r2ysq; k12 += r2nxy;
	k21 += r2nxy; k22 += r2xsq;
	
	cpFloat det_inv = 1.0f/(k11*k22 - k12*k21);
	jnt->k1 = cpv( k22*det_inv, -k12*det_inv);
	jnt->k2 = cpv(-k21*det_inv,  k11*det_inv);
	
	
	// calculate bias velocity
	cpVect delta = cpvsub(cpvadd(b->p, jnt->r2), cpvadd(a->p, jnt->r1));
	jnt->bias = cpvmult(delta, -cp_constraint_bias_coef*dt_inv);
	jnt->jBias = cpvzero;
	
	// apply accumulated impulse
	apply_impulses(a, b, jnt->r1, jnt->r2, jnt->jAcc);
}

static void
pivotJointApplyImpulse(cpConstraint *joint)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	
	cpPivotJoint *jnt = (cpPivotJoint *)joint;
	cpVect r1 = jnt->r1;
	cpVect r2 = jnt->r2;
	cpVect k1 = jnt->k1;
	cpVect k2 = jnt->k2;
	
	//calculate bias impulse
	cpVect vbr = relative_velocity(r1, a->v_bias, a->w_bias, r2, b->v_bias, b->w_bias);
	vbr = cpvsub(jnt->bias, vbr);
	
	cpVect jb = cpv(cpvdot(vbr, k1), cpvdot(vbr, k2));
	jnt->jBias = cpvadd(jnt->jBias, jb);
	
	apply_bias_impulses(a, b, jnt->r1, jnt->r2, jb);
	
	// compute relative velocity
	cpVect vr = relative_velocity(r1, a->v, a->w, r2, b->v, b->w);
	
	// compute normal impulse
	cpVect j = cpv(-cpvdot(vr, k1), -cpvdot(vr, k2));
	jnt->jAcc = cpvadd(jnt->jAcc, j);
	
	// apply impulse
	apply_impulses(a, b, jnt->r1, jnt->r2, j);
}

static const cpConstraintClass pivotJointClass = {
	pivotJointPreStep,
	pivotJointApplyImpulse,
};

cpPivotJoint *
cpPivotJointAlloc(void)
{
	return (cpPivotJoint *)malloc(sizeof(cpPivotJoint));
}

cpPivotJoint *
cpPivotJointInit(cpPivotJoint *joint, cpBody *a, cpBody *b, cpVect pivot)
{
	cpConstraintInit((cpConstraint *)joint, &pivotJointClass, a, b);
	
	joint->anchr1 = cpvunrotate(cpvsub(pivot, a->p), a->rot);
	joint->anchr2 = cpvunrotate(cpvsub(pivot, b->p), b->rot);
	
	joint->jAcc = cpvzero;
	
	return joint;
}

cpConstraint *
cpPivotJointNew(cpBody *a, cpBody *b, cpVect pivot)
{
	return (cpConstraint *)cpPivotJointInit(cpPivotJointAlloc(), a, b, pivot);
}
