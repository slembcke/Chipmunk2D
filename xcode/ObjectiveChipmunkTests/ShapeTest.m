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

@interface ShapeTest : XCTestCase {}
@end

@implementation ShapeTest

#define TestAccessors(o, p, v) o.p = v; XCTAssertEqual(o.p, v, @"");

static void
testPropertiesHelper(id self, ChipmunkBody *body, ChipmunkShape *shape)
{
	XCTAssertNotEqual(shape.shape, NULL, @"");
	XCTAssertEqual(body, shape.body, @"");
	XCTAssertNil(shape.userData, @"");
	XCTAssertFalse(shape.sensor, @"");
	XCTAssertEqual(shape.elasticity, (cpFloat)0, @"");
	XCTAssertEqual(shape.friction, (cpFloat)0, @"");
	XCTAssertEqual(shape.surfaceVelocity, cpvzero, @"");
	XCTAssertNil(shape.collisionType, @"");
	XCTAssertEqual(shape.filter, CP_SHAPE_FILTER_ALL, @"");
	
	cpBB bb = [shape cacheBB];
	XCTAssertEqual(shape.bb, bb, @"");
	
	TestAccessors(shape, userData, @"object");
	TestAccessors(shape, sensor, YES);
	TestAccessors(shape, elasticity, (cpFloat)0);
	TestAccessors(shape, friction, (cpFloat)0);
	TestAccessors(shape, surfaceVelocity, cpv(5,6));
	TestAccessors(shape, collisionType, @"type");
	cpShapeFilter f = {@"group", 456, 789};
	TestAccessors(shape, filter, f);
}

-(void)testProperties {
	ChipmunkBody *body = [ChipmunkBody bodyWithMass:1 andMoment:1];
	
	ChipmunkCircleShape *circle = [ChipmunkCircleShape circleWithBody:body radius:1 offset:cpv(1,2)];
	testPropertiesHelper(self, body, circle);
	XCTAssertEqual(circle.radius, (cpFloat)1, @"");
	XCTAssertEqual(circle.offset, cpv(1,2), @"");
	
	XCTAssertTrue([circle pointQuery:cpv(1,2)].distance <= 0.0f, @"");
	XCTAssertTrue([circle pointQuery:cpv(1,2.9)].distance <= 0.0f, @"");
	XCTAssertFalse([circle pointQuery:cpv(1,3.1)].distance <= 0.0f, @"");
	
	
	ChipmunkSegmentShape *segment = [ChipmunkSegmentShape segmentWithBody:body from:cpvzero to:cpv(1,0) radius:1];
	testPropertiesHelper(self, body, segment);
	XCTAssertEqual(segment.a, cpvzero, @"");
	XCTAssertEqual(segment.b, cpv(1,0), @"");
	XCTAssertEqual(segment.normal, cpv(0,-1), @"");
	
	XCTAssertTrue([segment pointQuery:cpvzero].distance <= 0.0f, @"");
	XCTAssertTrue([segment pointQuery:cpv(1,0)].distance <= 0.0f, @"");
	XCTAssertTrue([segment pointQuery:cpv(0.5, 0.5)].distance <= 0.0f, @"");
	XCTAssertFalse([segment pointQuery:cpv(0,3)].distance <= 0.0f, @"");
	
	ChipmunkPolyShape *poly = [ChipmunkPolyShape boxWithBody:body width:10 height:10 radius:0.0f];
	testPropertiesHelper(self, body, poly);
	XCTAssertTrue([poly pointQuery:cpv(0,0)].distance <= 0.0f, @"");
	XCTAssertTrue([poly pointQuery:cpv(3,3)].distance <= 0.0f, @"");
	XCTAssertFalse([poly pointQuery:cpv(-10,0)].distance <= 0.0f, @"");
	
	// TODO should add segment query tests
}

-(void)testSpace {
	// TODO
}

@end
