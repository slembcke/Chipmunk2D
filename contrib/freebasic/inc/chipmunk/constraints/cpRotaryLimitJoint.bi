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



''/ @defgroup cpRotaryLimitJoint cpRotaryLimitJoint

''/ @{

extern "c"

declare function cpRotaryLimitJointGetClass() as const cpConstraintClass ptr



''/ @private

type cpRotaryLimitJoint

	as cpConstraint constraint

	as cpFloat min, max

	

	as cpFloat iSum

		

	as cpFloat bias

	as cpFloat jAcc

end type



''/ Allocate a damped rotary limit joint.

declare function cpRotaryLimitJointAlloc() as cpRotaryLimitJoint ptr

''/ Initialize a damped rotary limit joint.

declare function cpRotaryLimitJointInit(byval joint as cpRotaryLimitJoint ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval min as cpFloat, byval max as cpFloat) as cpRotaryLimitJoint ptr

''/ Allocate and initialize a damped rotary limit joint.

declare function cpRotaryLimitJointNew(byval a as cpBody ptr, byval b as cpBody ptr, byval min as cpFloat, byval max as cpFloat) as cpConstraint ptr



CP_DefineConstraintProperty(cpRotaryLimitJoint, cpFloat, min, Min)

CP_DefineConstraintProperty(cpRotaryLimitJoint, cpFloat, max, Max)



''/ @}

end extern
