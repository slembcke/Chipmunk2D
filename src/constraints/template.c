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
NAMEStep(cpConstraint *constraint, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = constraint->a;
	cpBody *b = constraint->b;
	NAME *LOCAL_NAME = (NAME *)constraint;
}

static void
pinJointApplyImpulse(cpConstraint *constraint)
{
	cpBody *a = constraint->a;
	cpBody *b = constraint->b;
	
	NAME *LOCAL_NAME = (NAME *)constraint;
}

static const cpConstraintClass pinJointClass = {
	pinJointPreStep,
	pinJointApplyImpulse,
};

NAME *
NAMEAlloc(void)
{
	return (NAME *)malloc(sizeof(NAME));
}

NAME *
NAMEInit(NAME *joint, cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2)
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
NAMENew(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2)
{
	return (cpConstraint *)NAMEInit(NAMEAlloc(), a, b, anchr1, anchr2);
}
