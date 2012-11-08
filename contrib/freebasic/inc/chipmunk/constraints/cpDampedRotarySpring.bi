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



''/ @defgroup cpDampedRotarySpring cpDampedRotarySpring

''/ @{

extern "c"

type as function(byval spring as cpConstraint ptr, byval relativeAngle as cpFloat) as cpFloat cpDampedRotarySpringTorqueFunc



declare function cpDampedRotarySpringGetClass() as const cpConstraintClass ptr



''/ @private

type cpDampedRotarySpring

	as cpConstraint constraint

	as cpFloat restAngle

	as cpFloat stiffness

	as cpFloat damping

	as cpDampedRotarySpringTorqueFunc springTorqueFunc

	

	as cpFloat target_wrn

	as cpFloat w_coef

	

	as cpFloat iSum

end type



''/ Allocate a damped rotary spring.

declare function cpDampedRotarySpringAlloc() as cpDampedRotarySpring ptr

''/ Initialize a damped rotary spring.

declare function cpDampedRotarySpringInit(byval joint as cpDampedRotarySpring ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval restAngle as cpFloat, byval stiffness as cpFloat, byval damping as cpFloat) as cpDampedRotarySpring ptr

''/ Allocate and initialize a damped rotary spring.

declare function cpDampedRotarySpringNew(byval a as cpBody ptr, byval b as cpBody ptr, byval restAngle as cpFloat, byval stiffness as cpFloat, byval damping as cpFloat) as cpConstraint ptr



CP_DefineConstraintProperty(cpDampedRotarySpring, cpFloat, restAngle, RestAngle)
CP_DefineConstraintProperty(cpDampedRotarySpring, cpFloat, stiffness, Stiffness)
CP_DefineConstraintProperty(cpDampedRotarySpring, cpFloat, damping, Damping)
CP_DefineConstraintProperty(cpDampedRotarySpring, cpDampedRotarySpringTorqueFunc, springTorqueFunc, SpringTorqueFunc)



''/ @}

end extern
