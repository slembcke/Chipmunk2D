/' Copyright (c) 2007 Scott Lembcke
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
 '/

#ifndef CHIPMUNK_SLIDEJOINT
#define CHIPMUNK_SLIDEJOINT

extern "c"

''/ @defgroup cpSlideJoint cpSlideJoint

''/ @{



declare function cpSlideJointGetClass() as const cpConstraintClass ptr



''/ @private

type cpSlideJoint

	as cpConstraint constraint

	as cpVect anchr1, anchr2

	as cpFloat min, max

	

	as cpVect r1, r2

	as cpVect n

	as cpFloat nMass

	

	as cpFloat jnAcc

	as cpFloat bias

end type



''/ Allocate a slide joint.

declare function cpSlideJointAlloc() as cpSlideJoint ptr

''/ Initialize a slide joint.

declare function cpSlideJointInit(byval joint as cpSlideJoint ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect, byval min as cpFloat, byval max as cpFloat) as cpSlideJoint ptr

''/ Allocate and initialize a slide joint.

declare function cpSlideJointNew(byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect, byval min as cpFloat, byval max as cpFloat) as cpConstraint ptr

CP_DefineConstraintProperty(cpSlideJoint, cpVect, anchr1, Anchr1)
CP_DefineConstraintProperty(cpSlideJoint, cpVect, anchr2, Anchr2)
CP_DefineConstraintProperty(cpSlideJoint, cpFloat, min, Min)
CP_DefineConstraintProperty(cpSlideJoint, cpFloat, max, Max)



''/ @}

end extern

#endif