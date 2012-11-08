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



''/ @defgroup cpDampedSpring cpDampedSpring

''/ @{

extern "c"

type as cpDampedSpring cpDampedSpring_



type as function(byval spring as cpConstraint ptr, byval dist as cpFloat) as cpFloat cpDampedSpringForceFunc



declare function cpDampedSpringGetClass() as const cpConstraintClass ptr



''/ @private

type cpDampedSpring

	as cpConstraint constraint

	as cpVect anchr1, anchr2

	as cpFloat restLength

	as cpFloat stiffness

	as cpFloat damping

	as cpDampedSpringForceFunc springForceFunc

	

	as cpFloat target_vrn

	as cpFloat v_coef

	

	as cpVect r1, r2

	as cpFloat nMass

	as cpVect n

end type



''/ Allocate a damped spring.

declare function cpDampedSpringAlloc() as cpDampedSpring ptr

''/ Initialize a damped spring.

declare function cpDampedSpringInit(byval joint as cpDampedSpring ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect, byval restLength as cpFloat, byval stiffness as cpFloat, byval damping as cpFloat) as cpDampedSpring ptr

''/ Allocate and initialize a damped spring.

declare function cpDampedSpringNew(byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect, byval restLength as cpFloat, byval stiffness as cpFloat, byval damping as cpFloat) as cpConstraint ptr



CP_DefineConstraintProperty(cpDampedSpring, cpVect, anchr1, Anchr1)

CP_DefineConstraintProperty(cpDampedSpring, cpVect, anchr2, Anchr2)

CP_DefineConstraintProperty(cpDampedSpring, cpFloat, restLength, RestLength)

CP_DefineConstraintProperty(cpDampedSpring, cpFloat, stiffness, Stiffness)

CP_DefineConstraintProperty(cpDampedSpring, cpFloat, damping, Damping)

CP_DefineConstraintProperty(cpDampedSpring, cpDampedSpringForceFunc, springForceFunc, SpringForceFunc)



''/ @}

end extern
