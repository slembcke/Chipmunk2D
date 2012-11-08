''
''	Example of how to make a local gravity entity
''

#include "../easyChipmunk.bi"
#include "fbgfx.bi"
using fb

#define GRABABLE_MASK_BIT (1 shl 31)
#define NOT_GRABABLE_MASK (not GRABABLE_MASK_BIT)

' Change these values and recompile to change the simulation behaviour
#define SPAWN_BOUNCY	0.1
#define	SPAWN_FRICTION	0.1
#define SPAWN_MASS		10
#define SPAWN_RADIUS	10
#define SPAWN_DELAY   1.0


type myPlanet
	body				as cpBody ptr
	shape				as cpShape ptr
	gravityStrength		as cpFloat
end type

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
		if s->id = "planet" then
			circle( shape->body->p.x, shape->body->p.y ), cpCircleShapeGetRadius( shape ), rgb( 200, 100, 100 ),,,,F
			circle( shape->body->p.x, shape->body->p.y ), cpCircleShapeGetRadius( shape ), rgb( 255, 255, 255 )
			var radAng = ((shape->body->a * 180 / 3.14159) mod 360) * (3.14159/180)
			var rotX = cos( radAng ) * cpCircleShapeGetRadius( shape )
			var rotY = sin( radAng ) * cpCircleShapeGetRadius( shape )
			line ( shape->body->p.x, shape->body->p.y )-step( rotX, rotY ), rgb( 255, 255, 255 )
		else
			circle( shape->body->p.x, shape->body->p.y ), cpCircleShapeGetRadius( shape ), colour
			var radAng = ((shape->body->a * 180 / 3.14159) mod 360) * (3.14159/180)
			var rotX = cos( radAng ) * cpCircleShapeGetRadius( shape )
			var rotY = sin( radAng ) * cpCircleShapeGetRadius( shape )
			line ( shape->body->p.x, shape->body->p.y )-step( rotX, rotY ), colour
		end if
	case CP_SEGMENT_SHAPE
		var a = cpSegmentShapeGetA( shape )
		var b = cpSegmentShapeGetB( shape )
		line( a.x, a.y )-( b.x, b.y ), colour
	case CP_POLY_SHAPE
		dim as cpPolyShape ptr polygon = cptr( cpPolyShape ptr, shape )
		dim as cpVect center = shape->body->p
		''var vert = polygon->tVert[0]
		''pset( center.x + vert.x, center.y + vert.y ), colour
		for i as integer = 0 to polygon->numVerts-1
			''var vert = polygon->tVert[i]
			''line -( center.x + vert.x, center.y + vert.y ), colour
			pset( center.x + polygon->tVert[i].x, center.y + polygon->tVert[i].y ), colour
		next
		''vert = polygon->tVert[0]
		''line -( center.x + vert.x, center.y + vert.y ), colour
	end select
	return 1
end function

function removeFirstBody( byval parent as physicsObjectList ptr, byval s as physicsObject ptr, byval u as any ptr ) as ubyte
	dim shape as cpShape ptr = s->getShape
	dim body as cpBody ptr = shape->body
	if (not cpBodyIsStatic( body )) and (not cpBodyIsRogue( body )) then
		parent->removeObjectByPtr( cptr( any ptr, body ) )
		parent->removeObjectByPtr( cptr( any ptr, shape ) )
		return 0
	end if
	return 1
end function

declare sub setupScreen( byval w as integer, byval h as integer, byval instance as easyChipmunk ptr )

dim shared planet as myPlanet

' Update function for local gravity - see chipmunk's official demos, specifically planet.c for more information
sub planetGravityVelocityFunc cdecl( byval body as cpBody ptr, byval gravity as cpVect, byval damping as cpFloat, byval dt as cpFloat )
	' Offset body
	var p = cpvsub( cpBodyGetPos( body ), planet.body->p )
	var sqdist = cpvlengthsq( p )
	var g = cpvmult( p, -planet.gravityStrength / ( sqdist * cpfsqrt( sqdist ) ) )
	
	cpBodyUpdateVelocity( body, g, damping, dt )
end sub

' Not a necessary function, but is good to implement in the long run
sub updateFunc( byval ec as any ptr )
	dim physics as easyChipmunk ptr = cptr( easyChipmunk ptr, ec )
	dim planet as myPlanet ptr = cptr( myPlanet ptr, physics->userData )
	cpBodyUpdatePosition( planet->body, physics->deltaTime )
end sub

dim physics as easyChipmunk = easyChipmunk()

' Create a planet
planet.body = cpBodyNewStatic( )
planet.body->w = 0.2
planet.body->p = cpv( 400, 300 )
planet.shape = cpCircleShapeNew( planet.body, 70.0, cpvzero )
planet.shape->e = 1.0
planet.shape->u = 1.0
planet.shape->layers = NOT_GRABABLE_MASK
planet.gravityStrength = 500000

physics.lock()
	setupScreen( 800, 600, @physics )
	cpSpaceSetIterations( physics.space, 20 )
	cpSpaceSetGravity( physics.space, cpv( 0, 0 ) )
	physics.space->sleepTimeThreshold = 0.05
	physics.space->idleSpeedThreshold = 1
	physics.onUpdate = cptr( ecUpdateFunc, @updateFunc )			' * Set update function
	physics.list("planet")->addShape( planet.shape, "planet" )
	''planet.shape = cpSpaceAddShape( physics.space, planet.shape )	' * Add planet shape to simulation
	physics.userData = @planet										' * Set the planet as the user data
	physics.pause = 0
physics.unlock()

dim as integer mx, my, mb
dim as double spawnTimer = Timer
dim as cpConstraint ptr mouseSpring = NULL
dim as cpBody ptr mousePointer = cpBodyNewStatic( )
dim as integer frameTimer = Timer + 1
dim as integer framecount = 0, fps = 0, shapecount = 0
dim as double deleteTimer = Timer

do
	physics.simulate()
	physics.lock()
		screenlock()
			cls
			locate 1, 2: print "Left click to move around objects"
			locate 2, 2: print "Right click to spawn a random object"
			locate 3, 2: print "Right click + Ctrl to remove an object"
			locate 4, 2 : Print Using "Simulating #### shapes"; shapecount
			locate 1, LoWord( width() ) - 9 : print using "##### fps"; fps
			shapecount = physics.eachObject( easyChipmunk_SHAPE, @drawEachShape, NULL )
			if mouseSpring <> NULL then
				line( mouseSpring->a->p.x, mouseSpring->a->p.y )-( mouseSpring->b->p.x, mouseSpring->b->p.y ), rgb( 200, 200, 255 )
			end if
			'circle( planet.body->p.x, planet.body->p.y ), cpCircleShapeGetRadius( planet.shape ), rgba( 255, 0, 0, 255 )
			'paint( planet.body->p.x, planet.body->p.y ), rgba( 255, 200, 200, 100 ), rgba( 255, 0, 0, 255 )
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
					spawnTimer = Timer + SPAWN_DELAY
					var newBody = cpBodyNew( SPAWN_MASS, cpMomentForCircle( SPAWN_MASS, SPAWN_RADIUS, 0.0, cpvzero ) )
					var newShape = cpCircleShapeNew( newBody, SPAWN_RADIUS, cpvzero )
					newShape->e = SPAWN_BOUNCY
					newShape->u = SPAWN_FRICTION
					newBody->p = mousePointer->p
					
					var pull = cpvsub( newShape->body->p, planet.body->p )
					var r = cpvlength( pull )
					var v = cpfsqrt( planet.gravityStrength / r) / r
					newshape->body->v = cpvmult( cpvperp( pull ), v )
					newShape->body->w = v
					cpBodySetAngle( newShape->body, cpfatan2( newShape->body->p.y, newShape->body->p.x ) )
					newBody->velocity_func = @planetGravityVelocityFunc
					
					physics.list("balls")->addBody( newBody )
					var shapeObject = physics.list("balls")->addShape( newShape )
					shapeObject->userDataAlloc( sizeof( integer ) )
					*cptr( integer ptr, shapeObject->userData ) = rgb( (rnd*100)+155, (rnd*100)+155, (rnd*100)+155 )
				end if
			end if
		end if
		if ( Timer > spawnTimer ) And multikey( SC_C ) then
			spawnTimer = Timer + 0.005
			physics.eachObject( easyChipmunk_SHAPE, @removeFirstBody )
		end if
	physics.unlock()
	
	framecount += 1
	if Timer > frameTimer then
		fps = framecount
		framecount = 0
		frameTimer = Timer + 1
	end if
loop until multikey( &h01 )

physics.lock()
	cpSpaceRemoveShape( physics.space, planet.shape )
	if planet.body then
		cpSpaceRemoveBody( physics.space, planet.body )
		cpBodyFree( planet.body )
	end if
	cpShapeFree( planet.shape )
physics.unlock()

end 0

sub setupScreen( byval w as integer, byval h as integer, byval instance as easyChipmunk ptr )
	screenres w, h, 16, , GFX_ALPHA_PRIMITIVES
	Randomize Timer
	
	dim wallVerts(0 to 3) as cpVect = { cpv(0, 0), cpv(w,0), cpv(w,h), cpv(0, h) }
	dim j as integer
	for i as integer = 0 to 3
		j = i + 1
		if j > 3 then j = 0
		dim ws as cpShape ptr
		ws = cpSegmentShapeNew( instance->space->staticBody, wallVerts(i), wallVerts(j), 15.0 )
		ws->u = 0.1
		ws->e = 1.0
		ws->layers = NOT_GRABABLE_MASK
		'cpSpaceAddStaticShape( instance->space, ws )
		var shapeObject = instance->list("boundaries")->addShape( ws )
		shapeObject->userDataAlloc( sizeof( integer ) )
		*cptr( integer ptr, shapeObject->userData ) = rgb( (rnd*100)+155, (rnd*100)+155, (rnd*100)+155 )
	next i
	
	
end sub
