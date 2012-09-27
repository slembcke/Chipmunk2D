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

#include "chipmunk_private.h"
#include "constraints/util.h"

static void
preStep(cpCustomConstraint *constraint, cpFloat dt)
{
	cpBody *a = constraint->constraint.a;
	cpBody *b = constraint->constraint.b;
	
	constraint->r1 = cpvrotate(constraint->anchr1, a->rot);
	constraint->r2 = cpvrotate(constraint->anchr2, b->rot);
	
	// Calculate mass tensor
	constraint->k = k_tensor(a, b, constraint->r1, constraint->r2);
	
	// compute max impulse
	constraint->jMaxLen = J_MAX(constraint, dt);
	
	// calculate bias velocity
	cpVect delta = cpvsub(cpvadd(b->p, constraint->r2), cpvadd(a->p, constraint->r1));
	constraint->bias = cpvclamp(cpvmult(delta, -bias_coef(constraint->constraint.errorBias, dt)/dt), constraint->constraint.maxBias);
}

static void
applyCachedImpulse(cpCustomConstraint *constraint, cpFloat dt_coef)
{
	cpBody *a = constraint->constraint.a;
	cpBody *b = constraint->constraint.b;
	
	apply_impulses(a, b, constraint->r1, constraint->r2, cpvmult(constraint->jAcc, dt_coef));
}

static void
applyImpulse(cpCustomConstraint *constraint)
{
	cpBody *a = constraint->constraint.a;
	cpBody *b = constraint->constraint.b;
	
	cpVect r1 = constraint->r1;
	cpVect r2 = constraint->r2;
		
	// compute relative velocity
	cpVect vr = relative_velocity(a, b, r1, r2);
	
	// compute normal impulse
	cpVect j = cpMat2x2Transform(constraint->k, cpvsub(constraint->bias, vr));
	cpVect jOld = constraint->jAcc;
	constraint->jAcc = cpvclamp(cpvadd(constraint->jAcc, j), constraint->jMaxLen);
	j = cpvsub(constraint->jAcc, jOld);
	
	// apply impulse
	apply_impulses(a, b, constraint->r1, constraint->r2, j);
}

static cpFloat
getImpulse(cpConstraint *constraint)
{
	return cpvlength(((cpCustomConstraint *)constraint)->jAcc);
}

static const cpConstraintClass klass = {
	(cpConstraintPreStepImpl)preStep,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulse,
	(cpConstraintApplyImpulseImpl)applyImpulse,
	(cpConstraintGetImpulseImpl)getImpulse,
};
CP_DefineClassGetter(cpCustomConstraint)

cpCustomConstraint *
cpCustomConstraintAlloc(void)
{
	return (cpCustomConstraint *)cpcalloc(1, sizeof(cpCustomConstraint));
}

cpCustomConstraint *
cpCustomConstraintInit(cpCustomConstraint *constraint, cpBody *a, cpBody *b)
{
	cpConstraintInit((cpConstraint *)constraint, &klass, a, b);
	
	constraint->jAcc = cpvzero;
	
	return constraint;
}

cpConstraint *
cpCustomConstraintNew(cpBody *a, cpBody *b)
{
	return (cpConstraint *)cpCustomConstraintInit(cpCustomConstraintAlloc(), a, b);
}
