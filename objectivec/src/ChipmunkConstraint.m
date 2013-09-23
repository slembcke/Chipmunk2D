// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#define CP_ALLOW_PRIVATE_ACCESS 1
#import "ObjectiveChipmunk.h"

@interface ChipmunkSpace(DoubleDispatch)

- (ChipmunkConstraint *)addConstraint:(ChipmunkConstraint *)obj;
- (ChipmunkConstraint *)removeConstraint:(ChipmunkConstraint *)obj;

@end

@implementation ChipmunkConstraint

@synthesize userData = _userData;

+(ChipmunkConstraint *)constraintFromCPConstraint:(cpConstraint *)constraint
{	
	ChipmunkConstraint *obj = constraint->userData;
	cpAssertHard([obj isKindOfClass:[ChipmunkConstraint class]], "'constraint->data' is not a pointer to a ChipmunkConstraint object.");
	
	return obj;
}

- (void) dealloc
{
	cpConstraint *constraint = self.constraint;
	[self.bodyA release];
	[self.bodyB release];
	cpConstraintDestroy(constraint);
	
	[super dealloc];
}

- (cpConstraint *)constraint
{
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}

// accessor macros
#define getter(type, lower, upper) \
- (type)lower {return self.constraint->lower;}
#define setter(type, lower, upper) \
- (void)set##upper:(type)value {self.constraint->lower = value;};
#define both(type, lower, upper) \
getter(type, lower, upper) \
setter(type, lower, upper)

both(cpFloat, maxForce, MaxForce)
both(cpFloat, errorBias, ErrorBias)
both(cpFloat, maxBias, MaxBias)

-(cpFloat)impulse {return cpConstraintGetImpulse(self.constraint);}

-(ChipmunkSpace *)space {
	cpSpace *space = cpConstraintGetSpace(self.constraint);
	return (ChipmunkSpace *)(space ? cpSpaceGetUserData(space) : nil);
}

- (ChipmunkBody *)bodyA
{
	cpBody *body = self.constraint->a;
	return (body ? body->userData : nil);
}

//- (void)setBodyA:(ChipmunkBody *)value {
//	if(self.bodyA != value){
//		[self.bodyA release];
//		self.constraint->a = [[value retain] body];
//	}
//}

- (ChipmunkBody *)bodyB
{
	cpBody *body = self.constraint->b;
	return (body ? body->userData : nil);
}

//- (void)setBodyB:(ChipmunkBody *)value {
//	if(self.bodyB != value){
//		[self.bodyB release];
//		self.constraint->b = [[value retain] body];
//	}
//}

- (NSArray *)chipmunkObjects {return [NSArray arrayWithObject:self];}
- (void)addToSpace:(ChipmunkSpace *)space {[space addConstraint:self];}
- (void)removeFromSpace:(ChipmunkSpace *)space {[space removeConstraint:self];}

-(void)preSolve:(ChipmunkSpace *)space {}
-(void)postSolve:(ChipmunkSpace *)space {}

// MARK: Callbacks
static void
PreSolve(cpConstraint *constraint, cpSpace *space)
{
	[(ChipmunkConstraint *)constraint->userData preSolve:(ChipmunkSpace *)space->userData];
}

static void
PostSolve(cpConstraint *constraint, cpSpace *space)
{
	[(ChipmunkConstraint *)constraint->userData postSolve:(ChipmunkSpace *)space->userData];
}

// Check if the method was overridden.
// No reason to add the extra method overhead if it's not needed.
-(BOOL)methodIsOverriden:(SEL)selector
{
	return ([self methodForSelector:selector] != [[ChipmunkConstraint class] instanceMethodForSelector:selector]);
}

-(void)setupCallbacks
{
	if([self methodIsOverriden:@selector(preSolve:)]){
		cpConstraintSetPreSolveFunc(self.constraint, PreSolve);
	}
	
	if([self methodIsOverriden:@selector(postSolve:)]){
		cpConstraintSetPostSolveFunc(self.constraint, PostSolve);
	}
}

@end

// accessor macros
#define getter2(type, struct, lower, upper) \
- (type)lower {return struct##Get##upper((cpConstraint *)&_constraint);}
#define setter2(type, struct, lower, upper) \
- (void)set##upper:(type)value {struct##Set##upper((cpConstraint *)&_constraint, value);};
#define both2(type, struct, lower, upper) \
getter2(type, struct, lower, upper) \
setter2(type, struct, lower, upper)


@implementation ChipmunkPinJoint

+ (id)pinJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2
{
	return [[[self alloc] initWithBodyA:a bodyB:b anchr1:anchr1 anchr2:anchr2] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpPinJointInit(&_constraint, a.body, b.body, anchr1, anchr2);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpVect, cpPinJoint, anchr1, Anchr1)
both2(cpVect, cpPinJoint, anchr2, Anchr2)
both2(cpFloat, cpPinJoint, dist, Dist)

@end


@implementation ChipmunkSlideJoint

+ (id)slideJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2 min:(cpFloat)min max:(cpFloat)max
{
	return [[[self alloc] initWithBodyA:a bodyB:b anchr1:anchr1 anchr2:anchr2 min:min max:max] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2 min:(cpFloat)min max:(cpFloat)max
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpSlideJointInit(&_constraint, a.body, b.body, anchr1, anchr2, min, max);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpVect, cpSlideJoint, anchr1, Anchr1)
both2(cpVect, cpSlideJoint, anchr2, Anchr2)
both2(cpFloat, cpSlideJoint, min, Min)
both2(cpFloat, cpSlideJoint, max, Max)

@end


@implementation ChipmunkPivotJoint

+ (id)pivotJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2
{
	return [[[self alloc] initWithBodyA:a bodyB:b anchr1:anchr1 anchr2:anchr2] autorelease];
}

+ (id)pivotJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b pivot:(cpVect)pivot
{
	return [[[self alloc] initWithBodyA:a bodyB:b pivot:pivot] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpPivotJointInit(&_constraint, a.body, b.body, anchr1, anchr2);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b pivot:(cpVect)pivot
{
	return [self initWithBodyA:a bodyB:b anchr1:[a worldToLocal:pivot] anchr2:[b worldToLocal:pivot]];
}

both2(cpVect, cpPivotJoint, anchr1, Anchr1)
both2(cpVect, cpPivotJoint, anchr2, Anchr2)

@end


@implementation ChipmunkGrooveJoint

+ (id)grooveJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b groove_a:(cpVect)groove_a groove_b:(cpVect)groove_b anchr2:(cpVect)anchr2
{
	return [[[self alloc] initWithBodyA:a bodyB:b groove_a:groove_a groove_b:groove_b anchr2:anchr2] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b groove_a:(cpVect)groove_a groove_b:(cpVect)groove_b anchr2:(cpVect)anchr2
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpGrooveJointInit(&_constraint, a.body, b.body, groove_a, groove_b, anchr2);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpVect, cpGrooveJoint, grooveA, GrooveA)
both2(cpVect, cpGrooveJoint, grooveB, GrooveB)
both2(cpVect, cpPivotJoint, anchr2, Anchr2)

@end


@implementation ChipmunkDampedSpring

+ (id)dampedSpringWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2 restLength:(cpFloat)restLength stiffness:(cpFloat)stiffness damping:(cpFloat)damping
{
	return [[[self alloc] initWithBodyA:a bodyB:b anchr1:anchr1 anchr2:anchr2 restLength:restLength stiffness:stiffness damping:damping] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b anchr1:(cpVect)anchr1 anchr2:(cpVect)anchr2 restLength:(cpFloat)restLength stiffness:(cpFloat)stiffness damping:(cpFloat)damping
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpDampedSpringInit(&_constraint, a.body, b.body, anchr1, anchr2, restLength, stiffness, damping);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpVect, cpDampedSpring, anchr1, Anchr1)
both2(cpVect, cpDampedSpring, anchr2, Anchr2)
both2(cpFloat, cpDampedSpring, restLength, RestLength)
both2(cpFloat, cpDampedSpring, stiffness, Stiffness)
both2(cpFloat, cpDampedSpring, damping, Damping)

@end


@implementation ChipmunkDampedRotarySpring

+ (id)dampedRotarySpringWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b restAngle:(cpFloat)restAngle stiffness:(cpFloat)stiffness damping:(cpFloat)damping
{
	return [[[self alloc] initWithBodyA:a bodyB:b restAngle:restAngle stiffness:stiffness damping:damping] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b restAngle:(cpFloat)restAngle stiffness:(cpFloat)stiffness damping:(cpFloat)damping
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpDampedRotarySpringInit(&_constraint, a.body, b.body, restAngle, stiffness, damping);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpFloat, cpDampedRotarySpring, restAngle, RestAngle)
both2(cpFloat, cpDampedRotarySpring, stiffness, Stiffness)
both2(cpFloat, cpDampedRotarySpring, damping, Damping)

@end


@implementation ChipmunkRotaryLimitJoint

+ (id)rotaryLimitJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b min:(cpFloat)min max:(cpFloat)max
{
	return [[[self alloc] initWithBodyA:a bodyB:b min:min max:max] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b min:(cpFloat)min max:(cpFloat)max
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpRotaryLimitJointInit(&_constraint, a.body, b.body, min, max);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpFloat, cpRotaryLimitJoint, min, Min)
both2(cpFloat, cpRotaryLimitJoint, max, Max)

@end


@implementation ChipmunkSimpleMotor

+ (id)simpleMotorWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b rate:(cpFloat)rate
{
	return [[[self alloc] initWithBodyA:a bodyB:b rate:rate] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b rate:(cpFloat)rate
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpSimpleMotorInit(&_constraint, a.body, b.body, rate);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpFloat, cpSimpleMotor, rate, Rate)

@end


@implementation ChipmunkGearJoint

+ (id)gearJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b phase:(cpFloat)phase ratio:(cpFloat)ratio
{
	return [[[self alloc] initWithBodyA:a bodyB:b phase:phase ratio:ratio] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b phase:(cpFloat)phase ratio:(cpFloat)ratio
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpGearJointInit(&_constraint, a.body, b.body, phase, ratio);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpFloat, cpGearJoint, phase, Phase)
both2(cpFloat, cpGearJoint, ratio, Ratio)

@end


@implementation ChipmunkRatchetJoint

+ (id)ratchetJointWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b phase:(cpFloat)phase ratchet:(cpFloat)ratchet
{
	return [[[self alloc] initWithBodyA:a bodyB:b phase:phase ratchet:ratchet] autorelease];
}

- (cpConstraint *)constraint {return (cpConstraint *)&_constraint;}

- (id)initWithBodyA:(ChipmunkBody *)a bodyB:(ChipmunkBody *)b phase:(cpFloat)phase ratchet:(cpFloat)ratchet
{
	if((self = [super init])){
		[a retain];
		[b retain];
		cpRatchetJointInit(&_constraint, a.body, b.body, phase, ratchet);
		self.constraint->userData = self;
		
		[self setupCallbacks];
	}
	
	return self;
}

both2(cpFloat, cpRatchetJoint, angle, Angle)
both2(cpFloat, cpRatchetJoint, phase, Phase)
both2(cpFloat, cpRatchetJoint, ratchet, Ratchet)

@end
