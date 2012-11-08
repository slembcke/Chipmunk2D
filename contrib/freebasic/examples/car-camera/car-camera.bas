''
''	Example of how to use easyChipmunk
''
''	This source file is part of the chipmunk-freebasic project.
''	http://code.google.com/p/chipmunk-freebasic/
''	You can find downloadable packages and SVN updates for the latest
''	code releases and tutorials.
''

'#define NOTHREAD
#include "../easyChipmunk.bi"
#include "fbgfx.bi"
using fb

' Change these values and recompile to change the simulation behaviour
#define SPAWN_BOUNCY	0.1
#define	SPAWN_FRICTION	1.0
#define	SPAWN_MASS		1.0

#define MOTOR_TICK_RATE 0.1

#define GRABABLE_MASK_BIT (1 shl 31)
#define NOT_GRABABLE_MASK (not GRABABLE_MASK_BIT)

#define ResX	800
#define ResY	600

dim shared as cpVect camera '' So some of the simple functions have access

type physParticle
	declare constructor( byval instance as easyChipmunk ptr, byval p as cpVect, byval c as uinteger, byval lifespan as double = 1.0 )
	declare destructor( )
	
	as cpBody ptr	b
	as cpShape ptr	s
	as double		birth, death
	as uinteger		colour
end type

type drivingMotor
	declare constructor( byval instance as easyChipmunk ptr, byval p as cpVect )
	declare destructor()
	declare sub forward()
	declare sub reverse()
	declare sub boost()
	speed					as cpFloat
	slowRate				as cpFloat
	min						as cpFloat
	max						as cpFloat
	nextUpdate		as double
	motor					as cpConstraint ptr
  list					as physicsObjectList ptr
	ec						as easyChipmunk ptr
end type

constructor drivingMotor( byval instance as easyChipmunk ptr, byval p as cpVect )
	this.list = instance->newList( "motor" )
  
	this.slowRate = 100
	this.speed = 0
	this.min = 0
	this.max = 5
	this.nextUpdate = Timer
	this.ec = instance
  
  var body = cpBodyNew( 10, cpMomentForCircle( 10, 20, 0, cpvzero ) )
	var shape = cpCircleShapeNew( body, 20, cpvzero )
	shape->u = 0.7
	shape->e = 0.99
	body->p = p
	
	this.list->addBody( body, "wheelBody" )
	this.list->addShape( shape, "wheelShape" )
	
	'this.motor = cpSimpleMotorNew( body, instance->space->staticBody, 0.0 )
  
	'this.list->addConstraint( this.motor )
end constructor

destructor drivingMotor()
	this.ec->removeList( "motor" )
end destructor

sub drivingMotor.forward()
	if Timer < this.nextUpdate then return
  this.nextUpdate = Timer + MOTOR_TICK_RATE
  
  var wheelBody = this.list->getObjectById( "wheelBody" )->getBody

	this.speed += 1.0
	if this.speed > this.max then
		this.speed = this.max
	end if
  
  cpBodySetAngVel( wheelBody, this.speed )
end sub
	
sub drivingMotor.reverse()
	if Timer < this.nextUpdate then return
  this.nextUpdate = Timer + MOTOR_TICK_RATE

  var wheelBody = this.list->getObjectById( "wheelBody" )->getBody

	this.speed -= 1.0
	if this.speed < this.min then
		this.speed = this.min
	end if
  
  cpBodySetAngVel( wheelBody, this.speed )
end sub

sub drivingMotor.boost()
	'if Timer < this.nextUpdate then return
  'this.nextUpdate = Timer + MOTOR_TICK_RATE
  
  var wheelBody = this.list->getObjectById( "wheelBody" )->getBody
  
  cpBodyApplyImpulse( wheelBody, cpv(0, -3 ), cpvzero )
end sub
	

function drawEachShape( byval parent as physicsObjectList ptr, byval s as physicsObject ptr, byval u as any ptr ) as ubyte
	dim shape as cpShape ptr = s->getShape
	dim colour as uinteger
	if s->userData then
		colour = *cptr( uinteger ptr, s->userData )
	else
		colour = rgb( 50, 50, 50 )
	end if
	dim as cpVect center = cpvsub( shape->body->p, camera )
	select case shape->klass->type
	case CP_CIRCLE_SHAPE
		/'if s->id = "wheelShape" then
			circle( center.x, center.y ), cpCircleShapeGetRadius( shape ), rgb( 25, 25, 25 ),,,,F
			circle( center.x, center.y ), cpCircleShapeGetRadius( shape ), colour
			var radAng = ((shape->body->a * 180 / 3.14159) mod 360) * (3.14159/180)
			for u as integer = 0 to 3
				var rotX = cos( radAng + (u*90*3.14159/180) ) * (cpCircleShapeGetRadius( shape )/2)
				var rotY = sin( radAng + (u*90*3.14159/180) ) * (cpCircleShapeGetRadius( shape )/2)
				pset( center.x + rotX, center.x + rotY ), rgb(255,255,255)
			next u
		else'/
			circle( center.x, center.y ), cpCircleShapeGetRadius( shape ), colour
			var radAng = ((shape->body->a * 180 / 3.14159) mod 360) * (3.14159/180)
			var rotX = cos( radAng ) * cpCircleShapeGetRadius( shape )
			var rotY = sin( radAng ) * cpCircleShapeGetRadius( shape )
			line ( center.x, center.y )-step( rotX, rotY ), colour
		'end if
	case CP_SEGMENT_SHAPE
		var a = cpvsub( cpSegmentShapeGetA( shape ), camera )
		var b = cpvsub( cpSegmentShapeGetB( shape ), camera )
		line( a.x, a.y )-( b.x, b.y ), colour
	case CP_POLY_SHAPE
		dim as cpPolyShape ptr polygon = cptr( cpPolyShape ptr, shape )
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

declare sub setupScreen( byval w as integer, byval h as integer, byval instance as easyChipmunk ptr )

dim physics as easyChipmunk = easyChipmunk()
dim as physicsObject ptr wheelBody, wheelShape
dim shared wheel as drivingMotor ptr
dim shared as cpFloat rpm = 0.0

physics.lock()
	setupScreen( ResX, ResY, @physics )
	physics.space->gravity = cpv( 0, 600 )
	physics.space->sleepTimeThreshold = 0.05
	physics.space->idleSpeedThreshold = 0.1
  
	wheel = new drivingMotor( @physics, cpv( 50, 50 ) )
	wheel->max = 50
	wheel->min = -50
	wheel->slowRate = 0.01
	physics.pause = 0
	physics.newList( "balls" )
physics.unlock()

dim as integer mx, my, mb
dim as double spawnTimer = Timer
dim as cpConstraint ptr mouseSpring = NULL
dim as cpBody ptr mousePointer = cpBodyNewStatic( )
dim as integer frameTimer = Timer + 1
dim as integer framecount = 0, fps = 0, shapecount = 0

camera = cpv(0,0)

do
	physics.simulate()
	physics.lock()
		'wheelBody = physics.list("wheel")->getObjectById( "circle-body" )->getBody
		screenlock()
			'line (0,0)-(ResX, ResY), rgb(0,0,0), bf
			cls
			line ( 1, 1 )-step( 350, 50 ), rgba( 100, 100, 200, 100 ), bf
			draw string ( 5, 2 ), "Left click to move around objects", rgb( 255, 255, 255 )
			draw string ( 5, 12 ), "Right click to spawn a random object", rgb( 255, 255, 255 )
			draw string ( 5, 22 ), "Right click + Ctrl to remove an object", rgb( 255, 255, 255 )
			draw string ( 5, 32 ), "Simulating " & shapecount & " shapes", rgb( 255, 255, 255 )
			locate 1, LoWord( width() ) - 9 : print using "##### fps"; fps
			shapecount = physics.eachObject( easyChipmunk_SHAPE, @drawEachShape, NULL )
			if mouseSpring <> NULL then
				var a1 = cpvsub( cpBodyLocal2World( mouseSpring->a, cpDampedSpringGetAnchr1( mouseSpring ) ), camera )
				var a2 = cpvsub( cpBodyLocal2World( mouseSpring->b, cpDampedSpringGetAnchr2( mouseSpring ) ), camera )
				line( a1.x, a1.y )-( a2.x, a2.y ), rgb( 200, 200, 255 )
			end if
		screenunlock()
		getmouse mx, my, , mb
		if mb = -1 then mb = 0
		if ( mx <> -1 ) and ( my <> -1 ) then mousePointer->p = cpvadd( cpv( mx, my ), camera )
		if ( mb And 1 ) then
			if mouseSpring = NULL then
				var query = cpSpacePointQueryFirst( physics.space, mousePointer->p, GRABABLE_MASK_BIT, 0 )
				if query <> NULL then
					mouseSpring = cpDampedSpringNew( mousePointer, query->body, cpvzero, cpBodyWorld2Local( query->body, mousePointer->p ), 1.0, 100.0, 2.0 )
					''mouseSpring->biasCoef = 1.0
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
					''removeShape( shapes(), @physics, query )
          var lst = physics.list("balls")
					var o = lst->hasObject( query )
          if 0 <> NULL then
            lst->removeObjectByPtr( cptr( any ptr, query->body ) )
            lst->removeObjectByPtr( cptr( any ptr, query ) )
          end if
				end if
			else
				if ( Timer > spawnTimer ) then
					spawnTimer = Timer + 0.01
					var radius = csng( 1 + ( rnd * 3 ) )
					var newBody = cpBodyNew( SPAWN_MASS, cpMomentForCircle( SPAWN_MASS, radius, 0.0, cpvzero ) )
					var newShape = cpCircleShapeNew( newBody, radius, cpvzero )
					newShape->e = SPAWN_BOUNCY
					newShape->u = SPAWN_FRICTION
					newBody->p = mousePointer->p
					physics.list("balls")->addBody( newBody )
					var shapeObject = physics.list("balls")->addShape( newShape )
					shapeObject->userDataAlloc( sizeof( uinteger ) )
					*cptr( uinteger ptr, shapeObject->userData ) = rgb( 100 + int(rnd*150), 100 + int(rnd*150), 100 + int(rnd*150) )
				end if
			end if
		end if
		
		if ( Timer > spawnTimer ) And multikey( SC_C ) then
			spawnTimer = Timer + 0.005
			physics.list("balls")->eachObject( easyChipmunk_SHAPE, @removeFirstShape )
		end if
		
		if multikey( SC_LEFT ) then
			wheel->reverse()
		end if
		if multikey( SC_RIGHT ) then
			wheel->forward()
		end if
    if multikey( SC_SPACE ) then
      wheel->boost()
    end if
    'camera = cpvsub( wheel->list->getObjectById( "wheelBody" )->getBody->p, cpv(ResX/2,ResY*0.66) )
	physics.unlock()
	
	framecount += 1
	if Timer > frameTimer then
		fps = framecount
		framecount = 0
		frameTimer = Timer + 1
	end if
loop until multikey( &h01 )

physics.lock()
  delete wheel
physics.unlock()

end 0

sub setupScreen( byval w as integer, byval h as integer, byval instance as easyChipmunk ptr )
	screenres w, h, 16 ', , GFX_ALPHA_PRIMITIVES
	Randomize Timer
	
	dim wallVerts(0 to 6) as cpVect = { _
      cpv(0, 0), _
      cpv(w,0), _
      cpv(w,h*0.75), _
      cpv(w*0.9,h*0.75), _
      cpv(w*0.9,h*0.6), _
      cpv(w*0.5, h*0.5), _
      cpv(0, h*0.5) _
  }
	dim j as integer
	for i as integer = lbound(wallVerts) to ubound(wallVerts)
		j = i + 1
		if j > 6 then j = 0
		dim ws as cpShape ptr = cpSegmentShapeNew( instance->space->staticBody, wallVerts(i), wallVerts(j), 10.0 )
		ws->u = 1.1
		ws->e = 0.5
		ws->layers = NOT_GRABABLE_MASK
		'cpSpaceAddStaticShape( instance->space, boundaries(i+1).shape )
		var shapeObject = instance->list("boundaries")->addShape( ws )
		shapeObject->userDataAlloc( sizeof( uinteger ) )
		*cptr( uinteger ptr, shapeObject->userData ) = rgb( 254,254,254 )
	next i
end sub
