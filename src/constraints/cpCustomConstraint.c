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
preStep(cpCustomConstraint *custom, cpFloat dt)
{
	cpBody *a = custom->constraint.a;
	cpBody *b = custom->constraint.b;
	
	cpVect r1 = custom->r1 = cpvrotate(custom->anchr1, a->rot);
	cpVect r2 = custom->r2 = cpvrotate(custom->anchr2, b->rot);
	
	// Calculate mass tensor
	custom->k = k_tensor(a, b, r1, r2);
	
	// calculate bias velocity
	cpVect delta = cpvsub(cpvadd(b->p, r2), cpvadd(a->p, r1));
	custom->bias = cpvclamp(cpvmult(delta, -bias_coef(custom->constraint.errorBias, dt)/dt), custom->constraint.maxBias);
}

static void
applyCachedImpulse(cpCustomConstraint *custom, cpFloat dt_coef)
{
	cpBody *a = custom->constraint.a;
	cpBody *b = custom->constraint.b;
	
	apply_impulses(a, b, custom->r1, custom->r2, cpvmult(custom->jAcc, dt_coef));
}

static cpVect
CLAMP(cpConstraint *custom, cpVect j, cpFloat dt)
{
	return cpvclamp(j, cpConstraintGetMaxForce(custom)*dt);
}

static void
applyImpulse(cpCustomConstraint *custom, cpFloat dt)
{
	cpBody *a = custom->constraint.a;
	cpBody *b = custom->constraint.b;
	
	cpVect r1 = custom->r1;
	cpVect r2 = custom->r2;
		
	// compute relative velocity
	cpVect vr = relative_velocity(a, b, r1, r2);
	
	// compute normal impulse
	cpVect j = cpMat2x2Transform(custom->k, cpvsub(custom->bias, vr));
	cpVect jOld = custom->jAcc;
	custom->jAcc = CLAMP((cpConstraint *)custom, cpvadd(custom->jAcc, j), dt);
	j = cpvsub(custom->jAcc, jOld);
	
	// apply impulse
	apply_impulses(a, b, r1, r2, j);
}

static cpFloat
getImpulse(cpConstraint *custom)
{
	return cpvlength(((cpCustomConstraint *)custom)->jAcc);
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
cpCustomConstraintInit(cpCustomConstraint *custom, cpBody *a, cpBody *b)
{
	cpConstraintInit((cpConstraint *)custom, &klass, a, b);
	
	custom->jAcc = cpvzero;
	
	return custom;
}

cpConstraint *
cpCustomConstraintNew(cpBody *a, cpBody *b)
{
	return (cpConstraint *)cpCustomConstraintInit(cpCustomConstraintAlloc(), a, b);
}
