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
pivotJointPreStep(cpConstraint *joint, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	cpPivotJoint *jnt = (cpPivotJoint *)joint;
	
	jnt->r1 = cpvrotate(jnt->anchr1, a->rot);
	jnt->r2 = cpvrotate(jnt->anchr2, b->rot);
	
	// Calculate mass tensor
	k_tensor(a, b, jnt->r1, jnt->r2, &jnt->k1, &jnt->k2);
	
	// compute max impulse
	jnt->jMaxLen = J_MAX(jnt, dt);
	
	// calculate bias velocity
	cpVect delta = cpvsub(cpvadd(b->p, jnt->r2), cpvadd(a->p, jnt->r1));
	jnt->bias = cpvmult(delta, -jnt->constraint.biasCoef*dt_inv);
	
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
		
	// compute relative velocity
	cpVect vr = relative_velocity(a, b, r1, r2);
	
	// compute normal impulse
	cpVect j = cpvsub(jnt->bias, mult_k(vr, jnt->k1, jnt->k2));
	cpVect jOld = jnt->jAcc;
	jnt->jAcc = clamp_vect(cpvadd(jnt->jAcc, j), jnt->jMaxLen);
	j = cpvsub(jnt->jAcc, jOld);
	
	// apply impulse
	apply_impulses(a, b, jnt->r1, jnt->r2, j);
}

static cpFloat
cpPivotJointGetImpulse(cpConstraint *joint)
{
	return cpvlength(((cpPivotJoint *)joint)->jAcc);
}

static const cpConstraintClass pivotJointClass = {
	pivotJointPreStep,
	pivotJointApplyImpulse,
	cpPivotJointGetImpulse,
};

cpPivotJoint *
cpPivotJointAlloc(void)
{
	return (cpPivotJoint *)malloc(sizeof(cpPivotJoint));
}

cpPivotJoint *
cpPivotJointInit(cpPivotJoint *joint, cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2)
{
	cpConstraintInit((cpConstraint *)joint, &pivotJointClass, a, b);
	
//	joint->anchr1 = cpvunrotate(cpvsub(pivot, a->p), a->rot);
//	joint->anchr2 = cpvunrotate(cpvsub(pivot, b->p), b->rot);
	joint->anchr1 = anchr1;
	joint->anchr2 = anchr2;
	
	joint->jAcc = cpvzero;
	
	return joint;
}

cpConstraint *
cpPivotJointNew(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2)
{
	return (cpConstraint *)cpPivotJointInit(cpPivotJointAlloc(), a, b, anchr1, anchr2);
}
