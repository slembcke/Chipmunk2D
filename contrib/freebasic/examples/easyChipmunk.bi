''
''	Interface to help make chipmunk simulations run consistently on all machines.
''	The interface prefers threading, but if it isn't supported on the current
''	platform, or is disabled via -lang qb, this also supports a non-threaded
''	method to keep the simulation consistent.
''
''	Programmed by: Alex "Oz" Barry, 2010 - alex.barry@gmail.com
''	If you have any questions, comments, or anything else about this header, feel
''	free to email me and I'll get back to you as soon as I can.
''
''	You can use this code for any of your projects, as long as this comment header
''	remains as it is.  If you modify this in any way, please note the changes.  If
''	you have any improvements, let me know and I'll add you to the credits.
''

#include "../inc/chipmunk/chipmunk.bi"
#inclib "chipmunk"

#ifndef __EASYCHIPMUNK
#define __EASYCHIPMUNK

#if defined( __FB_DOS__ ) OR ( __FB_LANG__ = "qb" )				' If this is compiled for DOS, or with the -lang qb flag, don't allow threading
#	ifdef CAN_THREAD											' Make sure the programmer isn't trying to trick the header (What sort of asshole would do this?)
#		undef CAN_THREAD										' Destroy any CAN_THREAD define, just in case
#	endif
#else															' If this is compiled for any other platform (win32, *nix) or without the -lang qb flag
#	define CAN_THREAD											' Inform the rest of the header that it can/should use threading to stabilize the simulation
#endif

#define easyChipmunk_NONE			0
#define easyChipmunk_BODY			1
#define easyChipmunk_SHAPE			2
#define easyChipmunk_CONSTRAINT		4
#define easyChipmunk_ANY			(easyChipmunk_BODY Or easyChipmunk_SHAPE Or easyChipmunk_CONSTRAINT)

type easyChipmunk_ as easyChipmunk								' Forward declare easyChipmunk for any self references

' Physics object
type physicsObject
public:
	declare constructor(byref id as string)						' Create a new clean list item and attach it to a space
	declare destructor()										' Cleanup id string
	
	declare property id() as string								' Get this objects id
	
	declare property getPointer() as any ptr					' Get the memory address of the body/shape/constraint
	
	declare property setBody(byval b as cpBody ptr)				' Reset this object as a body	
	declare property setShape(byval s as cpShape ptr)			' Reset this object as a shape
	declare property setConstraint(byval c as cpConstraint ptr)	' Reset this object as a constraint
	
	' * These properties return NULL if this object's type
	'   does not match the property type
	declare property getBody() as cpBody ptr					' Get a reference to the current body
	declare property getShape() as cpShape ptr					' Get a reference to the current shape
	declare property getConstraint() as cpConstraint ptr		' Get a reference to the current constraint
	
	declare function isObjectType _								' Check to see if this object is of type 'typeToCheck'
			(byval typeToCheck as integer) as ubyte				'  Type to check
			
	declare function userDataAlloc _							' Allocate/Reallocate the user data pointer
			( byval size as integer ) as ubyte					'  Size to reallocate to (0 will deallocate, anything else will resize)
	
	' * These could also be represented in an ANY PTR, but
	'   since a union does not take any more space, and no
	'   processing power would go towards a pointer cast,
	'   I decided on using a union.
	union
		_body		as cpBody ptr								' Reference to body
		_shape		as cpShape ptr								' Refence to shape
		_constraint	as cpConstraint ptr							' Refence to shape
	end union
	userData		as any ptr									' User data associated with this object
	_next			as physicsObject ptr						' Next body in the list
private:
	declare sub _destruct()
	_id				as string									' String id
	_type			as integer									' The type of reference (helps when using the get* property)
	_allocatedUD	as ubyte									' Whether or not the userData has been allocated (and needs to be deallocated when the destructor is called)
end type

' Physics Object List
type physicsObjectList
public:
	declare constructor( byref listIdentifier as string, byval instance as easyChipmunk_ ptr ) ' Create a new object list and attach it to the easyChipmunk instance
	declare destructor( ) ' Clean up the object (remove all objects from space, and free memory)
	
	declare property listName() as string ' Get the list's name
	
	declare property getObjectById _ ' Find an object by it's string name
			(byref index as string) _    ' The string id
			as physicsObject ptr
	
	declare property getObjectByPtr _ ' Find an object by it's memory address
			(byval index as any ptr) _    ' The memory pointer
			as physicsObject ptr
	
	declare function addBody _                ' Add a body to this list
			(byval objectPointer as cpBody ptr, _ ' The cpBody pointer - this will automatically be converted into an object
			byref id as string="") _              ' Assign it a string, otherwise it gets an automatic string assigned to it
			as physicsObject ptr
	
	declare function addShape _               ' Add a shape to this list
			(byval objectPointer as cpShape ptr, _  ' The cpShape pointer (see addBody for details)
			byref id as string="") _                ' Assign it a string, otherwise it gets an automatic string assigned to it
			as physicsObject ptr
	
	declare function addConstraint _          ' Add a constraint to this list
			(byval objectPointer as cpConstraint ptr, _ ' The cpConstraint pointer (see addBody for details)
			byref id as string="") _                    ' Assign it a string, otherwise it gets an automatic string assigned to it
			as physicsObject ptr
	
	declare sub removeObjectById( byref id as string )  ' Remove an object by it's string identifier
	
	declare sub removeObjectByPtr( byval physObj as physicsObject ptr ) ' Remove an object by it's physics object pointer
			
	declare function hasObject( byval objectPointer as any ptr ) as ubyte ' Check to see if this list has a object pointer (can be cpBody, cpShape or cpConstraint pointer)

	declare function eachObject _								' Loop through all physicsObjects that are of 'objectType' type; returns number of matched objects
			(byval objectType as integer, _						'  Object type filter
			byval fn as function _								'  Function callback for each object (return 1 to keep going, 0 to stop looping)
				(byval as physicsObjectList ptr, _				'   Reference to this instance of easyChipmunk
				byval as physicsObject ptr, _					'   Physics object processed in the function
				byval as any ptr) as ubyte, _					'   User data processed in the function (defaults to null)
			byval udata as any ptr = NULL _						'  Pass this data to the function (optional)
			) as integer
	
	_next	as physicsObjectList ptr
private:
	listId		as string
	objects		as physicsObject ptr
	ec			as easyChipmunk_ ptr
end type

type ecUpdateFunc as sub( byval self as any ptr )				' Subroutine prototype for anything that needs to be called each physics iteration
type easyChipmunk
public:
	declare constructor()										' Setup a basic space, and tell the simulation that it should be paused.  If threading is supported, the thread is started here
	declare destructor()										' Release all bodies and shapes attached to the space, and free the space.  Stop the thread is threading is supported.
	
	declare function isUsingThread() as ubyte					' Check to see if this instance of easyChipmunk is using threading - this is defined at compile time.
	
	declare sub simulate()										' Dummy function to advance the simulation.  If threading is supported, this is a dummy function
	declare sub _DOSIMULATIONSTEP()								' Real simulation step.  Do not call this directly for any reason!  Use simulate() instead.
	
	declare property physicsTick() as double					' Get the physics tick rate in hertz (tick per second)
	declare property physicsTick(byval newValue as double)		' Set the physics tick rate, and calculate the necessary delta time.  Modifying this during a simulation is not advised!
	declare property deltaTime() as double						' Get the deltatime step used for the simulation
	
	declare sub lock()											' Lock the simulation mutex.  This is a dummy function is threading is not supported.
	declare sub unlock()										' Unlock the simulation mutex.  This is a dummy function is threading is not supported.
	
	declare property list _									' Get a physics object list
			( byref id as string ) _						' All lists have an unique string id attached to it.
			as physicsObjectList ptr
			
	declare function newList _							' Create a new physics object list (useful for physics objects with multiple shapes, or grouped objects)
			( byref id as string ) _						' Assign it a string representation
			as physicsObjectList ptr
	
	declare sub removeList( byref id as string ) ' Remove a physics object list (you're probably done with it's shapes)
	
	declare function objectInList _					' Find a physics object inside a list (or just check for it's existence)
			( byval objectPointer as any ptr ) _ ' The object you're searching for.
			as physicsObjectList ptr
	
	declare function eachObject _								' Loop through all physicsObjects that are of 'objectType' type; returns number of matched objects
			(byval objectType as integer, _						'  Object type filter
			byval fn as function _								'  Function callback for each object (return 1 to keep going, 0 to stop looping)
				(byval as physicsObjectList ptr, _					'   Reference to this instance of easyChipmunk
				byval as physicsObject ptr, _					'   Physics object processed in the function
				byval as any ptr) as ubyte, _					'   User data processed in the function (defaults to null)
			byval udata as any ptr = NULL _						'  Pass this data to the function (optional)
			) as integer
	
	declare function saveScene( byref path as string ) as ubyte	' Save a physics scene
	declare function loadScene( byref path as string ) as ubyte	' Load a physics scene that was saved using easyChipmunk
	
#ifdef CAN_THREAD												' Block is only included if threading is supported
	declare static sub physicsThread( byval self as any ptr )	' Local subroutine for the thread
	thread			as any ptr									' Reference to thread
	mutex			as any ptr									' Reference to mutex
	killme			as ubyte									' Flag to tell thread if we are finished simulating or not.
#endif															' End thread-supported block
	
	space			as cpSpace ptr								' Space that is being simulated
	onUpdate		as ecUpdateFunc								' Updated every cpSpaceStep() call - reference to instance is passed
	
	lists			as physicsObjectList ptr					' Object list for any bodies, shapes, or constraints
	
	currentTime		as double									' Time that last simulation step occured
	accumulator		as double									' Number of seconds the simulation is behind
	
	userData		as any ptr									' User data - handy for the update callback, perhaps
	
	pause			as ubyte									' Toggle value to tell simulation to pause or not.  Should be wrapped in the lock() / unlock() members.
private:
	_deltaTime		as cpFloat									' Real value of delta time (use for caculation in cpSpaceStep)
	_physicsTick	as double									' Simulation rate in hertz
end type

declare sub __internal_InitEC() constructor						' Internal init function to insure that chipmunk has be initialized

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
''	Physics Object
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

constructor physicsObject( byref i as string )					' * Create a new physicsObject instance
	this._id = i												' Set the id
	this._type = easyChipmunk_NONE								' Default to a null object
	this.userData = NULL										' Nullify userData
	this._allocatedUD = 0										' Reset the flag to deallocate userData upon the destructor being called
	this._next = NULL											' No next node
end constructor

destructor physicsObject()										' * Cleanup a physics object
	this._id=""													' Remove string data to avoid memory issues
	this._destruct()											' Destruct any data if needed
	if this._allocatedUD then									' If the userData needs to be deallocated
		if this.userData then deallocate this.userData			'  Deallocate userData as long as it's not NULL
		this.userData = NULL									'  Set userData as NULL, just in case
	end if														'
	if this._next then delete this._next							' Delete the next object in the list if one exists
end destructor

property physicsObject.id() as string							' * Property to read the protected _id variable
	return this._id												' Return the string
end property

property physicsObject.getPointer() as any ptr					' * Property to get a non-typed pointer of the body/shape/constraint
	select case this._type										'
	case easyChipmunk_BODY										' If this object is a cpBody
		return cptr( any ptr, this._body )						'  Return a cast()'d version of _body
	case easyChipmunk_SHAPE										' If this object is a cpShape
		return cptr( any ptr, this._shape )						'  Return a cast()'d version of _shape
	case easyChipmunk_CONSTRAINT								' If this object is a cpConstraint
		return cptr( any ptr, this._constraint )				'  Return a cast()'d version of _constraint
	end select													'
	return NULL													' Return NULL if none of the above criteria was met
end property

property physicsObject.setBody( byval b as cpBody ptr )			' * Property to convert this object into a cpBody
	this._destruct()											' Cleanup this object if it has been previously set as anything else
	this._type = easyChipmunk_BODY								' Define the type as a BODY
	this._body = b												' Set the body pointer from 'b'
end property

property physicsObject.setShape( byval s as cpShape ptr )		' * Property to convert this object into a cpShape
	this._destruct()											' Cleanup this object if it has been previously set as anything else
	this._type = easyChipmunk_SHAPE								' Define the type as a SHAPE
	this._shape = s												' Set the shape pointer from 's'
end property

property physicsObject.setConstraint( byval c as cpConstraint ptr ) ' * Property to convert this object into a cpConstraint (or cpJoint using the depreciated chipmunk API)
	this._destruct()											' Cleanup this object if it has been previously set as anything else
	this._type = easyChipmunk_CONSTRAINT						' Define the type as a CONSTRAINT
	this._constraint = c										' Set the constraint pointer from 'c'
end property

property physicsObject.getBody( ) as cpBody ptr					' * Property to get the body pointer
	if this._type <> easyChipmunk_BODY then return NULL			' If this object is not a body, return NULL
	return this._body											' If this object is a body, return the pointer
end property

property physicsObject.getShape( ) as cpShape ptr				' * Property to get the shape pointer
	if this._type <> easyChipmunk_SHAPE then return NULL		' If this object is not a shape, return NULL
	return this._shape											' If this object is a shape, return the pointer
end property

property physicsObject.getConstraint( ) as cpConstraint ptr		' * Property to get the constraint pointer
	if this._type <> easyChipmunk_CONSTRAINT then return NULL	' If this object is not a constraint, return NULL
	return this._constraint										' If this object is a constraint, return the pointer
end property

function physicsObject.isObjectType( byval typeToCheck as integer ) as ubyte ' * Function to test the type of object
	return (this._type = typeToCheck)							' Return whether or not the checked type is the same as the objects type
end function

function physicsObject.userDataAlloc( byval size as integer ) as ubyte ' * Function to allocate the user data (will safely deallocate any data)
	if size = 0 then											' If this is a deallocation
		if this.userData then deallocate this.userData			'  If there is data, deallocate it
		this.userData = NULL									'  Reset userData as NULL, just in case
		this._allocatedUD = 0									'  Make sure this isn't accidently deallocated a second time
	else														' If there is an allocate/reallocate
		dim as any ptr temp = reallocate( this.userData, size )	'  Safely reallocate the data
		if temp = NULL then return 0							'  If the allocation went bad, return 0
		this.userData = temp									'  Update the pointer if everything went well
		this._allocatedUD = 1									'  Let the object know it needs to deallocate the userData when destructed
	end if														'
	return 1													' Everything went okay if the code reaches this point
end function

sub physicsObject._destruct()									' * Subroutine to free up memory when deleting an object's internal type
	if this._type = easyChipmunk_NONE then return				' If this object hasn't been set, then there is no need to call this function
	select case this._type										' Detect the type the object is
	case easyChipmunk_BODY										' If this object is a body...
		cpBodyFree( this._body )								'  Free the body
		this._body = NULL										'  Set it as NULL (so there are no issues within the union)
	case easyChipmunk_SHAPE										' If this object is a shape
		cpShapeFree( this._shape )								'  Free the shape
		this._shape = NULL										'  Set it as NULL (so there are no issues within the union)
	case easyChipmunk_CONSTRAINT								' If this object is a constraint
		cpConstraintFree( this._constraint )					'  Free the constraint
		this._constraint = NULL									'  Set it as NULL (so there are no issues within the union)
	end select													'
end sub

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
''	Physics Object List
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

constructor physicsObjectList( byref listIdentifier as string, byval instance as easyChipmunk_ ptr )
	this.objects = NULL
	this.listId = listIdentifier
	this.ec = instance
	this._next = NULL
end constructor

destructor physicsObjectList( )
	if this.objects then delete this.objects
	if this._next then delete this._next
	this.listId = ""
end destructor

property physicsObjectList.listName() as string
	return this.listId
end property

property physicsObjectList.getObjectById(byref index as string) as physicsObject ptr
	dim as physicsObject ptr current = this.objects
	if current = NULL then return NULL
	do
		if ( current->id = index ) then return current
		current = current->_next
	loop until current = NULL
	return NULL
end property

property physicsObjectList.getObjectByPtr(byval index as any ptr) as physicsObject ptr
	dim as physicsObject ptr current = this.objects
	if current = NULL then return NULL
	do
		if ( current->getPointer = index ) then return current
		current = current->_next
	loop until current = NULL
	return NULL
end property

function physicsObjectList.addBody(byval objectPointer as cpBody ptr, byref id as string="") as physicsObject ptr
	dim as physicsObject ptr node = NULL, current = this.objects
	dim as string stringId = id
	
	if id = "" then stringId = "body_<0x" & hex( cint( objectPointer ) ) & ">"
	
	if objectPointer = NULL then return NULL
	
	node = new physicsObject( stringId )
	node->setBody = objectPointer
	node->_next = NULL
	cpSpaceAddBody( this.ec->space, node->getBody )

	if ( current = NULL ) then
		this.objects = node
		return node
	else
		do
			if ( current->_next = NULL ) then
				current->_next = node
				return node
			end if
			current = current->_next
		loop until current = NULL
	end if
	return NULL
end function

function physicsObjectList.addShape(byval objectPointer as cpShape ptr, byref id as string="") as physicsObject ptr
	dim as physicsObject ptr node = NULL, current = this.objects
	dim as string stringId = id
	
	if id = "" then stringId = "shape_<0x" & hex( cint( objectPointer ) ) & ">"
	
	if objectPointer = NULL then return NULL
	
	node = new physicsObject( stringId )
	node->setShape = objectPointer
	node->_next = NULL
	cpSpaceAddShape( this.ec->space, node->getShape )

	if ( current = NULL ) then
		this.objects = node
		return node
	else
		do
			if ( current->_next = NULL ) then
				current->_next = node
				return node
			end if
			current = current->_next
		loop until current = NULL
	end if
	return NULL
end function

function physicsObjectList.addConstraint(byval objectPointer as cpConstraint ptr, byref id as string="") as physicsObject ptr
	dim as physicsObject ptr node = NULL, current = this.objects
	dim as string stringId = id
	
	if id = "" then stringId = "object_<0x" & hex( cint( objectPointer ) ) & ">"
	
	if objectPointer = NULL then return NULL
	
	node = new physicsObject( stringId )
	node->setConstraint = objectPointer
	node->_next = NULL
	open cons for output as #1
		print #1, "Adding a constraint to this objects list (" & this.listId & ")"
		print #1, "Space: 0x" & hex( cint( this.ec->space ) ), "Constraint: 0x" & hex( cint( objectPointer ) )
    print #1, "Constraint bodies: body<0x" & hex( cint( objectPointer->a ) ) & ">-=-body<0x" & hex( cint( objectPointer->b ) ) & ">"
	cpSpaceAddConstraint( this.ec->space, node->getConstraint )
		print #1, "++++"
	close #1

	if ( current = NULL ) then
		this.objects = node
		return node
	else
		do
			if ( current->_next = NULL ) then
				current->_next = node
				return node
			end if
			current = current->_next
		loop until current = NULL
	end if
	return NULL
end function

sub physicsObjectList.removeObjectById( byref id as string )
	if this.objects = NULL then return
	dim as physicsObject ptr current = this.objects, prev = NULL
	do
		if ( current->id = id ) then
			if prev = NULL then this.objects = current->_next else prev->_next = current->_next
			current->_next = NULL
			if current->isObjectType( easyChipmunk_BODY ) then
				cpSpaceRemoveBody( this.ec->space, current->getBody )
			elseif current->isObjectType( easyChipmunk_SHAPE ) then
				cpSpaceRemoveShape( this.ec->space, current->getShape )
			elseif current->isObjectType( easyChipmunk_CONSTRAINT ) then
				cpSpaceRemoveConstraint( this.ec->space, current->getConstraint )
			end if
			delete current
		end if
		prev = current
		current = current->_next
	loop until current = NULL
	return
end sub

sub physicsObjectList.removeObjectByPtr( byval physObj as physicsObject ptr )
	if this.objects = NULL then return
	dim as physicsObject ptr current = this.objects, prev = NULL
	do
		if ( current->getPointer = physObj ) then
			if prev = NULL then this.objects = current->_next else prev->_next = current->_next
			current->_next = NULL
			if current->isObjectType( easyChipmunk_BODY ) then
				cpSpaceRemoveBody( this.ec->space, current->getBody )
			elseif current->isObjectType( easyChipmunk_SHAPE ) then
				cpSpaceRemoveShape( this.ec->space, current->getShape )
			elseif current->isObjectType( easyChipmunk_CONSTRAINT ) then
				cpSpaceRemoveConstraint( this.ec->space, current->getConstraint )
			end if
			delete current
		end if
		prev = current
		current = current->_next
	loop until current = NULL
	return
end sub

function physicsObjectList.hasObject( byval objectPointer as any ptr ) as ubyte
	dim as physicsObject ptr current = this.objects
	if current = NULL then return 0
	do
		if current->getPointer = objectPointer then return 1
		current = current->_next
	loop until current = NULL
	return 0
end function

function physicsObjectList.eachObject(byval objectType as integer, byval fn as function( byval as physicsObjectList ptr, byval as physicsObject ptr, byval as any ptr) as ubyte, byval udata as any ptr = NULL ) as integer
	dim as physicsObject ptr current = this.objects
	if current = NULL then return NULL
	dim count as integer = 0
	do
		if current->isObjectType( objectType ) then
			var r = fn( @this, current, udata )
			count += 1
			if r = 0 then exit do
		end if
		current = current->_next
	loop until current = NULL
	return count
end function

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
''	Easy Chipmunk
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

constructor easyChipmunk()										' * Create a new instance of easyChipmunk
	this.onUpdate = NULL										' Don't use an update function by default
	this.userData = NULL										' Quick and easily accessible user data
	this.space = cpSpaceNew()									' Create a new space.  This will usually need to be setup further after an instance is declared (wrap in lock() / unlock() members)
	this.currentTime = Timer									' Set the currentTime - there could be issues if this isn't set to a time relatively soon to the simulation
	this.accumulator = 0.0										' Reset second/deltatime accumulator
	this.physicsTick = 60.0										' Set the physics tickrate to 60 and calculate the deltatime
	this.pause = 1												' By default, the simulation is paused
#ifdef CAN_THREAD												' Block is only included if threading is supported
	this.killme = 0												' Reset thread kill flag
	this.mutex = mutexcreate()									' Create a new mutex
	this.thread = threadcreate( @physicsThread, @this )			' Create the physics thread
#endif															' End thread-supported block
	this.lists = NULL											' Clear all the lists
end constructor

destructor easyChipmunk()										' * Clean up this instance of easyChipmunk
#ifdef CAN_THREAD												' Block is only included if threading is supported
	this.lock()													' Lock the thread
		this.killme = 1											' Set thread kill flag
	this.unlock()												' Unlock the thread
	threadwait( this.thread )									' Wait for the thread to finish up
	mutexdestroy( this.mutex )									' Destroy the mutex
#endif															' End thread-supported block
	if this.lists then delete this.lists						' Free all associated lists
	cpSpaceFree( this.space )									' Free the space
end destructor

function easyChipmunk.isUsingThread() as ubyte					' * Function to see if threading is supported
	' This probably doesn't have to be a function, but I'm awesome enough that
	' I made it one anyway.
#ifdef CAN_THREAD												' Block is only included if threading is supported
	return 1													' Return "true" - threading is being used!
#else															' Block is only included if threading is NOT supported
	return 0													' Return "false" - threading is not available!
#endif															' End thread-supported block
end function

sub easyChipmunk.simulate()										' * Subroutine to advance the simulation
	' This function is a dummy function on platforms that do support threading,
	' however, it should still be used to make sure there are no problems compiling
	' and using this software on platforms that do not support threading.
#ifndef CAN_THREAD												' Block is only included if threading is NOT supported
	this._DOSIMULATIONSTEP										' Manually do a simulation step
#endif															' End thread-supported block
end sub

sub easyChipmunk._DOSIMULATIONSTEP()							' * Subroutine to manually advance the simulation
	dim as double newTime = Timer()								' Grab the current time
	dim as double ftime = newTime - this.currentTime			' Calculate the elapsed time (since last call, in seconds)
	this.accumulator += ftime									' Add the elapesed time to the accumulator
	if not this.pause then										' Make sure the simulation isn't paused
		while this.accumulator >= this.deltaTime				' Loop while there is at least time for one cpSpaceStep with the current delta time
			cpSpaceStep( this.space, this.deltaTime )			' Do one space step
			if this.onUpdate then this.onUpdate( @this )
			this.accumulator -= this.deltaTime					' Take away one timestep (deltatime) from the accumulator
		wend
	end if
	this.currentTime = newTime									' Update the current time
end sub

property easyChipmunk.physicsTick() as double					' * Property to hide the _physicsTick variable
	return this._physicsTick									' just return the value
end property

property easyChipmunk.physicsTick( byval newValue as double )	' * Property to hide the _physicsTick variable so that _deltatime is calculated properly (wrap this with the lock() / unlock() members)
	this._physicsTick = newValue								' Set the new tick rate in hertz
	this._deltaTime = 1.0 / newValue							' Calculate the correct deltatime value
	this.unlock()
end property

property easyChipmunk.deltaTime() as double						' * Property to hide the _deltatime variable so it isn't monkeyed with (no ability to directly write to the variable)
	return this._deltaTime										' just return the value
end property

sub easyChipmunk.lock()											' * Subroutine to lock the thread's mutex
	' This function is a dummy function on platforms that do not support threading,
	' however, it should still be used to make sure there are no problems compiling
	' and using this software on platforms that do support threading.
#ifdef CAN_THREAD												' Block is only included if threading is supported
	mutexlock( this.mutex )										' Lock the mutex
#endif															' End thread-supported block
end sub

sub easyChipmunk.unlock()										' * Subroutine to unlock the thread's mutex
	' This function is a dummy function on platforms that do not support threading,
	' however, it should still be used to make sure there are no problems compiling
	' and using this software on platforms that do support threading.
#ifdef CAN_THREAD												' Block is only included if threading is supported
	mutexunlock( this.mutex )									' Unlock the mutex
#endif															' End thread-supported block
end sub

property easyChipmunk.list( byref id as string ) as physicsObjectList ptr
	if this.lists = NULL then
		return this.newList( id )
	end if
	dim as physicsObjectList ptr node = this.lists
	do
		if node->listName = id then return node
		node = node->_next
	loop until node = NULL
	return this.newList( id )
end property

function easyChipmunk.newList ( byref id as string ) as physicsObjectList ptr
	if id = "" then return NULL
	dim as physicsObjectList ptr node = this.lists
	if node = NULL then
		this.lists = new physicsObjectList( id, @this )
		return this.lists
	end if
	do
		if node->_next = NULL then
			node->_next = new physicsObjectList( id, @this )
			return node->_next
		end if
		if node->listName = id then return node
		node = node->_next
	loop until node = NULL
	' Should never get this far...
	return NULL
end function

sub easyChipmunk.removeList( byref id as string )
	if this.lists = NULL then return
	dim as physicsObjectList ptr current = this.lists, prev = NULL
	do
		if current->listName = id then
			if prev = NULL then this.lists = current->_next else prev->_next = current->_next
			current->_next = NULL
			delete current
			return
		end if
		prev = current
		current = current->_next
	loop until current = NULL
end sub

function easyChipmunk.objectInList ( byval objectPointer as any ptr ) as physicsObjectList ptr
	if this.lists = NULL then return NULL
	dim as physicsObjectList ptr node = this.lists
	dim as integer count = 0
	do
		if node->hasObject( objectPointer ) then return node
		node = node->_next
	loop until node = NULL
	return NULL
end function

function easyChipmunk.eachObject(byval objectType as integer, byval fn as function(byval as physicsObjectList ptr, byval as physicsObject ptr, byval as any ptr) as ubyte, byval udata as any ptr ) as integer
	if this.lists = NULL then return 0
	dim as physicsObjectList ptr node = this.lists
	dim as integer count = 0
	do
		count += node->eachObject( objectType, fn, udata )
		node = node->_next
	loop until node = NULL
	return count
end function

function easyChipmunk.saveScene( byref path as string ) as ubyte
	return 0
end function

function easyChipmunk.loadScene( byref path as string ) as ubyte
	return 0
end function

#ifdef CAN_THREAD												' Block is only included if threading is supported
sub easyChipmunk.physicsThread( byval self as any ptr )			' * Subroutine that is called as a thread (has to be static to keep fbc/createthread happy)
	dim physics as easyChipmunk ptr								' Create a easyChipmunk pointer reference
	physics = cptr( easyChipmunk ptr, self )					' Convert the self parameter back into an easyChipmunk instance
	dim quitThread as ubyte = 0									' Variable to track whether or not the thread has recieved the kill flag
	while quitThread = 0										' Do a loop while no kill flag has been found
		physics->lock()											' Lock the main thread out from tampering with the physics while it is updated
			physics->_DOSIMULATIONSTEP()						' Manually do one simulation step
			if physics->killme = 1 then quitThread = 1			' If the thread kill flag has been set, then inform the loop to quit
		physics->unlock()										' Unlock the physics mutex, so the main thread can tamper with the physics
		sleep 1, 1												' Sleep the absolute smallest amount the OS allows, so the CPU doesn't overwork itself.
	wend
end sub
#endif

sub __internal_InitEC() constructor
	cpInitChipmunk()
end sub

#endif ' __EASYCHIPMUNK
