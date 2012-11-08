''
''	Physics Demo - Basic Physics
''
''		Basically more of a toy to help get your physics games working nicely
''		Also, feel free to steal my wrapper-UDT for smooth physics.
''
''		To compile:
''			fbc demo-basic.bas
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
type PhysEngine
	declare constructor( byval Hz as integer )	' This allocates/initializes all the main/global chipmunk variables
	declare destructor( )						' De-allocate/initialize all the chipmunk stuff
	
	declare sub simulate( )						' Starts the simulation ticks
	declare sub stop( )							' Stop simulation ticks
	declare function iskilled( ) as ubyte		' checks to see if the simulation was stopped
	
	declare sub getinput()						' Input handling
	declare sub update()						' Update the screen
	
	mousePoint	as cpVect						' Mouse position
	mouseBody	as cpBody ptr					' A static body attached to the mouse position
	mouseJoint	as cpConstraint ptr				' This connects a non-static body to the mouse temporarily
	
	space		as cpSpace ptr					' Simulation space
	staticbody	as cpBody ptr					' Main static body (for attaching any shape to)
	
	kill		as unsigned integer				' Flag to stop the physics engine
	mutex		as any ptr						' Mutex for thread safety
	physthread	as any ptr						' Thread to make physics simulation consistent
	
	deltaTime	as cpFloat						' dt used in cpSpaceStep
	currentTime	as double						' Used to help smooth out the simulation
	accumulator	as double						' Helps figure out how many more cpSpaceStep calls are needed
end type

' Physics engine tick thread
sub physicstick( byval e as any ptr )
	var physics = cptr( PhysEngine ptr, e )
	
	do
		mutexlock( physics->mutex )
			var newTime = Timer()
			var ftime = newTime - physics->currentTime
			physics->accumulator += ftime
			while physics->accumulator >= physics->deltaTime
				cpSpaceStep( physics->space, physics->deltaTime )
				physics->accumulator -= physics->deltaTime
			wend
			physics->currentTime = newTime
		mutexunlock( physics->mutex )
		
		' Sleep a consistent amount of time to make a smooth simulation without affecting the framerate
		sleep 1, 1
	loop until physics->iskilled()
end sub

function main( byval argc as integer, byval argv as zstring ptr ptr ) as integer
	dim i as integer
	
	dim instance as PhysEngine ptr = new PhysEngine( 60 )	' Create a new instance of the physics engine (60 hertz)
	
	' Create a simulation
	dim as cpShape ptr ground(0 to 3)
	ground(0) = cpSegmentShapeNew( instance->staticBody, cpv( 10, 10 ), cpv( 790, 10 ), 1 )
	ground(1) = cpSegmentShapeNew( instance->staticBody, cpv( 790, 10 ), cpv( 790, 590 ), 1 )
	ground(2) = cpSegmentShapeNew( instance->staticBody, cpv( 790, 590 ), cpv( 10, 590 ), 1 )
	ground(3) = cpSegmentShapeNew( instance->staticBody, cpv( 10, 590 ), cpv( 10, 10 ), 1 )
	for i = 0 to 3
		ground(i)->e = 1.0
		ground(i)->u = 10.0
		ground(i)->layers = NOT_GRABABLE_MASK
		cpSpaceAddStaticShape( instance->space, ground( i ) )
	next i
	
	dim as cpShape ptr Sbox
	dim verts(0 to 3) as cpVect = { cpv( 0, 10 ), cpv( 600, 10 ), cpv( 600, 0 ), cpv( 200, 0 ) }
	Sbox = cpPolyShapeNew( instance->staticBody, 4, @verts(0), cpv( 100, 200 ) )
	Sbox->e = 0.5
	Sbox->u = 5.0
	Sbox->layers = NOT_GRABABLE_MASK
	cpSpaceAddShape( instance->space, Sbox )
	
	dim nball as integer = 50
	dim ballb(0 to nball - 1) as cpBody ptr, balls(0 to nball - 1) as cpShape ptr
	
	var radius = 15.0
	var mass = 10.0
	
	for i = 0 to nball - 1
		ballb(i) = cpBodyNew( mass, cpMomentForCircle( mass, 0.0, radius, cpvzero ) )
		ballb(i)->p = cpv( int(rnd * 700)+50, int(rnd * 450)+ 50 )
		cpSpaceAddBody( instance->space, ballb(i) )
		
		balls(i) = cpCircleShapeNew( ballb(i), radius, cpvzero )
		balls(i)->e = 1.0
		balls(i)->u = 1.0
		cpSpaceAddShape( instance->space, balls(i) )
	next i
	
	' Begin the simulation
	instance->simulate( )
	
	do
		instance->getinput()
		
		mutexlock( instance->mutex )
		
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
		
		mutexunlock( instance->mutex )
		
		instance->update()
		
		sleep 1, 1
	loop until instance->iskilled()
	
	delete instance										' Cleanup
	
	return 0											' Nothing went wrong :)
end function

end main( __FB_ARGC__, __FB_ARGV__ )					' Main Entry Point

'' ------------------------------ PhysEngine
'  Wrapper class to help "organize" my demo

constructor PhysEngine( byval Hz as integer )
	screenres 800, 600, 32
	screenset 0, 1
	Window ( 0, 600 ) - ( 800, 0 )						' Chipmunk flips the y axis (as most game libraries do), so I have flipped it
	
	randomize timer
	
	cpInitChipmunk()									' Initialize the chipmunk engine
		
	this.kill = 0
	this.mutex = mutexcreate()
	
	this.staticBody = cpBodyNew(INFINITY, INFINITY)
	this.mouseBody = cpBodyNew(INFINITY, INFINITY)
	
	this.space = cpSpaceNew()
	this.space->iterations = 10
	cpSpaceResizeStaticHash( this.space, 30.0, 1000 )
	cpSpaceResizeActiveHash( this.space, 30.0, 1000 )
	this.space->gravity = cpv(0, -100)
	
	WindowTitle "Chipmunk Physics Engine v" & cpVersion()
	
	this.deltaTime = 1.0 / csng( Hz )
	this.currentTime = TIMER
	this.accumulator = 0.0
end constructor

destructor PhysEngine( )
	if this.iskilled() = 0 then
		this.stop()
		mutexdestroy this.mutex
	end if
	cpBodyFree( this.staticBody )
	cpBodyFree( this.mouseBody )
	cpSpaceFree( this.space )
end destructor
	
	
sub PhysEngine.simulate( )
	this.physthread = threadcreate( @physicstick, @this )
end sub

sub PhysEngine.stop()
	mutexlock( this.mutex )
		this.kill = 1
	mutexunlock( this.mutex )
	threadwait this.physthread
	cpSpaceFreeChildren( this.space )
end sub

function PhysEngine.iskilled() as ubyte
	mutexlock( this.mutex )
		var status = this.kill
	mutexunlock( this.mutex )
	return status
end function

sub PhysEngine.getinput()
	dim e as Event
	dim killit as integer = 0
	if ScreenEvent( @e ) then
		mutexlock( this.mutex )
		select case e.type
			case EVENT_MOUSE_EXIT
				' Remove any joint between the mouse and a shape if the mouse leaves the window
				if this.mouseJoint <> 0 then
					cpSpaceRemoveConstraint( this.space, this.mouseJoint )
					cpConstraintFree( this.mouseJoint )
					this.mouseJoint = NULL
				end if
				
			case EVENT_MOUSE_MOVE
				' Update the mouse location
				var newPoint = cpv( e.x, 600 - e.y )
				'this.mouseBody->v = cpvmult( cpvsub( newPoint, this.mousePoint ), 0.01 )
				this.mousePoint = newPoint
				this.mouseBody->p = this.mousePoint
			case EVENT_MOUSE_BUTTON_PRESS
				if e.button = BUTTON_LEFT then
					var shape = cpSpacePointQueryFirst( this.space, mousePoint, GRABABLE_MASK_BIT, 0 )
					if shape <> 0 then
						' If you click on a shape, then attach it to the mouse
						var body = shape->body
						this.mouseJoint = cpPivotJointNew2( this.mouseBody, body, cpvzero, cpBodyWorld2Local( body, this.mousePoint ) )
						this.mouseJoint->maxForce = INFINITY
						this.mouseJoint->biasCoef = 1.0
						cpSpaceAddConstraint( this.space, this.mouseJoint )
					end if
				end if
			case EVENT_MOUSE_BUTTON_RELEASE
				if e.button = BUTTON_LEFT then
					if this.mouseJoint <> 0 then
						' Release the any attached shape from the mouse
						cpSpaceRemoveConstraint( this.space, this.mouseJoint )
						cpConstraintFree( this.mouseJoint )
						this.mouseJoint = NULL
					end if
				end if
			case EVENT_KEY_RELEASE
				' Exit if the user presses escape
				if e.scancode = SC_ESCAPE then killit = 1
		end select
		mutexunlock( this.mutex )
	end if
	if killit = 1 then this.stop()
end sub

sub PhysEngine.update()		
	screencopy
	flip
end sub

