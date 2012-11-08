''
''	Physics Demo - Mouse Interaction
''
''		If you press the left click on the mouse, the program queries the mouse
''		position for any physics shapes, and makes a constraint between the
''		mouse static body and the body of the shape.  When you release the click,
''		the constraint is released, and the shape interacts with the simulation
''		normally again.
''
''		To compile:
''			fbc demo-mouse.bas
''

#include "chipmunk/chipmunk.bi"
#inclib "chipmunk"

#include "fbgfx.bi"
#include "fbgfx.bi"
#if __FB_LANG__ = "fb"
using fb
#endif

#ifndef NULL
#  define NULL cptr( any ptr, 0 )
#endif
#define GRABABLE_MASK_BIT (1 shl 31)
#define NOT_GRABABLE_MASK (not GRABABLE_MASK_BIT)

type physSpace
	space		as cpSpace ptr
	tickRate	as unsigned integer
	mutex		as any ptr
	thread		as any ptr
	state		as unsigned byte
	deltatime	as double
	currenttime	as double
	accumulator	as double
end type

declare sub physicTick( byval info as any ptr )

declare sub startPhysTick( byval ps as physSpace ptr, byval ticksPerSecond as unsigned integer, byval gravity as cpVect )
declare sub pausePhysTick( byval ps as physSpace ptr )
declare sub resumePhysTick( byval ps as physSpace ptr )
declare sub stopPhysTick( byval ps as physSpace ptr )
#define lockPhysics( ps ) mutexlock( cptr( physSpace ptr, ps )->mutex )
#define unlockPhysics( ps ) mutexunlock( cptr( physSpace ptr, ps )->mutex )

declare function handleEvents( byval ps as physSpace ptr, byref mbody as cpBody ptr, byref mJoint as cpConstraint ptr ) as integer

const as cpFloat mass = 10.0, radius = 10

screenres 800, 600, 16

cpInitChipmunk()

dim simulation as physSpace
dim circleBody as cpBody ptr
dim circleShape as cpShape ptr

dim mouseBody as cpBody ptr = cpBodyNew( INFINITY, INFINITY )
dim mouseConstraint as cpConstraint ptr = 0

startPhysTick( @simulation, 60, cpv( 0, 50 ) )

circleBody = cpBodyNew( mass, cpMomentForCircle( mass, 0.0, radius, cpvzero ) )
circleBody->p = cpv( 400, 0 )
cpSpaceAddBody( simulation.space, circleBody )

circleShape = cpCircleShapeNew( circleBody, radius, cpvzero )
cpSpaceAddShape( simulation.space, circleShape )

resumePhysTick( @simulation )
dim killsimulation as integer = 0
do
	lockPhysics( @simulation )
		screenlock()
			cls
			circle( circleBody->p.x, circleBody->p.y ), radius, rgb( 255, 0, 0 )
		screenunlock()
		killsimulation = handleEvents( @simulation, mouseBody, mouseConstraint )
	unlockPhysics( @simulation )
	sleep 1, 1
loop until killsimulation

stopPhysTick( @simulation )

end 0

sub physicTick( byval info as any ptr )
	dim ps as physSpace ptr = cptr( physSpace ptr, info )
	dim quitThread as integer = 0

	do
		lockPhysics( ps )
			if ps->state = 0 then
				var newTime = Timer()
				var ftime = newTime - ps->currentTime
				ps->accumulator += ftime
				while ps->accumulator >= ps->deltatime
					cpSpaceStep( ps->space, ps->deltatime )
					ps->accumulator -= ps->deltaTime
				wend
				ps->currentTime = newTime
			end if
			if ps->state = 2 then quitThread = 1
		unlockPhysics( ps )
		sleep 1, 1
	loop until quitThread	
end sub

sub startPhysTick( byval ps as physSpace ptr, byval ticksPerSecond as unsigned integer, byval gravity as cpVect )
	ps->space = cpSpaceNew()
	ps->space->iterations = 10
	ps->space->gravity = gravity
	ps->tickRate = ticksPerSecond
	ps->state = 1
	ps->currenttime = Timer
	ps->accumulator = 0.0
	ps->deltatime = 1.0 / csng( ps->tickRate )
	ps->mutex = mutexcreate()
	ps->thread = threadcreate( @physicTick, ps )
end sub

sub pausePhysTick( byval ps as physSpace ptr )
	lockPhysics( ps )
		ps->state = 1
	unlockPhysics( ps )
end sub

sub resumePhysTick( byval ps as physSpace ptr )
	lockPhysics( ps )
		ps->state = 0
	unlockPhysics( ps )
end sub

sub stopPhysTick( byval ps as physSpace ptr )
	lockPhysics( ps )
		ps->state = 2
	unlockPhysics( ps )
	threadwait( ps->thread )
	mutexdestroy( ps->mutex )
	cpSpaceFreeChildren( ps->space )
	cpSpaceFree( ps->space )
end sub

function handleEvents( byval ps as physSpace ptr, byref mbody as cpBody ptr, byref mJoint as cpConstraint ptr ) as integer
	dim e as Event
	if ScreenEvent( @e ) then
		select case e.type
		case EVENT_MOUSE_EXIT
			' Remove any joint between the mouse and a shape if the mouse leaves the window
			if mJoint <> 0 then
				cpSpaceRemoveConstraint( ps->space, mJoint )
				cpConstraintFree( mJoint )
				mJoint = NULL
			end if

		case EVENT_MOUSE_MOVE
			' Update the mouse location
			mBody->p = cpv( e.x, e.y )
        
		case EVENT_MOUSE_BUTTON_PRESS
			if e.button = BUTTON_LEFT then
				var shape = cpSpacePointQueryFirst( ps->space, mBody->p, GRABABLE_MASK_BIT, 0 )
				if shape then
					' If you click on a shape, then attach it to the mouse
					var body = shape->body
					mJoint = cpPivotJointNew2( mBody, body, cpvzero, cpBodyWorld2Local( body, mBody->p ) )
					mJoint->maxForce = INFINITY
					mJoint->biasCoef = 0.5
					cpSpaceAddConstraint( ps->space, mJoint )
				end if
			end if

		case EVENT_MOUSE_BUTTON_RELEASE
			if e.button = BUTTON_LEFT then
				if mJoint <> 0 then
					' Release the any attached shape from the mouse
					cpSpaceRemoveConstraint( ps->space, mJoint )
					cpConstraintFree( mJoint )
					mJoint = NULL
				end if
			end if

		case EVENT_KEY_RELEASE
			if e.scancode = SC_ESCAPE then return 1 ' Send kill signal

		end select
	end if
	return 0
end function

