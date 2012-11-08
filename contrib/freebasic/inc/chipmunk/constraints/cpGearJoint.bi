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



''/ @defgroup cpGearJoint cpGearJoint

''/ @{

extern "c"

declare function cpGearJointGetClass() as const cpConstraintClass ptr



''/ @private

type cpGearJoint

	as cpConstraint constraint

	as cpFloat phase, ratio

	as cpFloat ratio_inv

	

	as cpFloat iSum

	as cpFloat bias

	as cpFloat jAcc

end type



''/ Allocate a gear joint.

declare function cpGearJointAlloc() as cpGearJoint ptr

''/ Initialize a gear joint.

declare function cpGearJointInit(byval joint as cpGearJoint ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval phase as cpFloat, byval ratio as cpFloat) as cpGearJoint ptr

''/ Allocate and initialize a gear joint.

declare function cpGearJointNew(byval a as cpBody ptr, byval b as cpBody ptr, byval phase as cpFloat, byval ratio as cpFloat) as cpConstraint ptr



CP_DefineConstraintProperty(cpGearJoint, cpFloat, phase, Phase)
CP_DefineConstraintGetter(cpGearJoint, cpFloat, ratio, Ratio)
''/ Set the ratio of a gear joint.
declare sub cpGearJointSetRatio( byval constraint as cpConstraint ptr, byval value as cpFloat )


''/ @}

end extern
