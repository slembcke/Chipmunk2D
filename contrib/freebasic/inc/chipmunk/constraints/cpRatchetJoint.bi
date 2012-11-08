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



''/ @defgroup cpRatchetJoint cpRatchetJoint

''/ @{

extern "c"

declare function cpRatchetJointGetClass() as const cpConstraintClass ptr



''/ @private

type cpRatchetJoint

	as cpConstraint constraint

	as cpFloat angle, phase, ratchet

	

	as cpFloat iSum

	as cpFloat bias

	as cpFloat jAcc

end type : type as cpRatchetJoint cpRatchetJoint



''/ Allocate a ratchet joint.

declare function cpRatchetJointAlloc() as cpRatchetJoint ptr

''/ Initialize a ratched joint.

declare function cpRatchetJointInit(byval joint as cpRatchetJoint ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval phase as cpFloat, byval ratchet as cpFloat) as cpRatchetJoint ptr

''/ Allocate and initialize a ratchet joint.

declare function cpRatchetJointNew(byval a as cpBody ptr, byval b as cpBody ptr, byval phase as cpFloat, byval ratchet as cpFloat) as cpConstraint ptr



'' TODO: translate (sorry)
CP_DefineConstraintProperty(cpRatchetJoint, cpFloat, angle, Angle)

CP_DefineConstraintProperty(cpRatchetJoint, cpFloat, phase, Phase)

CP_DefineConstraintProperty(cpRatchetJoint, cpFloat, ratchet, Ratchet)



''/ @}

end extern
