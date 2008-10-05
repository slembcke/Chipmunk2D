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
cpBreakableJointPreStep(cpConstraint *constraint, cpFloat dt, cpFloat dt_inv)
{
	cpBreakableJoint *breakable = (cpBreakableJoint *)constraint;
	cpConstraint *child = breakable->child;

	if(child->klass->getImpulse(child)*breakable->last_dt_inv >= constraint->maxForce){
		cpSpaceRemoveConstraint(breakable->space, constraint);
		cpConstraintFree(constraint);
		
		return;
	}
	
	child->klass->preStep(child, dt, dt_inv);
	breakable->last_dt_inv = dt_inv;
}

static void
cpBreakableJointApplyImpulse(cpConstraint *constraint)
{
	cpConstraint *child = ((cpBreakableJoint *)constraint)->child;
	child->klass->applyImpulse(child);
}

static cpFloat
cpBreakableJointGetImpulse(cpConstraint *constraint)
{
	cpConstraint *child = ((cpBreakableJoint *)constraint)->child;
	return child->klass->getImpulse(child);
}

static const cpConstraintClass cpBreakableJointClass = {
	cpBreakableJointPreStep,
	cpBreakableJointApplyImpulse,
	cpBreakableJointGetImpulse,
};

cpBreakableJoint *
cpBreakableJointAlloc(void)
{
	return (cpBreakableJoint *)malloc(sizeof(cpBreakableJoint));
}

cpBreakableJoint *
cpBreakableJointInit(cpBreakableJoint *breakable, cpConstraint *child, struct cpSpace *space)
{
	cpConstraintInit((cpConstraint *)breakable, &cpBreakableJointClass, NULL, NULL);
	
	breakable->child = child;
	breakable->space = space;
	
	return breakable;
}

cpConstraint *
cpBreakableJointNew(cpConstraint *child, struct cpSpace *space)
{
	return (cpConstraint *)cpBreakableJointInit(cpBreakableJointAlloc(), child, space);
}
