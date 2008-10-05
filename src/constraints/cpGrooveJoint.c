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
grooveJointPreStep(cpConstraint *joint, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	cpGrooveJoint *jnt = (cpGrooveJoint *)joint;
	
	// calculate endpoints in worldspace
	cpVect ta = cpBodyLocal2World(a, jnt->grv_a);
	cpVect tb = cpBodyLocal2World(a, jnt->grv_b);

	// calculate axis
	cpVect n = cpvrotate(jnt->grv_n, a->rot);
	cpFloat d = cpvdot(ta, n);
	
	jnt->grv_tn = n;
	jnt->r2 = cpvrotate(jnt->anchr2, b->rot);
	
	// calculate tangential distance along the axis of r2
	cpFloat td = cpvcross(cpvadd(b->p, jnt->r2), n);
	// calculate clamping factor and r2
	if(td <= cpvcross(ta, n)){
		jnt->clamp = 1.0f;
		jnt->r1 = cpvsub(ta, a->p);
	} else if(td >= cpvcross(tb, n)){
		jnt->clamp = -1.0f;
		jnt->r1 = cpvsub(tb, a->p);
	} else {
		jnt->clamp = 0.0f;
		jnt->r1 = cpvsub(cpvadd(cpvmult(cpvperp(n), -td), cpvmult(n, d)), a->p);
	}
	
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

static inline cpVect
grooveConstrain(cpGrooveJoint *jnt, cpVect j){
	cpVect n = jnt->grv_tn;
	cpVect jClamp = (jnt->clamp*cpvcross(j, n) > 0.0f) ? j : cpvmult(n, cpvdot(j, n));
	return clamp_vect(jClamp, jnt->jMaxLen);
}

static void
grooveJointApplyImpulse(cpConstraint *joint)
{
	cpBody *a = joint->a;
	cpBody *b = joint->b;
	
	cpGrooveJoint *jnt = (cpGrooveJoint *)joint;
	cpVect r1 = jnt->r1;
	cpVect r2 = jnt->r2;
	
	// compute impulse
	cpVect vr = relative_velocity(a, b, r1, r2);

	cpVect j = cpvsub(jnt->bias, mult_k(vr, jnt->k1, jnt->k2));
	cpVect jOld = jnt->jAcc;
	jnt->jAcc = grooveConstrain(jnt, cpvadd(jOld, j));
	j = cpvsub(jnt->jAcc, jOld);
	
	// apply impulse
	apply_impulses(a, b, jnt->r1, jnt->r2, j);
}

static cpFloat
cpGrooveJointApplyImpulse(cpConstraint *joint)
{
	return cpvlength(((cpGrooveJoint *)joint)->jAcc);
}

static const cpConstraintClass grooveJointClass = {
	grooveJointPreStep,
	grooveJointApplyImpulse,
	cpGrooveJointApplyImpulse,
};

cpGrooveJoint *
cpGrooveJointAlloc(void)
{
	return (cpGrooveJoint *)malloc(sizeof(cpGrooveJoint));
}

cpGrooveJoint *
cpGrooveJointInit(cpGrooveJoint *joint, cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchr2)
{
	cpConstraintInit((cpConstraint *)joint, &grooveJointClass, a, b);
	
	joint->grv_a = groove_a;
	joint->grv_b = groove_b;
	joint->grv_n = cpvperp(cpvnormalize(cpvsub(groove_b, groove_a)));
	joint->anchr2 = anchr2;
	
	joint->jAcc = cpvzero;
	
	return joint;
}

cpConstraint *
cpGrooveJointNew(cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchr2)
{
	return (cpConstraint *)cpGrooveJointInit(cpGrooveJointAlloc(), a, b, groove_a, groove_b, anchr2);
}
