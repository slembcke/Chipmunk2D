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

@interface UpdateTestBody : ChipmunkBody

@property(nonatomic, assign) BOOL velocityUpdated;
@property(nonatomic, assign) BOOL positionUpdated;

@end


@implementation UpdateTestBody

-(void)updateVelocity:(cpFloat)dt gravity:(cpVect)gravity damping:(cpFloat)damping
{
	self.velocityUpdated = TRUE;
}

-(void)updatePosition:(cpFloat)dt
{
	self.positionUpdated = TRUE;
}

@end


@interface BodyTest : XCTestCase {}
@end

@implementation BodyTest

#define TestAccessors(o, p, v) o.p = v; XCTAssertEqual(o.p, v, @"");
#define TestAccessorsV(o, p, v) o.p = v; XCTAssertTrue(cpveql(o.p, v), @"");

-(void)testProperties {
	ChipmunkBody *body = [ChipmunkBody bodyWithMass:123 andMoment:123];
	XCTAssertEqual(body.mass, (cpFloat)123, @"");
	XCTAssertEqual(body.moment, (cpFloat)123, @"");
	
	XCTAssertNotEqual(body.body, NULL, @"");
	XCTAssertNil(body.userData, @"");
	
	TestAccessors(body, userData, @"object");
	TestAccessors(body, mass, (cpFloat)5);
	TestAccessors(body, moment, (cpFloat)5);
	TestAccessorsV(body, position, cpv(5,6));
	TestAccessorsV(body, velocity, cpv(5,6));
	TestAccessorsV(body, force, cpv(5,6));
	TestAccessors(body, angle, (cpFloat)5);
	TestAccessors(body, angularVelocity, (cpFloat)5);
	TestAccessors(body, torque, (cpFloat)5);
	
	body.angle = 0;
	body.angle = CP_PI;
	body.angle = CP_PI/2.0f;
	// TODO transform tests
	
	XCTAssertFalse(body.isSleeping, @"");
	XCTAssertEqual(body.type, CP_BODY_TYPE_DYNAMIC, @"");
}

-(void)testBasic {
	ChipmunkBody *body = [ChipmunkBody bodyWithMass:1 andMoment:1];
	
	[body applyForce:cpv(0,1) atWorldPoint:cpv(1,0)];
	XCTAssertTrue(body.force.y > 0, @"");
	XCTAssertTrue(body.force.x == 0, @"");
	XCTAssertTrue(body.torque > 0, @"");
	
	body.force = cpvzero;
	body.torque = 0.0f;
	XCTAssertTrue(cpveql(body.force, cpvzero), @"");
	XCTAssertEqual(body.torque, (cpFloat)0, @"");
	
	[body applyImpulse:cpv(0,1) atWorldPoint:cpv(1,0)];
	XCTAssertTrue(body.velocity.y > 0, @"");
	XCTAssertTrue(body.velocity.x == 0, @"");
	XCTAssertTrue(body.angularVelocity > 0, @"");
}

-(void)testMisc {
	ChipmunkBody *staticBody = [ChipmunkBody staticBody];
	XCTAssertFalse(staticBody.isSleeping, @"");
	XCTAssertEqual(staticBody.type, CP_BODY_TYPE_STATIC, @"");
	
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.sleepTimeThreshold = 1.0;
	
	ChipmunkBody *body = [ChipmunkBody bodyWithMass:1 andMoment:1];
	[space add:body];
	XCTAssertFalse(body.isSleeping, @"");
	XCTAssertEqual(body.type, CP_BODY_TYPE_DYNAMIC, @"");
	
	[body sleep];
	XCTAssertTrue(body.isSleeping, @"");
	[space release];
}

-(void)testUpdate {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	UpdateTestBody *body = [space add:[UpdateTestBody bodyWithMass:1.0 andMoment:1.0]];
	
	XCTAssertTrue(body.body->velocity_func != cpBodyUpdateVelocity, @"Did not set velocity integration callback.");
	XCTAssertTrue(body.body->position_func != cpBodyUpdatePosition, @"Did not set position integration callback.");
	
	[space step:1.0];
	XCTAssertTrue(body.velocityUpdated, @"Did not call velocity integration callback.");
	XCTAssertTrue(body.positionUpdated, @"Did not call position integration callback.");
	
	[space release];
}

-(void)testSpace {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	ChipmunkBody *body = [ChipmunkBody bodyWithMass:1.0 andMoment:1.0];
	
	XCTAssertNil(body.space, @"body.space should be nil.");
	
	[space add:body];
	XCTAssertEqual(body.space, space, @"body.space should be space.");
	
	[space release];
}

-(void)testTimeSteping {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.gravity = cpv(0, -1);
	
	ChipmunkBody *dynamicBody = [space add:[ChipmunkBody bodyWithMass:1.0 andMoment:1.0]];
	ChipmunkBody *staticBody = [space add:[ChipmunkBody staticBody]];
	ChipmunkBody *kinematicBody = [space add:[ChipmunkBody kinematicBody]];
	kinematicBody.velocity = cpv(1, 0);
	
	[space step:1.0];
	
	// Should have accelerated but not moved.
	XCTAssert(cpveql(dynamicBody.position, cpvzero), @"");
	XCTAssert(cpveql(dynamicBody.velocity, cpv(0, -1)), @"");
	
	// Should not have moved nor accelerated.
	XCTAssert(cpveql(staticBody.position, cpvzero), @"");
	XCTAssert(cpveql(staticBody.velocity, cpvzero), @"");
	
	// Should have moved right but not accelerated downward.
	XCTAssert(cpveql(kinematicBody.position, cpv(1, 0)), @"");
	XCTAssert(cpveql(kinematicBody.velocity, cpv(1, 0)), @"");
	
	[space step:1.0];
	
	// Should have moved and accelerated more.
	XCTAssert(cpveql(dynamicBody.position, cpv(0, -1)), @"");
	XCTAssert(cpveql(dynamicBody.velocity, cpv(0, -2)), @"");
	
	// Still should not have moved nor accelerated.
	XCTAssert(cpveql(staticBody.position, cpvzero), @"");
	XCTAssert(cpveql(staticBody.velocity, cpvzero), @"");
	
	// Should have moved right more but still not accelerated downward.
	XCTAssert(cpveql(kinematicBody.position, cpv(2, 0)), @"");
	XCTAssert(cpveql(kinematicBody.velocity, cpv(1, 0)), @"");
	
	[space release];
}

@end
