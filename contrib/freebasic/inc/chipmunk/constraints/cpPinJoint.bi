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

 

''/ @defgroup cpPinJoint cpPinJoint

''/ @{

extern "c"

declare function cpPinJointGetClass() as const cpConstraintClass ptr



''/ @private

type cpPinJoint

	as cpConstraint constraint

	as cpVect anchr1, anchr2

	as cpFloat dist

	

	as cpVect r1, r2

	as cpVect n

	as cpFloat nMass

	

	as cpFloat jnAcc

	as cpFloat bias

end type : type as cpPinJoint cpPinJoint



''/ Allocate a pin joint.

declare function cpPinJointAlloc() as cpPinJoint ptr

''/ Initialize a pin joint.

declare function cpPinJointInit(byval joint as cpPinJoint ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect) as cpPinJoint ptr

''/ Allocate and initialize a pin joint.

declare function cpPinJointNew(byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect) as cpConstraint ptr



'' TODO: translate (sorry)
CP_DefineConstraintProperty(cpPinJoint, cpVect, anchr1, Anchr1)

CP_DefineConstraintProperty(cpPinJoint, cpVect, anchr2, Anchr2)

CP_DefineConstraintProperty(cpPinJoint, cpFloat, dist, Dist)



''/@}

end extern
