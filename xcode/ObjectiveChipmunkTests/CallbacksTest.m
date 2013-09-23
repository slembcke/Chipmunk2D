#include "SimpleTestCase.h"

#include "ObjectiveChipmunk.h"

@interface CallbacksTest : SimpleTestCase {}
@end

@implementation CallbacksTest

// TODO test callbacks trigger
// TODO test reject from begin
// TODO test reject from pre-solve
// TODO test sensors
// TODO test first collision
// TODO test post-step callbacks

static cpBool
Begin(cpArbiter *arb, cpSpace *space, NSMutableString *string){
	[string appendString:@"Begin-"];
	
	return cpTrue;
}

static cpBool
PreSolve(cpArbiter *arb, cpSpace *space, NSMutableString *string){
	[string appendString:@"PreSolve-"];
	
	return cpTrue;
}

static void
PostSolve(cpArbiter *arb, cpSpace *space, NSMutableString *string){
	[string appendString:@"PostSolve-"];
}

static void
Separate(cpArbiter *arb, cpSpace *space, NSMutableString *string){
	[string appendString:@"Separate-"];
}

static void
testHandlersHelper(id self, bool separateByRemove, bool enableContactGraph){
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.collisionBias = 1.0f;
	
	cpFloat radius = 5;
	
	ChipmunkBody *body1 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	body1.pos = cpv(0*radius*1.5,0);
	
	[space add:[ChipmunkCircleShape circleWithBody:body1 radius:radius offset:cpvzero]];
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	body2.pos = cpv(1*radius*1.5,0);
	
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:radius offset:cpvzero]];
	
	NSMutableString *string = [NSMutableString string];
	
	cpSpaceAddCollisionHandler(space.space, nil, nil,
		(cpCollisionBeginFunc)Begin,
		(cpCollisionPreSolveFunc)PreSolve,
		(cpCollisionPostSolveFunc)PostSolve,
		(cpCollisionSeparateFunc)Separate,
		string
	);
	
	// Test for separate callback when moving:
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-", NULL);
	
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-PreSolve-PostSolve-", NULL);
	
	if(separateByRemove){
		[space remove:shape2];
	} else {
		body2.pos = cpv(100, 100);
		[space step:0.1];
	}
	
	GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-PreSolve-PostSolve-Separate-", NULL);
	
	// Step once more to check for dangling pointers
	[space step:0.1];
	
	// Cleanup
	[space release];
}

-(void)testHandlers {
	testHandlersHelper(self, true, true);
	testHandlersHelper(self, false, true);
	testHandlersHelper(self, false, false);
	testHandlersHelper(self, true, false);
}

static void
testHandlersSleepingHelper(id self, int wakeRemoveType){
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.collisionBias = 1.0f;
	space.sleepTimeThreshold = 0.15;
	
	NSString *type = @"type";
	cpFloat radius = 5;
	
	ChipmunkBody *body1 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	body1.pos = cpv(0*radius*1.5,0);
	
	ChipmunkShape *shape1 = [space add:[ChipmunkCircleShape circleWithBody:body1 radius:radius offset:cpvzero]];
	shape1.collisionType = type;
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	body2.pos = cpv(1*radius*1.5,0);
	
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:radius offset:cpvzero]];
	shape2.collisionType = type;
	
	NSMutableString *string = [NSMutableString string];
	
	cpSpaceAddCollisionHandler(space.space, type, type,
		(cpCollisionBeginFunc)Begin,
		(cpCollisionPreSolveFunc)PreSolve,
		(cpCollisionPostSolveFunc)PostSolve,
		(cpCollisionSeparateFunc)Separate,
		string
	);
	
	// Test for separate callback when moving:
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-", NULL);
	
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-PreSolve-", NULL);
	
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-PreSolve-", NULL);
	
	switch(wakeRemoveType){
		case 0:
			// Separate by removal
			[space remove:shape2];
			GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-PreSolve-Separate-", NULL);
			break;
		case 1:
			// Separate by move
			body2.pos = cpv(100, 100);
			[space step:0.1];
			GHAssertEqualStrings(string, @"Begin-PreSolve-PostSolve-PreSolve-Separate-", NULL);
			break;
			
		default:break;
	}
	
	// Step once more to check for dangling pointers
	[space step:0.1];
	
	// Cleanup
	[space release];
}

-(void)testHandlersSleeping {
	testHandlersSleepingHelper(self, 0);
	testHandlersSleepingHelper(self, 1);
	
	// BUG if the time threshold is less than dt the bodies fall asleep the same frame after being awoken
	// Separate is not called because of the short circuit in cpSpaceArbiterSetFilter().
	// This is a weird edge case though as it's a really bad idea to use such a small threshold
}

static void
testSleepingSensorCallbacksHelper(id self, int wakeRemoveType){
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.collisionBias = 1.0f;
	space.sleepTimeThreshold = 0.15;
	
	cpFloat radius = 5;
	
	ChipmunkBody *body1 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	body1.pos = cpv(0*radius*1.5,0);
	
	[space add:[ChipmunkCircleShape circleWithBody:body1 radius:radius offset:cpvzero]];
	
	ChipmunkBody *body2 = [ChipmunkBody staticBody];
	body2.pos = cpv(1*radius*1.5,0);
	
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:radius offset:cpvzero]];
	shape2.sensor = true;
	
	NSMutableString *string = [NSMutableString string];
	
	cpSpaceAddCollisionHandler(space.space, nil, nil,
		(cpCollisionBeginFunc)Begin,
		(cpCollisionPreSolveFunc)PreSolve,
		(cpCollisionPostSolveFunc)PostSolve,
		(cpCollisionSeparateFunc)Separate,
		string
	);
	
	// Test for separate callback when moving:
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-", NULL);
	
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-PreSolve-", NULL);
	
	[space step:0.1];
	GHAssertEqualStrings(string, @"Begin-PreSolve-PreSolve-", NULL);
	
	switch(wakeRemoveType){
		case 0:
			// Separate by removal
			[space remove:shape2];
			GHAssertEqualStrings(string, @"Begin-PreSolve-PreSolve-Separate-", NULL);
			break;
		case 1:
			// Separate by move
			body1.pos = cpv(100, 100);
			[space step:0.1];
			GHAssertEqualStrings(string, @"Begin-PreSolve-PreSolve-Separate-", NULL);
			break;
			
		default:break;
	}
	
	// Step once more to check for dangling pointers
	[space step:0.1];
	
	// Cleanup
	[space release];
}

-(void)testSleepingSensorCallbacks {
	testSleepingSensorCallbacksHelper(self, 0);
	testSleepingSensorCallbacksHelper(self, 1);
}

- (bool)postStepRemovalBegin:(cpArbiter *)arbiter space:(ChipmunkSpace*)space {
	CHIPMUNK_ARBITER_GET_SHAPES(arbiter, ballShape, barShape);
	[space addPostStepRemoval:barShape];
	
	return TRUE;
}

-(void)testPostStepRemoval {
	NSString *ballType = @"ballType";
	NSString *barType = @"barType";
	
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.gravity = cpv(0, -100);
	
	[space addCollisionHandler:self
		typeA:ballType typeB:barType
		begin:@selector(postStepRemovalBegin:space:)
		preSolve:nil postSolve:nil separate:nil
	];
	
	ChipmunkShape *shape;
	
	// The ball will stop on this bar
	shape = [space add:[ChipmunkSegmentShape segmentWithBody:space.staticBody from:cpv(-10,0) to:cpv(10,0) radius:1]];
	
	// but remove this one
	shape = [space add:[ChipmunkSegmentShape segmentWithBody:space.staticBody from:cpv(-10,2) to:cpv(10,2) radius:1]];
	shape.collisionType = barType;
	
	ChipmunkBody *ball = [space add:[ChipmunkBody bodyWithMass:1 andMoment:cpMomentForCircle(1, 0, 1, cpvzero)]];
	ball.pos = cpv(0, 10);
	
	shape = [space add:[ChipmunkCircleShape circleWithBody:ball radius:1 offset:cpvzero]];
	shape.collisionType = ballType;
	
	for(int i=0; i<100; i++) [space step:0.01];
	
	cpFloat cp_collision_slop = 0.5f; // TODO relpace
	GHAssertEqualsWithAccuracy(ball.pos.y, (cpFloat)2, 1.1*cp_collision_slop, nil);
	
	[space release];
}

static void CallBlock(cpArbiter *arb, cpSpace *space, void (^block)(void)){block();}

// Make sure that adding a post step callback from inside a post step callback works correctly.
-(void)testPostStepFromPostStep
{
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	
	ChipmunkShape *staticShape = [space add:[ChipmunkCircleShape circleWithBody:space.staticBody radius:1.0 offset:cpvzero]];
	staticShape.collisionType = staticShape;
	
	ChipmunkBody *body1 = [ChipmunkBody bodyWithMass:1.0 andMoment:1.0];
	ChipmunkShape *shape1 = [space add:[ChipmunkCircleShape circleWithBody:body1 radius:1.0 offset:cpvzero]];
	shape1.collisionType = shape1;
	
	ChipmunkBody *body2 = [ChipmunkBody bodyWithMass:1.0 andMoment:1.0];
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:10.0 offset:cpvzero]];
	shape2.collisionType = shape2;
	
	__block bool trigger1 = FALSE;
	__block bool trigger2 = FALSE;
	__block bool trigger3 = FALSE;
	__block bool trigger4 = FALSE;
	
	cpSpaceAddCollisionHandler(space.space, staticShape, shape1, nil, nil, nil, (cpCollisionSeparateFunc)CallBlock, ^(){
		GHAssertTrue(cpSpaceIsLocked(space.space), nil);
		
		// When body1 moves it will trigger the first separate callback.
		[space addPostStepBlock:^{
			GHAssertFalse(cpSpaceIsLocked(space.space), nil);
			
			// Calling remove will immediately trigger the next separate callback.
			[space remove:shape1];
			
			trigger2 = TRUE;
		} key:shape1];
		
		trigger1 = TRUE;
	});
	
	cpSpaceAddCollisionHandler(space.space, shape1, shape2, nil, nil, nil, (cpCollisionSeparateFunc)CallBlock, ^(){
		GHAssertTrue(cpSpaceIsLocked(space.space), nil);
		
		// schedule a second post step callback within the old one with the same key.
		// This one shouldn't be called.
		[space addPostStepBlock:^{trigger4 = TRUE;} key:shape1];
		
		trigger3 = TRUE;
	});
	
	[space step:1.0];
	body1.pos = cpv(10, 0);
	[space step:1.0];
	
	GHAssertTrue(trigger1, nil);
	GHAssertTrue(trigger2, nil);
	GHAssertTrue(trigger3, nil);
	GHAssertFalse(trigger4, nil);
	
	__block bool trigger5 = FALSE;
	__block bool trigger6 = FALSE;
	__block bool trigger7 = FALSE;
	
	cpSpaceAddCollisionHandler(space.space, staticShape, shape2, nil, nil, nil, (cpCollisionSeparateFunc)CallBlock, ^(){
		GHAssertTrue(cpSpaceIsLocked(space.space), nil);
		
		[space addPostStepBlock:^{
			[space addPostStepBlock:^{trigger7 = TRUE;} key:shape2];
			
			trigger6 = TRUE;
		} key:shape2];
		
		trigger5 = TRUE;
	});
	
	[space remove:shape2];
	
	GHAssertTrue(trigger5, nil);
	GHAssertTrue(trigger6, nil);
	GHAssertFalse(trigger7, nil);
	
	[space release];
}

-(void)testBlockIterators {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	
	ChipmunkShape *_staticShape = [ChipmunkCircleShape circleWithBody:space.staticBody radius:1.0 offset:cpvzero];
	
	ChipmunkBody *_body = [ChipmunkBody bodyWithMass:1.0 andMoment:1.0];
	ChipmunkShape *_shape = [ChipmunkCircleShape circleWithBody:_body radius:1.0 offset:cpvzero];
	ChipmunkConstraint *_constraint = [ChipmunkPivotJoint pivotJointWithBodyA:space.staticBody bodyB:_body pivot:cpvzero];
	
	{
		__block int counter = 0;
		
		cpSpaceEachBody_b(space.space, ^(cpBody *body){
			GHAssertEquals(body, _body, nil);
			counter++;
		});
		
		cpSpaceEachShape_b(space.space, ^(cpShape *shape){
			if(cpBodyIsStatic(shape->body)){
				GHAssertEquals(shape, _staticShape.shape, nil);
			} else {
				GHAssertEquals(shape, _shape.shape, nil);
			}
			
			counter++;
		});
		
		cpSpaceEachConstraint_b(space.space, ^(cpConstraint *constraint){
			GHAssertEquals(constraint, _constraint.constraint, nil);
			counter++;
		});
		
		
		cpBodyEachShape_b(space.staticBody.body, ^(cpShape *shape){
			GHAssertEquals(shape, _staticShape.shape, nil);
			counter++;
		});
		
		cpBodyEachConstraint_b(space.staticBody.body, ^(cpConstraint *constraint){
			GHAssertEquals(constraint, _constraint.constraint, nil);
			counter++;
		});
		
		
		cpBodyEachShape_b(_body.body, ^(cpShape *shape){
			GHAssertEquals(shape, _shape.shape, nil);
			counter++;
		});
		
		cpBodyEachConstraint_b(_body.body, ^(cpConstraint *constraint){
			GHAssertEquals(constraint, _constraint.constraint, nil);
			counter++;
		});
		
		GHAssertEquals(counter, 0, nil);
	}
	
	[space add:_staticShape];
	[space add:_body];
	[space add:_shape];
	[space add:_constraint];
	
	{
		__block int counter = 0;
		
		cpSpaceEachBody_b(space.space, ^(cpBody *body){
			GHAssertEquals(body, _body.body, nil);
			counter++;
		});
		
		cpSpaceEachShape_b(space.space, ^(cpShape *shape){
			if(cpBodyIsStatic(shape->body)){
				GHAssertEquals(shape, _staticShape.shape, nil);
			} else {
				GHAssertEquals(shape, _shape.shape, nil);
			}
			
			counter++;
		});
		
		cpSpaceEachConstraint_b(space.space, ^(cpConstraint *constraint){
			GHAssertEquals(constraint, _constraint.constraint, nil);
			counter++;
		});
		
		
		cpBodyEachShape_b(space.staticBody.body, ^(cpShape *shape){
			GHAssertEquals(shape, _staticShape.shape, nil);
			counter++;
		});
		
		cpBodyEachConstraint_b(space.staticBody.body, ^(cpConstraint *constraint){
			GHAssertEquals(constraint, _constraint.constraint, nil);
			counter++;
		});
		
		
		cpBodyEachShape_b(_body.body, ^(cpShape *shape){
			GHAssertEquals(shape, _shape.shape, nil);
			counter++;
		});
		
		cpBodyEachConstraint_b(_body.body, ^(cpConstraint *constraint){
			GHAssertEquals(constraint, _constraint.constraint, nil);
			counter++;
		});
		
		GHAssertEquals(counter, 8, nil);
	}
	
	[space release];
}

@end