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

#define CP_ALLOW_PRIVATE_ACCESS 1
#import "ObjectiveChipmunk/ObjectiveChipmunk.h"

#define AssertRetainCount(obj, count) XCTAssertEqual([obj retainCount], (NSUInteger)count, @"")

@interface MemoryTest : XCTestCase {}
@end

@implementation MemoryTest

-(void)testBasic {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	
	ChipmunkBody *body1 = [[ChipmunkBody alloc] initWithMass:1 andMoment:1];
	ChipmunkBody *body2 = [[ChipmunkBody alloc] initWithMass:1 andMoment:1];
	
	ChipmunkShape *shape1 = [[ChipmunkCircleShape alloc] initWithBody:body1 radius:1 offset:cpvzero];
	ChipmunkShape *shape2 = [[ChipmunkCircleShape alloc] initWithBody:body2 radius:1 offset:cpvzero];
	
	ChipmunkConstraint *joint1 = [[ChipmunkPivotJoint alloc] initWithBodyA:body1 bodyB:body2 pivot:cpvzero];
	ChipmunkConstraint *joint2 = [[ChipmunkPivotJoint alloc] initWithBodyA:body1 bodyB:body2 pivot:cpvzero];
	
	[space add:body1];
	[space add:body2];
	[space add:shape1];
	[space add:shape2];
	[space add:joint1];
	[space add:joint2];
	
	AssertRetainCount(body1, 5);
	AssertRetainCount(body2, 5);
	AssertRetainCount(shape1, 2);
	AssertRetainCount(shape2, 2);
	AssertRetainCount(joint1, 2);
	AssertRetainCount(joint2, 2);
	
	[space remove:shape1];
	[space remove:joint1];
	
	AssertRetainCount(body1, 5);
	AssertRetainCount(body2, 5);
	AssertRetainCount(shape1, 1);
	AssertRetainCount(shape2, 2);
	AssertRetainCount(joint1, 1);
	AssertRetainCount(joint2, 2);
	
	[space release];
	
	AssertRetainCount(body1, 4);
	AssertRetainCount(body2, 4);
	AssertRetainCount(shape1, 1);
	AssertRetainCount(shape2, 1);
	AssertRetainCount(joint1, 1);
	AssertRetainCount(joint2, 1);
	
	[joint1 release];
	[joint2 release];
	
	AssertRetainCount(body1, 2);
	AssertRetainCount(body2, 2);
	
	[shape1 release];
	[shape2 release];
	
	AssertRetainCount(body1, 1);
	AssertRetainCount(body2, 1);
	
	[body1 release];
	[body2 release];
}

-(void)testStaticBody {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	ChipmunkBody *staticBody = space.staticBody;
	
	AssertRetainCount(staticBody, 1);
	
	ChipmunkShape *shape = [[ChipmunkCircleShape alloc] initWithBody:space.staticBody radius:1 offset:cpvzero];
	AssertRetainCount(staticBody, 2);
	
	[space add:shape];
	AssertRetainCount(shape, 2);
	AssertRetainCount(staticBody, 2);
	
	[space release];
	AssertRetainCount(shape, 1);
	AssertRetainCount(staticBody, 1);
	
	[shape release];
}

-(void)testSetters {
	ChipmunkBody *body = [[ChipmunkBody alloc] initWithMass:1.0 andMoment:1.0];
	
	
	ChipmunkShape *shape = [ChipmunkCircleShape circleWithBody:nil radius:1 offset:cpvzero];
	shape.body = body;
	AssertRetainCount(body, 2);
	
	shape.body = body;
	AssertRetainCount(body, 2);
	
	shape.body = nil;
	AssertRetainCount(body, 1);
	
	
//	ChipmunkConstraint *joint = [ChipmunkPivotJoint pivotJointWithBodyA:nil bodyB:nil pivot:cpvzero];
//	joint.bodyA = body; joint.bodyB = body;
//	AssertRetainCount(body, 3);
//	
//	joint.bodyA = body; joint.bodyB = body;
//	AssertRetainCount(body, 3);
//	
//	joint.bodyA = nil; joint.bodyB = nil;
//	AssertRetainCount(body, 1);
}

-(void)testPostStepCallbacks {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	NSObject *obj1 = [[NSObject alloc] init];
	NSObject *obj2 = [[NSObject alloc] init];
	
	// Lock the space to avoid triggering warnings.
	cpSpaceLock(space.space);
	
	// Registering the callback should retain the object twice
	XCTAssertTrue([space addPostStepCallback:obj1 selector:@selector(isEqual:) key:obj1], @"");
	AssertRetainCount(obj1, 3);
	
	// Registering the same callback a second time should not add more retains
	XCTAssertFalse([space addPostStepCallback:obj1 selector:@selector(isEqual:) key:obj1], @"");
	AssertRetainCount(obj1, 3);
	
	// A key can only be registered once to prevent double removals.
	// Registering a second target with the same key is a no-op.
	XCTAssertFalse([space addPostStepCallback:obj2 selector:@selector(isEqual:) key:obj1], @"");
	AssertRetainCount(obj1, 3);
	AssertRetainCount(obj2, 1);
	
	XCTAssertTrue([space addPostStepCallback:obj1 selector:@selector(isEqual:) key:obj2], @"");
	AssertRetainCount(obj1, 4);
	AssertRetainCount(obj2, 2);
	
	cpSpaceUnlock(space.space, FALSE);
	
	// Stepping the space should release the callback handler and both objects
	[space step:1];
	AssertRetainCount(obj1, 1);
	AssertRetainCount(obj2, 1);
	
	[space release];
	[obj1 release];
	[obj2 release];
}

@end
