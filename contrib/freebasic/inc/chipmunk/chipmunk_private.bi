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

#define CP_ALLOW_PRIVATE_ACCESS 1
#include "chipmunk.bi"

#define CP_HASH_COEF (3344921057ul)
#define CP_HASH_PAIR(A, B) ((cpHashValue)(A)*CP_HASH_COEF ^ (cpHashValue)(B)*CP_HASH_COEF)

type cpArray
	as integer		num, max
	as any ptr ptr	arr
end type

declare function cpArrayNew cdecl alias "cpArrayNew" ( byval as integer ) as cpArray ptr

declare sub cpArrayFree cdecl alias "cpArrayFree" ( byval as cpArray ptr )

declare sub cpArrayPush cdecl alias "cpArrayPush" ( byval as cpArray ptr, byval as any ptr )
declare function cpArrayPop cdecl alias "cpArrayPop" ( byval as cpArray ptr, byval as any ptr )
declare sub cpArrayDeleteObj cdecl alias "cpArrayDeleteObj" ( byval as cpArray ptr, byval as any ptr )
declare function cpArrayContains cdecl alias "cpArrayContains" ( byval as cpArray ptr, byval as any ptr )

declare sub cpArrayFreeEach cdecl alias "cpArrayFreeEach" ( byval as cpArray ptr, byval as sub( byval as any ptr ) )

function cpConstraintNext cdecl( byval node as cpConstraint ptr, byval body as cpBody ptr ) as cpConstraint ptr
	return iif( node->a = body, node->next_a, node->next_B )
end function

#macro CP_BODY_FOREACH_CONSTRAINT(bdy, v)
	dim v as cpConstraint ptr = bdy->constraintList
	do until not v
		v = cpConstraintNext( v, bdy )
#endmacro

function cpArbiterNext cdecl( byval node as cpArbiter ptr, byval body as cpBody ptr ) as cpArbiter ptr
	return iif( node->body_a, node->thread_a.next, node->thread_b.next )
end function

#macro CP_BODY_FOREACH_ARBITER(bdy, v)
	dim v as cpArbiter ptr = bdy->arbiterList
	do until not v
		v = cpArbiterNext( v, bdy )
#endmacro

#macro CP_BODY_FOREACH_SHAPE(bdy, v)
	dim v as cpShape ptr = bdy->shapeList
	do until not v
		v = v->_next
#endmacro

#macro CP_BODY_FOREACH_COMPONENT(root, v)
	dim v as cpBody ptr = root
	do until not v
		v = v->node._next
#endmacro

type cpHashSetEqlFunc as function cdecl( byval as any ptr, byval as any ptr ) as cpBool
type cpHashSetTransFunc as function cdecl( byval as any ptr, byval as any ptr ) as any ptr

declare function cpHashSetNew cdecl alias "cpHashSetNew" ( byval as integer, byval as cpHashSetEqlFunc ) as cpHashSet ptr
declare sub cpHashSetSetDefaultValue cdecl alias "cpHashSetSetDefaultValue" ( byval as cpHashSet ptr, byval as any ptr )

declare sub cpHashSetFree cdecl alias "cpHashSetFree" ( byval as cpHashSet ptr )

declare function cpHashSetCount cdecl alias "cpHashSetCount" ( byval as cpHashSet ptr ) as integer
declare function cpHashSetInsert cdecl alias "cpHashSetInsert" ( byval as cpHashSet ptr, byval cpHashValue, byval as any ptr, byval as any ptr, byval as cpHashSetTransFunc ) as any ptr
declare function cpHashSetRemove cdecl alias "cpHashSetRemove" ( byval as cpHashSet ptr, byval cpHashValue, byval as any ptr ) as any ptr
declare function cpHashSetFind cdecl alias "cpHashSetFind" ( byval as cpHashSet ptr, byval as cpHashValue, byval as any ptr ) as any ptr

type cpHashSetIteratorFunc as sub cdecl( byval as any ptr, byval as any ptr )
declare sub cpHashSetEach cdecl alias "cpHashSetEach" ( byval as cpHashSet ptr, byval cpHashSetIteratorFunc, byval as any ptr )

type  cpHashSetFilterFunc as function cdecl ( byval as any ptr, byval as any ptr ) as cpBool
declare sub cpHashSetFilter cdecl alias "cpHashSetFilter" ( byval as cpHashSet ptr, byval as cpHashSetFilterFunc, byval as any ptr )

declare sub cpBodyAddShape cdecl alias "cpBodyAddShape" ( byval as cpBody ptr, byval as cpShape ptr )
declare sub cpBodyRemoveShape cdecl alias "cpBodyRemoveShape" ( byval as cpBody ptr, byval as cpShape ptr )

declare function cpShapeInit cdecl alias "cpShapeInit" ( byval as cpShape ptr, byval as const cpShapeClass ptr, byval as cpBody ptr ) as cpShape ptr

function cpShapeActivate cdecl( byval shape as cpShape ptr ) as cpBool
	return shape->prev or ( shape->body->shapeList = shape )
end function

declare function cpCollideShape cdecl alias "cpCollideShapes" ( byval as const cpShape ptr, byval as const cpShape ptr, byval as cpContact ptr ) as integer

function cpPolyShapeValueOnAxis cdecl( byval poly as const cpPolyShape ptr, byval n as const cpVect, byval d as const cpFloat ) as cpFloat
	dim as cpVect ptr verts = poly->tVerts
	dim as cpFloat min = cpvdot( n, verts[0] )
	for i as integer = 1 to poly->numVerts-1
		min = cpfmin( min, cpvdot( n, verts[i] ) )
	next
	return min - d
end function

function cpPolyShapeContainsVert cdecl( byval poly as const cpPolyShape ptr, byval v as const cpVect ) as cpBool
	dim axes as cpPolyShapeAxis ptr = poly->tAxes
	for i as integer = 0 to poly->numVerts-1
		dim as cpFloat dist = cpvdot( axis[i].n, v ) - axes[i].d
		if dist > 0.0 then return cpFalse
	next
	return cpTrue
end function

function cpPolyShapeContainsVertParial cdecl( byval poly as const cpPolyShape ptr, byval v as const cpVect, byval n as const cpVect ) as cpBool
	dim axes as cpPolyShapeAxis ptr = poly->tAxes
	for i as integer = 0 to poly->numVerts-1
		if cpvdot(axes[i].n, n) < 0.0f continue
		dim as cpFloat dist = cpvdot( axis[i].n, v ) - axes[i].d
		if dist > 0.0 then return cpFalse
	next
	return cpTrue
end function


declare function cpSpatialIndexInit cdecl alias "cpSpatialIndexInit" ( byval as cpSpatialIndex ptr, byval as cpSpatialIndexClass ptr, byval as cpSpatialIndexBBFunc, byval as cpSpatialIndex ptr ) as cpSpatialIndex ptr

extern cpDefaultCollisionHandler alias "cpDefaultCollisionHandler" as cpCollisionHandler
declare sub cpSpaceProcessComponents cdecl alias "cpSpaceProcessComponents" ( byval as cpSpace ptr, byval as cpFloat )

declare function cpContactBufferGetArray cdecl alias "cpContactBufferGetArray" ( byval as cpSpace ptr ) as cpContact ptr
declare sub cpSpacePushContacts cdecl alias "cpSpacePushContacts" ( byval as cpSpace ptr, byval as integer )

declare function cpSpaceGetPostStepData cdecl alias "cpSpaceGetPostStepData" ( byval as cpSpace ptr, byval as any ptr ) as any ptr

declare sub cpSpaceFilterArbiters cdecl alias "cpSpaceFilterArbiters" ( byval as cpSpace ptr, byval as cpBody ptr, byval as cpShape ptr )

declare sub cpSpaceActivateBody cdecl alias "cpSpaceActivateBody" ( byval as cpSpace ptr, byval as cpBody ptr )
declare sub cpSpaceLock cdecl alias "cpSpaceLock" ( byval as cpSpace ptr )
declare sub cpSpaceUnlock cdecl alias "cpSpaceUnlock" ( byval as cpSpace ptr, byval as cpBool )

function cpSpaceLookupHandler cdecl( byval s as cpSpace ptr, byval a as cpCollisionType, byval b as cpCollisionType ) as cpCollisionHandler ptr
	dim as cpCollisionType types(0 To 1) = {a, b}
	return cptr( cpCollisionHandler ptr, cpHashSetFind( s->collisionHandlers, CP_HASH_PAIR( a, b ), @types(0) ) )
end function

sub cpSpaceUncacheArbiter cdecl( byval s as cpSpace ptr, byval arb as cpArbiter ptr )
	dim as cpShape ptr a = arb->a, b = arb->b
	dim as cpShape ptr shape_pair(0 To 1) = { a, b }
	dim as cpHashValue arbHashID = CP_HASH_PAIR( cint( a ), cint( b ) )
	cpHashSetRemove( s->cachedArbiters, arbHashID, @shape_pair(0) )
	cpArrayDeleteObj( s->arbiters, arb )
end sub

type cpContact
	as cpVect		p, n
	as cpFloat		dist
	as cpVect		r1, r2
	as cpFloat		nMass, tMass, bounce
	as cpFloat		jnAcc, jtAcc, jBias
	as cpFloat		bias
	as cpHashValue	hash
end type

declare function cpContactInit cdecl alias "cpContactInit" ( byval as cpContact ptr, byval as cpVect, byval as cpVect, byval as cpFloat, byval as cpHashValue ) as cpContact ptr
declare function cpArtbiterInit cdecl alias "cpArtbiterInit" ( byval as cpArbiter ptr, byval as cpShape ptr, byval as cpShape ptr ) as cpArbiter ptr

sub cpArbiterCallSeparate cdecl( byval arb as cpArbiter ptr, byval s as cpSpace ptr )
	'' The handler needs to be looked up again as the handler cached on the arbiter may have been deleted since the last step.
	dim as cpCollisionHandler ptr handler = cpSpaceLookupHandler( s, arb->a->collision_type, arb->b->collision_type )
	handler->separate( arb, s, handler->data )
end sub

function cpArbiterThreadForBody cdecl( byval arb as cpArbiter ptr, byval body as cpBody ptr ) as cpArbiterThread
	return iif( arb->body_a = body, @arb->thread_a, @arb->thread_b )
end function

declare sub cpArbiterUnthread cdecl alias "cpArbiterUnthread" ( byval as cpArbiter ptr )

declare sub cpArbiterUpdate cdecl alias "cpArbiterUpdate" ( byval as cpArbiter ptr, byval as cpContact ptr, byval as integer, byval as cpCollisionHandler ptr, byval as cpShape ptr, byval as cpShape ptr )
declare sub cpArbiterPreStep cdecl alias "cpArbiterPreStep" ( byval as cpArbiter ptr, byval as cpFloat, byval as cpFloat, byval as cpFloat )
declare sub cpArbiterApplyCachedImpulse cdecl alias "cpArbiterApplyCachedImpulse" ( byval as cpArbiter ptr, byval as cpFloat )
declare sub cpArbiterApplyImpulse cdecl alias "cpArbiterApplyImpulse" ( byval as cpArbiter ptr )
