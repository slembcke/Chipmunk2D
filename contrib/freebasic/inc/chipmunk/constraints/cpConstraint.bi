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

#ifndef CHIPMUNK_CONSTRAINT
#define CHIPMUNK_CONSTRAINT	1

extern "c"

''/ @defgroup cpConstraint cpConstraint

''/ @{



type as cpConstraintClass cpConstraintClass



type as sub(byval constraint as cpConstraint ptr, byval dt as cpFloat) cpConstraintPreStepImpl

type as sub(byval constraint as cpConstraint ptr, byval dt_coef as cpFloat) cpConstraintApplyCachedImpulseImpl

type as sub(byval constraint as cpConstraint ptr, byval dt as cpFloat) cpConstraintApplyImpulseImpl

type as function(byval constraint as cpConstraint ptr) as cpFloat cpConstraintGetImpulseImpl



''/ @private

type cpConstraintClass

	as cpConstraintPreStepImpl preStep

	as cpConstraintApplyCachedImpulseImpl applyCachedImpulse

	as cpConstraintApplyImpulseImpl applyImpulse

	as cpConstraintGetImpulseImpl getImpulse

end type



''/ Callback function type that gets called before solving a joint.

type as sub(byval constraint as cpConstraint ptr, byval space as cpSpace ptr) cpConstraintPreSolveFunc

''/ Callback function type that gets called after solving a joint.

type as sub(byval constraint as cpConstraint ptr, byval space as cpSpace ptr) cpConstraintPostSolveFunc





''/ Opaque cpConstraint struct.

type cpConstraint

	CP_PRIVATE(as const cpConstraintClass ptr klass)

	

	''/ The first body connected to this constraint.

	as cpBody ptr a

	''/ The second body connected to this constraint.

	as cpBody ptr b

	

	CP_PRIVATE(as cpSpace ptr space)

	CP_PRIVATE(as cpConstraint ptr next_a)

	CP_PRIVATE(as cpConstraint ptr next_b)

	

	''/ The maximum force that this constraint is allowed to use.

	''/ Defaults to infinity.

	as cpFloat maxForce

	''/ The rate at which joint error is corrected.

	''/ Defaults to pow(1.0 - 0.1, 60.0) meaning that it will

	''/ correct 10% of the error every 1/60th of a second.

	as cpFloat errorBias

	''/ The maximum rate at which joint error is corrected.

	''/ Defaults to infinity.

	as cpFloat maxBias

	

	''/ Function called before the solver runs.

	''/ Animate your joint anchors, update your motor torque, etc.

	as cpConstraintPreSolveFunc preSolve

	

	''/ Function called after the solver runs.

	''/ Use the applied impulse to perform effects like breakable joints.

	as cpConstraintPostSolveFunc postSolve

	

	''/ User definable data pointer.

	''/ Generally this points to your the game object class so you can access it

	''/ when given a cpConstraint reference in a callback.

	as cpDataPointer data

end type



''/ Destroy a constraint.

declare sub cpConstraintDestroy(byval constraint as cpConstraint ptr)

''/ Destroy and free a constraint.

declare sub cpConstraintFree(byval constraint as cpConstraint ptr)



''/ @private

#ifndef cpConstraintActiveBodies
#macro cpCOnstraintActiveBodies( constraint )
	if ( constraint->a <> NULL ) then cpBodyActivate( constraint->a )
	if ( constraint->b <> NULL ) then cpBodyActivate( constraint->b )
#endmacro
#endif



''/ @private
#ifndef CP_DefineConstraintStructGetter
#macro CP_DefineConstraintStructGetter( _type, _member, _name )
function cpConstraintGet##_name( byval constraint as const cpConstraint ptr ) as _type
	return constraint->(_member)
end function
#endmacro
#endif


''/ @private
#ifndef CP_DefineConstraintStructSetter
#macro CP_DefineConstraintStructSetter( _type, _member, _name )
sub cpConstraintSet##_name( byval constraint as cpConstraint ptr, byval value as _type )
	cpConstraintActivateBodies( constraint )
	constraint->(_member) = value
end sub
#endmacro
#endif


''/ @private
#ifndef CP_DefineConstraintStructProperty
#macro CP_DefineConstraintStructProperty( _type, _member, _name )
CP_DefineConstraintGetter( _type, _member, _name )
CP_DefineConstraintSetter( _type, _member, _name )
#endmacro
#endif


CP_DefineConstraintStructGetter(cpSpace ptr, CP_PRIVATE(space), Space)

CP_DefineConstraintStructGetter(cpBody ptr, a, A)

CP_DefineConstraintStructGetter(cpBody ptr, b, B)

CP_DefineConstraintStructProperty(cpFloat, maxForce, MaxForce)

CP_DefineConstraintStructProperty(cpFloat, errorBias, ErrorBias)

CP_DefineConstraintStructProperty(cpFloat, maxBias, MaxBias)

CP_DefineConstraintStructProperty(cpConstraintPreSolveFunc, preSolve, PreSolveFunc)

CP_DefineConstraintStructProperty(cpConstraintPostSolveFunc, postSolve, PostSolveFunc)

CP_DefineConstraintStructProperty(cpDataPointer, data, UserData)



'' Get the last impulse applied by this constraint.
#ifndef cpConstraintGetImpulse
#define cpConstraintGetImpulse( constraint )	constraint->CP_PRIVATE(klass)->getImpulse( constraint )
#endif


''/ @}



#define cpConstraintCheckCast( _constraint, _struct) cpAssertHard( (_constraint)->CP_PRIVATE(klass) = _struct##GetClass(), "Constraint is not a "#_struct)


#ifndef CP_DefineConstraintGetter
#macro CP_DefineConstraintGetter( _struct, _type, _member, _name )
function _struct##Get##_name( byval constraint as const cpConstraint ptr ) as _type
	cpConstraintCheckCast( constraint, _struct )
	return cptr( _struct ptr, constraint )->(_member)
end function
#endmacro
#endif


#ifndef CP_DefineConstraintSetter
#macro CP_DefineConstraintSetter( _struct, _type, _member, _name )
sub _struct##Set##name( byval constraint as cpConstraint ptr, byval value as _type )
	cpConstraintCheckCast( constraint, _struct )
	cpConstraintActivateBodies( constraint )
	
	cptr( _struct ptr, constraint)->(_member) = value
end sub
#endmacro
#endif

#ifndef CP_DefineConstraintProperty
#macro CP_DefineConstraintProperty( _struct, _type, _member, _name )
CP_DefineConstraintGetter( _struct, _type, _member, _name )
CP_DefineConstraintSetter( _struct, _type, _member, _name )
#endmacro
#endif


#include "cpPinJoint.h"
#include "cpSlideJoint.h"
#include "cpPivotJoint.h"
#include "cpGrooveJoint.h"
#include "cpDampedSpring.h"
#include "cpDampedRotarySpring.h"
#include "cpRotaryLimitJoint.h"
#include "cpRatchetJoint.h"
#include "cpGearJoint.h"
#include "cpSimpleMotor.h"

end extern

#endif