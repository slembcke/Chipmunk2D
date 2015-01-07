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
#import "ObjectiveChipmunk/ChipmunkAutoGeometry.h"


@interface MiscTest : XCTestCase {}
@end

@implementation MiscTest

#define AssertNearlyZero(__exp__) XCTAssertTrue((__exp__) < 1e-5, @"")

-(void)testSlerp {
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(1.0), 1.0);
		cpVect c = a;
		cpVect v = cpvslerp(a, b, 0.0);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(1.0), 1.0);
		cpVect c = b;
		cpVect v = cpvslerp(a, b, 1.0);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(1.0), 1.0);
		cpVect c = cpvmult(cpvforangle(0.5), 1.0);
		cpVect v = cpvslerp(a, b, 0.5);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(-1.0), 1.0);
		cpVect b = cpvmult(cpvforangle( 1.0), 1.0);
		cpVect c = cpvmult(cpvforangle( 0.0), 1.0);
		cpVect v = cpvslerp(a, b, 0.5);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(M_PI/2.0), 2.0);
		cpVect c = cpvadd(cpvmult(a, cpfcos(M_PI/4.0)), cpvmult(b, cpfsin(M_PI/4.0)));
		cpVect v = cpvslerp(a, b, 0.5);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = a;
		cpVect c = a;
		cpVect v = cpvslerp(a, b, 0.5);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	// TODO should it handle this?
//	{
//		cpVect a = cpv( 1.0, 0.01);
//		cpVect b = cpv(-1.0, 0.0);
//		cpVect v = cpvslerp(a, b, 0.5);
//		GHAssertLessThan(cpvdot(a, v));
//		GHAssertLessThan(cpvdot(b, v));
//		GHAssertLessThan(cpvlength(v) - 1.0);
//	}
	
	// Slerp const
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(M_PI/2.0), 1.0);
		cpVect c = cpvadd(cpvmult(a, cpfcos(M_PI/4.0)), cpvmult(b, cpfsin(M_PI/4.0)));
		cpVect v = cpvslerpconst(a, b, M_PI/4.0);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(M_PI/2.0), 1.0);
		cpVect c = b;
		cpVect v = cpvslerpconst(a, b, M_PI/2.0);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(M_PI/2.0), 1.0);
		cpVect c = b;
		cpVect v = cpvslerpconst(a, b, INFINITY);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(M_PI/2.0), 1.0);
		cpVect c = a;
		cpVect v = cpvslerpconst(a, b, 0);
		AssertNearlyZero(cpvdist(v, c));
	}
	
	{
		cpVect a = cpvmult(cpvforangle(0.0), 1.0);
		cpVect b = cpvmult(cpvforangle(M_PI/2.0), 1.0);
		cpVect c = cpvmult(cpvforangle(M_PI/4.0), 1.0);
		cpVect v = cpvslerpconst(a, b, M_PI/4.0);
		AssertNearlyZero(cpvdist(v, c));
	}
}

-(void)testImageSamplerLA
{
	{
		NSBundle *bundle = [NSBundle bundleForClass:self.class];
		CGImageRef image = [ChipmunkImageSampler loadImage:[bundle URLForResource:@"TestImageLA" withExtension:@"png"]];
		ChipmunkAbstractSampler *sampler = [[ChipmunkImageSampler alloc] initWithImage:image isMask:TRUE contextWidth:0 contextHeight:0];
		
		XCTAssertEqualWithAccuracy([sampler sample:cpv(0.5, 0.5)], (cpFloat)0.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(0.5, 3.5)], (cpFloat)1.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(3.5, 0.5)], (cpFloat)1.0, 1e-5, @"");
		
		XCTAssertEqualWithAccuracy([sampler sample:cpv(2.0 - 1e-5, 0.5)], (cpFloat)0.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(2.0 + 1e-5, 0.5)], (cpFloat)1.0, 1e-5, @"");
		
		XCTAssertEqualWithAccuracy([sampler sample:cpv(0.5, 2.0 - 1e-5)], (cpFloat)0.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(0.5, 2.0 + 1e-5)], (cpFloat)1.0, 1e-5, @"");
		
		[sampler release];
	}
	
	{
		NSBundle *bundle = [NSBundle bundleForClass:self.class];
		CGImageRef image = [ChipmunkImageSampler loadImage:[bundle URLForResource:@"TestImageLA" withExtension:@"png"]];
		ChipmunkAbstractSampler *sampler = [[ChipmunkImageSampler alloc] initWithImage:image isMask:FALSE contextWidth:0 contextHeight:0];
		
		XCTAssertEqualWithAccuracy([sampler sample:cpv(0.5, 0.5)], (cpFloat)1.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(0.5, 3.5)], (cpFloat)1.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(3.5, 0.5)], (cpFloat)1.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(3.5, 3.5)], (cpFloat)0.0, 1e-5, @"");
		
		XCTAssertEqualWithAccuracy([sampler sample:cpv(2.0 - 1e-5, 3.5)], (cpFloat)1.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(2.0 + 1e-5, 3.5)], (cpFloat)0.0, 1e-5, @"");
		
		XCTAssertEqualWithAccuracy([sampler sample:cpv(3.5, 2.0 - 1e-5)], (cpFloat)1.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy([sampler sample:cpv(3.5, 2.0 + 1e-5)], (cpFloat)0.0, 1e-5, @"");
		
		[sampler release];
	}
}

-(void)testMultiGrabSort
{
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	ChipmunkMultiGrab *multiGrab = [[ChipmunkMultiGrab alloc] initForSpace:space withSmoothing:0.0 withGrabForce:1.0];
	
	ChipmunkBody *body = [space add:[ChipmunkBody bodyWithMass:1.0 andMoment:1.0]];
	ChipmunkShape *big = [space add:[ChipmunkCircleShape circleWithBody:body radius:10.0 offset:cpvzero]];
	ChipmunkShape *small = [space add:[ChipmunkCircleShape circleWithBody:body radius:5.0 offset:cpvzero]];
	
	// Used for the custom sorting orders.
	big.userData = @0;
	small.userData = @1;
	
	ChipmunkGrab *grab1 = [multiGrab beginLocation:cpvzero];
	XCTAssertEqual(grab1.grabbedShape, big, @"Should have grabbed 'big' since it has the largest penetration depth.");
	
	multiGrab.grabSort = ^(ChipmunkShape *shape, cpFloat depth){
		NSNumber *n = shape.userData;
		return (cpFloat)n.floatValue;
	};
	
	// Should grab small since it's sorting order will be the largest;
	ChipmunkGrab *grab2 = [multiGrab beginLocation:cpvzero];
	XCTAssertEqual(grab2.grabbedShape, small, @"Should have grabbed 'small' since it has the highest custom sort value.");
	
	[multiGrab release];
	[space release];
}

-(void)testPolyArea
{
	cpFloat area1 = cpAreaForCircle(0.0, 1.0);
	cpFloat area2 = cpAreaForPoly(1, (cpVect[]){cpvzero}, 1.0);
	XCTAssertEqualWithAccuracy(area1, area2, 1e-3, @"");
	
	cpFloat area3 = cpAreaForSegment(cpv(-1,0), cpv(1,0), 1.0);
	cpFloat area4 = cpAreaForPoly(2, (cpVect[]){cpv(-1,0), cpv(1,0)}, 1.0);
	XCTAssertEqualWithAccuracy(area3, area4, 1e-3, @"");
	
	cpFloat area5 = area1 + 4.0;
	cpFloat area6 = cpAreaForPoly(2, (cpVect[]){cpv(-1,-1), cpv(1,-1), cpv(1,1), cpv(-1,1)}, 1.0);
	XCTAssertEqualWithAccuracy(area5, area6, 1e-3, @"");
}



@end
