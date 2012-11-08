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



''/ @defgroup cpPivotJoint cpPivotJoint

''/ @{

extern "c"

declare function cpPivotJointGetClass() as const cpConstraintClass ptr



''/ @private

type cpPivotJoint

	as cpConstraint constraint

	as cpVect anchr1, anchr2

	

	as cpVect r1, r2

	as cpMat2x2 k

	

	as cpVect jAcc

	as cpVect bias

end type



''/ Allocate a pivot joint

declare function cpPivotJointAlloc() as cpPivotJoint ptr

''/ Initialize a pivot joint.

declare function cpPivotJointInit(byval joint as cpPivotJoint ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect) as cpPivotJoint ptr

''/ Allocate and initialize a pivot joint.

declare function cpPivotJointNew(byval a as cpBody ptr, byval b as cpBody ptr, byval pivot as cpVect) as cpConstraint ptr

''/ Allocate and initialize a pivot joint with specific anchors.

declare function cpPivotJointNew2(byval a as cpBody ptr, byval b as cpBody ptr, byval anchr1 as cpVect, byval anchr2 as cpVect) as cpConstraint ptr



CP_DefineConstraintProperty(cpPivotJoint, cpVect, anchr1, Anchr1)

CP_DefineConstraintProperty(cpPivotJoint, cpVect, anchr2, Anchr2)



''/ @}

end extern
