''
''	Physics Demo - How to fix the timestep issue
''
''		Because different computers can process things at different speeds,
''		it is important to setup a mechanism that keeps the speed of the
''		simulation the same across different computers.  Some people would
''		frown at this, but I use threads to stabilize this, and it works
''		quite well from my tests.
''
''		To compile:
''			fbc demo-timestep.bas
''

#include "chipmunk/chipmunk.bi"
#inclib "chipmunk"

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

const as cpFloat mass = 100.0, radius = 10

screenres 800, 600, 16

cpInitChipmunk()

dim simulation as physSpace
dim circleBody as cpBody ptr
dim circleShape as cpShape ptr

startPhysTick( @simulation, 60, cpv( 0, 100 ) )

circleBody = cpBodyNew( mass, cpMomentForCircle( mass, 0.0, radius, cpvzero ) )
circleBody->p = cpv( 400, 0 )
cpSpaceAddBody( simulation.space, circleBody )

circleShape = cpCircleShapeNew( circleBody, radius, cpvzero )
cpSpaceAddShape( simulation.space, circleShape )

resumePhysTick( @simulation )
do
	screenlock()
		cls
		lockPhysics( @simulation )
			circle( circleBody->p.x, circleBody->p.y ), radius, rgb( 255, 0, 0 )
		unlockPhysics( @simulation )
	screenunlock()

	sleep 1, 1
loop until multikey(&H01)

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
