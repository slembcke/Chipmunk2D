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
#include <math.h>

#include "chipmunk.h"
#include "util.h"

//void
//cpDampedSpring(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat rlen, cpFloat k, cpFloat dmp, cpFloat dt)
//{
//	// Calculate the world space anchor coordinates.
//	cpVect r1 = cpvrotate(anchr1, a->rot);
//	cpVect r2 = cpvrotate(anchr2, b->rot);
//	
//	cpVect delta = cpvsub(cpvadd(b->p, r2), cpvadd(a->p, r1));
//	cpFloat dist = cpvlength(delta);
//	cpVect n = dist ? cpvmult(delta, 1.0f/dist) : cpvzero;
//	
//	cpFloat f_spring = (dist - rlen)*k;
//
//	// Calculate the world relative velocities of the anchor points.
//	cpVect v1 = cpvadd(a->v, cpvmult(cpvperp(r1), a->w));
//	cpVect v2 = cpvadd(b->v, cpvmult(cpvperp(r2), b->w));
//	
//	// Calculate the damping force.
//	// This really should be in the impulse solver and can produce problems when using large damping values.
//	cpFloat vrn = cpvdot(cpvsub(v2, v1), n);
//	cpFloat f_damp = vrn*cpfmin(dmp, 1.0f/(dt*(a->m_inv + b->m_inv)));
//	
//	// Apply!
//	cpVect f = cpvmult(n, f_spring + f_damp);
//	cpBodyApplyForce(a, f, r1);
//	cpBodyApplyForce(b, cpvneg(f), r2);
//}

static void
DampedSpringPreStep(cpConstraint *constraint, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = constraint->a;
	cpBody *b = constraint->b;
	cpDampedSpring *spring = (cpDampedSpring *)constraint;
	
	spring->r1 = cpvrotate(spring->anchr1, a->rot);
	spring->r2 = cpvrotate(spring->anchr2, b->rot);
	
	cpVect delta = cpvsub(cpvadd(b->p, spring->r2), cpvadd(a->p, spring->r1));
	cpFloat dist = cpvlength(delta);
	spring->n = cpvmult(delta, 1.0f/(dist ? dist : INFINITY));
	
	// calculate mass normal
	spring->nMass = 1.0f/k_scalar(a, b, spring->r1, spring->r2, spring->n);

	spring->dt = dt;
	spring->target_vrn = 0.0f;

	// apply spring force
	cpFloat f_spring = (spring->restLength - dist)*spring->stiffness;
	apply_impulses(a, b, spring->r1, spring->r2, cpvmult(spring->n, f_spring*dt));
}

static void
DampedSpringApplyImpulse(cpConstraint *constraint)
{
	cpBody *a = constraint->a;
	cpBody *b = constraint->b;
	
	cpDampedSpring *spring = (cpDampedSpring *)constraint;
	cpVect n = spring->n;
	cpVect r1 = spring->r1;
	cpVect r2 = spring->r2;

	// compute relative velocity
	cpFloat vrn = normal_relative_velocity(a, b, r1, r2, n) - spring->target_vrn;
	
	// compute normal impulse
	cpFloat v_damp = -vrn*(1.0f - exp(-spring->damping*spring->dt/spring->nMass));
	cpFloat j = v_damp*spring->nMass;
	spring->target_vrn = vrn + v_damp;
	
	apply_impulses(a, b, spring->r1, spring->r2, cpvmult(spring->n, j));
}

static const cpConstraintClass DampedSpringClass = {
	DampedSpringPreStep,
	DampedSpringApplyImpulse,
};

cpDampedSpring *
cpDampedSpringAlloc(void)
{
	return (cpDampedSpring *)malloc(sizeof(cpDampedSpring));
}

cpDampedSpring *
cpDampedSpringInit(cpDampedSpring *spring, cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat restLength, cpFloat stiffness, cpFloat damping)
{
	cpConstraintInit((cpConstraint *)spring, &DampedSpringClass, a, b);
	
	spring->anchr1 = anchr1;
	spring->anchr2 = anchr2;
	
	spring->restLength = restLength;
	spring->stiffness = stiffness;
	spring->damping = damping;
	
	return spring;
}

cpConstraint *
cpDampedSpringNew(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat restLength, cpFloat stiffness, cpFloat damping)
{
	return (cpConstraint *)cpDampedSpringInit(cpDampedSpringAlloc(), a, b, anchr1, anchr2, restLength, stiffness, damping);
}
