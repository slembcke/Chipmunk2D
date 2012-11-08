''
''	Physics Demo - Basic Physics Without Threads
''
''		This demo runs identically to demo.bas, but it runs without
''		threads.  This approach has it's problems, but it is effective
''		if you want to avoid the hassles of threading.
''
''		To compile:
''			fbc demo-nothread.bas
''

#include "chipmunk/chipmunk.bi"
#inclib "chipmunk"

#include "fbgfx.bi"
#if __FB_LANG__ = "fb"
using fb
#endif

'' I kidnapped these from ChipmunkDemo.h from <chipmunksvn>/Demos/
#define GRABABLE_MASK_BIT (1 shl 31)
#define NOT_GRABABLE_MASK (not GRABABLE_MASK_BIT)

' This is not necessary in a chipmunk program, but I wanted to do this to make it more readible
type physicsInstance
	declare constructor( byval Hz as integer, byval grav as cpVect )
	declare destructor()
	declare sub simulate()
	
	as cpFloat		deltaTime
	as double		currentTime
	as double		accumulator
	as cpSpace ptr	space
	as ubyte		pause
end type

dim shared as cpBody ptr		mouseBody = NULL
dim shared as cpConstraint ptr	mouseConstraint = NULL

declare function handleEvents( byval physics as physicsInstance ptr ) as integer

function main( byval argc as integer, byval argv as zstring ptr ptr ) as integer
	dim i as integer

	screenres 800, 600, 32
	screenset 0, 1
	Window ( 0, 600 ) - ( 800, 0 )						' Chipmunk flips the y axis (as most game libraries do), so I have flipped it
	
	randomize timer
	
	cpInitChipmunk()									' Initialize the chipmunk engine
	
	dim instance as physicsInstance = physicsInstance( 60, cpv(0, -100) )
	instance.space->iterations = 10
	cpSpaceResizeStaticHash( instance.space, 30.0, 1000 )
	cpSpaceResizeActiveHash( instance.space, 30.0, 1000 )
	WindowTitle "Chipmunk Physics Engine v" & cpVersion()
	
	mouseBody = cpBodyNew( INFINITY, INFINITY )
	mouseConstraint = NULL
	
	' Create a simulation
	dim as cpShape ptr ground(0 to 3)
	ground(0) = cpSegmentShapeNew( NULL, cpv( 10, 10 ), cpv( 790, 10 ), 1 )
	ground(1) = cpSegmentShapeNew( NULL, cpv( 790, 10 ), cpv( 790, 590 ), 1 )
	ground(2) = cpSegmentShapeNew( NULL, cpv( 790, 590 ), cpv( 10, 590 ), 1 )
	ground(3) = cpSegmentShapeNew( NULL, cpv( 10, 590 ), cpv( 10, 10 ), 1 )
	for i = 0 to 3
		ground(i)->e = 1.0
		ground(i)->u = 10.0
		ground(i)->layers = NOT_GRABABLE_MASK
		cpSpaceAddStaticShape( instance.space, ground( i ) )
	next i
	
	dim as cpShape ptr Sbox
	dim verts(0 to 3) as cpVect = { cpv( 0, 10 ), cpv( 600, 10 ), cpv( 600, 0 ), cpv( 200, 0 ) }
	Sbox = cpPolyShapeNew( NULL, 4, @verts(0), cpv( 100, 200 ) )
	Sbox->e = 0.5
	Sbox->u = 5.0
	Sbox->layers = NOT_GRABABLE_MASK
	cpSpaceAddStaticShape( instance.space, Sbox )
	
	dim nball as integer = 50
	dim ballb(0 to nball - 1) as cpBody ptr, balls(0 to nball - 1) as cpShape ptr
	
	var radius = 15.0
	var mass = 10.0
	
	for i = 0 to nball - 1
		ballb(i) = cpBodyNew( mass, cpMomentForCircle( mass, 0.0, radius, cpvzero ) )
		ballb(i)->p = cpv( int(rnd * 700)+50, int(rnd * 450)+ 50 )
		cpSpaceAddBody( instance.space, ballb(i) )
		
		balls(i) = cpCircleShapeNew( ballb(i), radius, cpvzero )
		balls(i)->e = 1.0
		balls(i)->u = 1.0
		cpSpaceAddShape( instance.space, balls(i) )
	next i
	
	dim iskilled as ubyte = 0
	
	do
		instance.simulate()
		screenlock()
			cls
			for i = 0 to 3
				var p1 = cptr( cpSegmentShape ptr, ground(i) )->ta, p2 = cptr( cpSegmentShape ptr, ground(i) )->tb
				line( p1.x, p1.y )-( p2.x, p2.y ), rgb( 100, 100, 100 )
			next i
		
			line ( 100, 200 )-step( 600, 10 ), rgb( 255, 0, 0 ), b
		
			for i = 0 to nball - 1
				circle (ballb(i)->p.x, ballb(i)->p.y), radius, rgb( 255, 255, 255 )
			next i
		screenunlock()
		
		iskilled = handleEvents( @instance )
		
		sleep 1, 1
	loop until iskilled
	
	return 0											' Nothing went wrong :)
end function

end main( __FB_ARGC__, __FB_ARGV__ )					' Main Entry Point

' ========================================================
'  handleEvents
' ========================================================
function handleEvents( byval physics as physicsInstance ptr ) as integer
	' returns 1 to kill program, 0 to keep going
	dim e as Event
	if ScreenEvent( @e ) then
		select case e.type
		case EVENT_MOUSE_EXIT
			if mouseConstraint then
				cpSpaceRemoveConstraint( physics->space, mouseConstraint )
				cpConstraintFree( mouseConstraint )
				mouseConstraint = NULL
			end if
			
		case EVENT_MOUSE_MOVE
			mouseBody->p = cpv( e.x, 600 - e.y )

		case EVENT_MOUSE_BUTTON_PRESS
			if e.button = BUTTON_LEFT then
				var shape = cpSpacePointQueryFirst( physics->space, mouseBody->p, GRABABLE_MASK_BIT, 0 )
				if shape then
					var body = shape->body
					mouseConstraint = cpPivotJointNew2( mouseBody, body, cpvzero, cpBodyWorld2Local( body, mouseBody->p ) )
					mouseConstraint->maxForce = INFINITY
					mouseConstraint->biasCoef = 1.0
					cpSpaceAddConstraint( physics->space, mouseConstraint )
				end if
			end if

		case EVENT_MOUSE_BUTTON_RELEASE
			if e.button = BUTTON_LEFT then
				if mouseConstraint then
					cpSpaceRemoveConstraint( physics->space, mouseConstraint )
					cpConstraintFree( mouseConstraint )
					mouseConstraint = NULL
				end if
			end if
			
		case EVENT_KEY_RELEASE
			if e.scancode = SC_ESCAPE then return 1
			
		end select
	end if
	return 0
end function

' ========================================================
'  physicsInstance
' ========================================================
constructor physicsInstance( byval Hz as integer, byval grav as cpVect )
	this.deltaTime = 1.0 / csng(Hz)
	this.currentTime = Timer()
	this.accumulator = 0.0
	this.space = cpSpaceNew()
	this.space->gravity = grav
end constructor

destructor physicsInstance()
	cpSpaceFreeChildren( this.space )
	cpSpaceFree( this.space )
end destructor

sub physicsInstance.simulate()
	dim as double newTime = Timer()
	dim as double ftime = newTime - this.currentTime
	this.accumulator += ftime
	if not this.pause then
		while this.accumulator >= this.deltaTime
			cpSpaceStep( this.space, this.deltaTime )
			this.accumulator -= this.deltaTime
		wend
	end if
	this.currentTime = newTime
end sub

