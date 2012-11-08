''
''	Example of how to use easyChipmunk
''

#include once "../../inc/chipmunk/chipmunk.bi"
#include once "../easyChipmunk.bi"
#include "fbgfx.bi"
using fb

' Change these values and recompile to change the simulation behaviour
#define SPAWN_BOUNCY	0.1
#define	SPAWN_FRICTION	0.5
#define SPAWN_MASS		1.0
#define SPAWN_RADIUS	12

#define GRABABLE_MASK_BIT (1 shl 31)
#define NOT_GRABABLE_MASK (not GRABABLE_MASK_BIT)

declare sub setupScreen( byval w as integer, byval h as integer, byval instance as easyChipmunk ptr )

function drawEachShape( byval parent as physicsObjectList ptr, byval s as physicsObject ptr, byval u as any ptr ) as ubyte
	dim shape as cpShape ptr = s->getShape
	dim colour as uinteger
	if s->userData then
		colour = *cptr( integer ptr, s->userData )
	else
		colour = rgb( 50, 50, 50 )
	end if
	select case shape->klass->type
	case CP_CIRCLE_SHAPE
		circle( shape->body->p.x, shape->body->p.y ), cpCircleShapeGetRadius( shape ), colour
		pset( shape->body->p.x, shape->body->p.y ), rgb(255,200,200)
	case CP_SEGMENT_SHAPE
		var a = cpSegmentShapeGetA( shape )
		var b = cpSegmentShapeGetB( shape )
		line( a.x, a.y )-( b.x, b.y ), colour
	case CP_POLY_SHAPE
		var center = shape->body->p
		var vert = cpPolyShapeGetVert( shape, 0 )
		pset( center.x + vert.x, center.y + vert.y ), colour
		for i as integer = 1 to cpPolyShapeGetNumVerts( shape )-1
			var vert = cpPolyShapeGetVert( shape, i )
			line -( center.x + vert.x, center.y + vert.y ), colour
		next
	end select
	return 1
end function

function removeFirstShape( byval parent as physicsObjectList ptr, byval s as physicsObject ptr, byval u as any ptr ) as ubyte
	dim shape as cpShape ptr = s->getShape
	dim body as cpBody ptr = shape->body
	if (not cpBodyIsStatic( body )) and (not cpBodyIsRogue( body )) then
		parent->removeObjectByPtr( cptr( any ptr, body ) )
		parent->removeObjectByPtr( cptr( any ptr, shape ) )
		return 0
	end if
	return 1
end function

dim physics as easyChipmunk = easyChipmunk()

physics.lock()
	setupScreen( 800, 600, @physics )
	physics.space->gravity = cpv( 0, 100 )
	physics.space->sleepTimeThreshold = 0.1
	physics.space->idleSpeedThreshold = 0.8
	physics.pause = 0
physics.unlock()

dim as integer mx, my, mb
dim as double spawnTimer = Timer
dim as cpConstraint ptr mouseSpring = NULL
dim as cpBody ptr mousePointer = cpBodyNewStatic( )
dim as integer frameTimer = Timer + 1
dim as integer framecount = 0, fps = 0, shapecount = 0

do
	physics.simulate()
	physics.lock()
		screenlock()
			'line (0,0)-(800, 600), rgb(0,0,0), bf
			cls
			'line ( 1, 1 )-step( 350, 50 ), rgba( 100, 100, 100, 100 ), bf
			'draw string ( 5, 2 ), "Left click to move around objects", rgb( 255, 255, 255 )
			'draw string ( 5, 12 ), "Right click to spawn a random object", rgb( 255, 255, 255 )
			'draw string ( 5, 22 ), "Right click + Ctrl to remove an object", rgb( 255, 255, 255 )
			'draw string ( 5, 32 ), "Simulating " & shapecount & " shapes", rgb( 255, 255, 255 )
			locate 1, LoWord( width() ) - 9 : print using "##### fps"; fps
			shapecount = physics.eachObject( easyChipmunk_SHAPE, @drawEachShape, NULL )
			if mouseSpring <> NULL then
				line( mouseSpring->a->p.x, mouseSpring->a->p.y )-( mouseSpring->b->p.x, mouseSpring->b->p.y ), rgb( 200, 200, 255 )
			end if
		screenunlock()
		getmouse mx, my, , mb
		if mb = -1 then mb = 0
		if ( mx <> -1 ) and ( my <> -1 ) then mousePointer->p = cpv( mx, my )
		if ( mb And 1 ) then
			if mouseSpring = NULL then
				var query = cpSpacePointQueryFirst( physics.space, mousePointer->p, GRABABLE_MASK_BIT, 0 )
				if query <> NULL then
					mouseSpring = cpDampedSpringNew( mousePointer, query->body, cpvzero, cpBodyWorld2Local( query->body, mousePointer->p ), 1.0, 100.0, 2.0 )
					mouseSpring->maxForce = INFINITY
					cpSpaceAddConstraint( physics.space, mouseSpring )
				end if
			end if
		else
			if mouseSpring <> NULL then
				cpSpaceRemoveConstraint( physics.space, mouseSpring )
				cpConstraintFree( mouseSpring )
				mouseSpring = NULL
			end if
		end if
		if (mb And 2 ) then
			if multikey( SC_CONTROL ) then
				var query = cpSpacePointQueryFirst( physics.space, mousePointer->p, GRABABLE_MASK_BIT, 0 )
				if query <> NULL then
					physics.list("balls")->removeObjectByPtr( cptr( any ptr, query->body ) )
					physics.list("balls")->removeObjectByPtr( cptr( any ptr, query ) )
				end if
			else
				if ( Timer > spawnTimer ) And ( ( mousePointer->p.x > 0 ) And ( mousePointer->p.y > 0 ) And ( mousePointer->p.x < 800 ) And ( mousePointer->p.y < 600 ) ) then
					spawnTimer = Timer + 0.01
					var newBody = cpBodyNew( SPAWN_MASS, cpMomentForCircle( SPAWN_MASS, SPAWN_RADIUS, 0.0, cpvzero ) )
					var newShape = cpCircleShapeNew( newBody, SPAWN_RADIUS, cpvzero )
					newShape->e = SPAWN_BOUNCY
					newShape->u = SPAWN_FRICTION
					newBody->p = mousePointer->p
					physics.list("balls")->addBody( newBody )
					var shapeObject = physics.list("balls")->addShape( newShape )
					shapeObject->userDataAlloc( sizeof( integer ) )
					*cptr( integer ptr, shapeObject->userData ) = rgb( (rnd*100)+155, (rnd*100)+155, (rnd*100)+155 )
				end if
			end if
		end if
		if ( Timer > spawnTimer ) And multikey( SC_C ) then
			spawnTimer = Timer + 0.001
			physics.eachObject( easyChipmunk_SHAPE, @removeFirstShape )
		end if
	physics.unlock()
	
	framecount += 1
	if Timer > frameTimer then
		fps = framecount
		framecount = 0
		frameTimer = Timer + 1
	end if
loop until multikey( &h01 )

end 0

sub setupScreen( byval w as integer, byval h as integer, byval instance as easyChipmunk ptr )
	screenres w, h, 16, , GFX_ALPHA_PRIMITIVES
	Randomize Timer
	
	dim wallVerts(0 to 3) as cpVect = { cpv(0, 0), cpv(w-1,0), cpv(w-1,h-1), cpv(0, h-1) }
	dim j as integer
	var boundaries = instance->newList( "boundaries" )
	for i as integer = 0 to 3
		j = i + 1
		if j > 3 then j = 0
		dim ws as cpShape ptr
		ws = cpSegmentShapeNew( instance->space->staticBody, wallVerts(i), wallVerts(j), 15.0 )
		ws->u = 0.1
		ws->e = 1.0
		ws->layers = NOT_GRABABLE_MASK
		var shapeObject = boundaries->addShape( ws )
		shapeObject->userDataAlloc( sizeof( uinteger ) )
		*cptr( uinteger ptr, shapeObject->userData ) = rgb( 254,254,254 )
	next i
end sub
