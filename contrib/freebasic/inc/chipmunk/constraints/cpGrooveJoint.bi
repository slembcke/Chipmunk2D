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

extern "c"

''/ @defgroup cpGrooveJoint cpGrooveJoint

''/ @{



declare function cpGrooveJointGetClass() as const cpConstraintClass ptr



''/ @private

type cpGrooveJoint

	as cpConstraint constraint

	as cpVect grv_n, grv_a, grv_b

	as cpVect  anchr2

	

	as cpVect grv_tn

	as cpFloat clamp

	as cpVect r1, r2

	as cpMat2x2 k

	

	as cpVect jAcc

	as cpVect bias

end type : type as cpGrooveJoint cpGrooveJoint



''/ Allocate a groove joint.

declare function cpGrooveJointAlloc() as cpGrooveJoint ptr

''/ Initialize a groove joint.

declare function cpGrooveJointInit(byval joint as cpGrooveJoint ptr, byval a as cpBody ptr, byval b as cpBody ptr, byval groove_a as cpVect, byval groove_b as cpVect, byval anchr2 as cpVect) as cpGrooveJoint ptr

''/ Allocate and initialize a groove joint.

declare function cpGrooveJointNew(byval a as cpBody ptr, byval b as cpBody ptr, byval groove_a as cpVect, byval groove_b as cpVect, byval anchr2 as cpVect) as cpConstraint ptr



CP_DefineConstraintGetter(cpGrooveJoint, cpVect, grv_a, GrooveA)

''/ Set endpoint a of a groove joint's groove
declare sub cpGrooveJointSetGrooveA( byval constraint as cpConstrain ptr, byval value as cpVect )

CP_DefineConstraintGetter(cpGrooveJoint, cpVect, grv_b, GrooveB)

''/ Set endpoint b of a groove joint's groove
declare sub cpGrooveJointSetGrooveB( byval constraint as cpConstrain ptr, byval value as cpVect )

CP_DefineConstraintProperty(cpGrooveJoint, cpVect, anchr2, Anchr2)



''/ @}

end extern
