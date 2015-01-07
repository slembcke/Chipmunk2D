/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#import <XCTest/XCTest.h>
#import "ObjectiveChipmunk/ObjectiveChipmunk.h"

@interface CallbacksTest : XCTestCase {}
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
	body1.position = cpv(0*radius*1.5,0);
	
	[space add:[ChipmunkCircleShape circleWithBody:body1 radius:radius offset:cpvzero]];
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	body2.position = cpv(1*radius*1.5,0);
	
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:radius offset:cpvzero]];
	
	NSMutableString *string = [NSMutableString string];
	
	cpCollisionHandler *handler = cpSpaceAddCollisionHandler(space.space, nil, nil);
	handler->beginFunc = (cpCollisionBeginFunc)Begin,
	handler->preSolveFunc = (cpCollisionPreSolveFunc)PreSolve,
	handler->postSolveFunc = (cpCollisionPostSolveFunc)PostSolve,
	handler->separateFunc = (cpCollisionSeparateFunc)Separate,
	handler->userData = string;
	
	// Test for separate callback when moving:
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-", @"");
	
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-PreSolve-PostSolve-", @"");
	
	if(separateByRemove){
		[space remove:shape2];
	} else {
		body2.position = cpv(100, 100);
		[space step:0.1];
	}
	
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-PreSolve-PostSolve-Separate-", @"");
	
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
	body1.position = cpv(0*radius*1.5,0);
	
	ChipmunkShape *shape1 = [space add:[ChipmunkCircleShape circleWithBody:body1 radius:radius offset:cpvzero]];
	shape1.collisionType = type;
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	body2.position = cpv(1*radius*1.5,0);
	
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:radius offset:cpvzero]];
	shape2.collisionType = type;
	
	NSMutableString *string = [NSMutableString string];
	
	cpCollisionHandler *handler = cpSpaceAddCollisionHandler(space.space, type, type);
	handler->beginFunc = (cpCollisionBeginFunc)Begin,
	handler->preSolveFunc = (cpCollisionPreSolveFunc)PreSolve,
	handler->postSolveFunc = (cpCollisionPostSolveFunc)PostSolve,
	handler->separateFunc = (cpCollisionSeparateFunc)Separate,
	handler->userData = string;
	
	// Test for separate callback when moving:
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-", @"");
	
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-PreSolve-", @"");
	
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-PreSolve-", @"");
	
	switch(wakeRemoveType){
		case 0:
			// Separate by removal
			[space remove:shape2];
			XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-PreSolve-Separate-", @"");
			break;
		case 1:
			// Separate by move
			body2.position = cpv(100, 100);
			[space step:0.1];
			XCTAssertEqualObjects(string, @"Begin-PreSolve-PostSolve-PreSolve-Separate-", @"");
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
	body1.position = cpv(0*radius*1.5,0);
	
	[space add:[ChipmunkCircleShape circleWithBody:body1 radius:radius offset:cpvzero]];
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody staticBody]];
	body2.position = cpv(1*radius*1.5,0);
	
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:radius offset:cpvzero]];
	shape2.sensor = true;
	
	NSMutableString *string = [NSMutableString string];
	
	cpCollisionHandler *handler = cpSpaceAddCollisionHandler(space.space, nil, nil);
	handler->beginFunc = (cpCollisionBeginFunc)Begin,
	handler->preSolveFunc = (cpCollisionPreSolveFunc)PreSolve,
	handler->postSolveFunc = (cpCollisionPostSolveFunc)PostSolve,
	handler->separateFunc = (cpCollisionSeparateFunc)Separate,
	handler->userData = string;
	
	// Test for separate callback when moving:
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-", @"");
	
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PreSolve-", @"");
	
	[space step:0.1];
	XCTAssertEqualObjects(string, @"Begin-PreSolve-PreSolve-", @"");
	
	switch(wakeRemoveType){
		case 0:
			// Separate by removal
			[space remove:shape2];
			XCTAssertEqualObjects(string, @"Begin-PreSolve-PreSolve-Separate-", @"");
			break;
		case 1:
			// Separate by move
			body1.position = cpv(100, 100);
			[space step:0.1];
			XCTAssertEqualObjects(string, @"Begin-PreSolve-PreSolve-Separate-", @"");
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

static cpBool CallBlock(cpArbiter *arb, cpSpace *space, cpBool (^block)(cpArbiter *arb)){return block(arb);}

-(void)testPostStepRemoval {
	NSString *ballType = @"ballType";
	NSString *barType = @"barType";
	
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.gravity = cpv(0, -100);
	
	// TODO
	cpCollisionHandler *handler = cpSpaceAddCollisionHandler(space.space, ballType, barType);
	handler->beginFunc = (cpCollisionBeginFunc)CallBlock,
	handler->userData = ^(cpArbiter *arb){
		CHIPMUNK_ARBITER_GET_SHAPES(arb, ballShape, barShape);
		[space addPostStepRemoval:barShape];
		
		return TRUE;
	};
	
	ChipmunkShape *shape;
	
	// The ball will stop on this bar
	shape = [space add:[ChipmunkSegmentShape segmentWithBody:space.staticBody from:cpv(-10,0) to:cpv(10,0) radius:1]];
	
	// but remove this one
	shape = [space add:[ChipmunkSegmentShape segmentWithBody:space.staticBody from:cpv(-10,2) to:cpv(10,2) radius:1]];
	shape.collisionType = barType;
	
	ChipmunkBody *ball = [space add:[ChipmunkBody bodyWithMass:1 andMoment:cpMomentForCircle(1, 0, 1, cpvzero)]];
	ball.position = cpv(0, 10);
	
	shape = [space add:[ChipmunkCircleShape circleWithBody:ball radius:1 offset:cpvzero]];
	shape.collisionType = ballType;
	
	for(int i=0; i<100; i++) [space step:0.01];
	
	XCTAssertEqualWithAccuracy(ball.position.y, (cpFloat)2, 1.1*space.collisionSlop, @"");
	
	[space release];
}

// Make sure that adding a post step callback from inside a post step callback works correctly.
-(void)testPostStepFromPostStep
{
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	
	ChipmunkShape *staticShape = [space add:[ChipmunkCircleShape circleWithBody:space.staticBody radius:1.0 offset:cpvzero]];
	staticShape.collisionType = staticShape;
	
	ChipmunkBody *body1 = [space add:[ChipmunkBody kinematicBody]];
	ChipmunkShape *shape1 = [space add:[ChipmunkCircleShape circleWithBody:body1 radius:1.0 offset:cpvzero]];
	shape1.collisionType = shape1;
	shape1.sensor = TRUE;
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody kinematicBody]];
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:10.0 offset:cpvzero]];
	shape2.collisionType = shape2;
	shape2.sensor = TRUE;
	
	__block bool trigger1 = FALSE;
	__block bool trigger2 = FALSE;
	__block bool trigger3 = FALSE;
	__block bool trigger4 = FALSE;
	__block bool trigger5 = FALSE;
	__block bool trigger6 = FALSE;
	__block bool trigger7 = FALSE;
	
	cpCollisionHandler *handler1 = cpSpaceAddCollisionHandler(space.space, staticShape, shape1);
	handler1->separateFunc = (cpCollisionSeparateFunc)CallBlock,
	handler1->userData = ^(cpArbiter *arb){
		XCTAssertTrue(cpSpaceIsLocked(space.space), @"");
		
		// When body1 moves it will trigger the first separate callback.
		[space addPostStepBlock:^{
			XCTAssertFalse(cpSpaceIsLocked(space.space), @"");
			
			// Calling remove will immediately trigger the next separate callback.
			[space remove:shape1];
			
			trigger2 = TRUE;
		} key:shape1];
		
		trigger1 = TRUE;
	};
	
	cpCollisionHandler *handler2 = cpSpaceAddCollisionHandler(space.space, shape1, shape2);
	handler2->separateFunc = (cpCollisionSeparateFunc)CallBlock,
	handler2->userData = ^(cpArbiter *arb){
		XCTAssertTrue(cpSpaceIsLocked(space.space), @"");
		
		// schedule a second post step callback within the old one with the same key.
		// This one shouldn't be called.
		[space addPostStepBlock:^{trigger4 = TRUE;} key:shape1];
		
		trigger3 = TRUE;
	};
	
	cpCollisionHandler *handler3 = cpSpaceAddCollisionHandler(space.space, staticShape, shape2);
	handler3->separateFunc = (cpCollisionSeparateFunc)CallBlock,
	handler3->userData = ^(cpArbiter *arb){
		XCTAssertTrue(cpSpaceIsLocked(space.space), @"");
		
		[space addPostStepBlock:^{
			[space addPostStepBlock:^{trigger7 = TRUE;} key:shape2];
			
			trigger6 = TRUE;
		} key:shape2];
		
		trigger5 = TRUE;
	};
	
	[space step:1.0];
	body1.position = cpv(10, 0);
	[space step:1.0];
	
	XCTAssertTrue(trigger1, @"");
	XCTAssertTrue(trigger2, @"");
	XCTAssertTrue(trigger3, @"");
	XCTAssertFalse(trigger4, @"");
	
	[space remove:shape2];
	
	XCTAssertTrue(trigger5, @"");
	XCTAssertTrue(trigger6, @"");
	XCTAssertFalse(trigger7, @"");
	
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
			XCTAssertEqual(cpBodyGetUserData(body), _body, @"");
			counter++;
		});
		
		cpSpaceEachShape_b(space.space, ^(cpShape *shape){
			if(cpBodyGetType(cpShapeGetBody(shape)) == CP_BODY_TYPE_STATIC){
				XCTAssertEqual(shape, _staticShape.shape, @"");
			} else {
				XCTAssertEqual(shape, _shape.shape, @"");
			}
			
			counter++;
		});
		
		cpSpaceEachConstraint_b(space.space, ^(cpConstraint *constraint){
			XCTAssertEqual(constraint, _constraint.constraint, @"");
			counter++;
		});
		
		
		cpBodyEachShape_b(space.staticBody.body, ^(cpShape *shape){
			XCTAssertEqual(shape, _staticShape.shape, @"");
			counter++;
		});
		
		cpBodyEachConstraint_b(space.staticBody.body, ^(cpConstraint *constraint){
			XCTAssertEqual(constraint, _constraint.constraint, @"");
			counter++;
		});
		
		
		cpBodyEachShape_b(_body.body, ^(cpShape *shape){
			XCTAssertEqual(shape, _shape.shape, @"");
			counter++;
		});
		
		cpBodyEachConstraint_b(_body.body, ^(cpConstraint *constraint){
			XCTAssertEqual(constraint, _constraint.constraint, @"");
			counter++;
		});
		
		XCTAssertEqual(counter, 0, @"");
	}
	
	[space add:_staticShape];
	[space add:_body];
	[space add:_shape];
	[space add:_constraint];
	
	{
		__block int counter = 0;
		
		cpSpaceEachBody_b(space.space, ^(cpBody *body){
			XCTAssertEqual(body, _body.body, @"");
			counter++;
		});
		
		cpSpaceEachShape_b(space.space, ^(cpShape *shape){
			if(cpBodyGetType(cpShapeGetBody(shape)) == CP_BODY_TYPE_STATIC){
				XCTAssertEqual(shape, _staticShape.shape, @"");
			} else {
				XCTAssertEqual(shape, _shape.shape, @"");
			}
			
			counter++;
		});
		
		cpSpaceEachConstraint_b(space.space, ^(cpConstraint *constraint){
			XCTAssertEqual(constraint, _constraint.constraint, @"");
			counter++;
		});
		
		
		cpBodyEachShape_b(space.staticBody.body, ^(cpShape *shape){
			XCTAssertEqual(shape, _staticShape.shape, @"");
			counter++;
		});
		
		cpBodyEachConstraint_b(space.staticBody.body, ^(cpConstraint *constraint){
			XCTAssertEqual(constraint, _constraint.constraint, @"");
			counter++;
		});
		
		
		cpBodyEachShape_b(_body.body, ^(cpShape *shape){
			XCTAssertEqual(shape, _shape.shape, @"");
			counter++;
		});
		
		cpBodyEachConstraint_b(_body.body, ^(cpConstraint *constraint){
			XCTAssertEqual(constraint, _constraint.constraint, @"");
			counter++;
		});
		
		XCTAssertEqual(counter, 8, @"");
	}
	
	[space release];
}

@end
