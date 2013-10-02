#import <XCTest/XCTest.h>
#import "ObjectiveChipmunk.h"


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

@end
