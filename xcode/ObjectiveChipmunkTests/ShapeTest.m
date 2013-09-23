#include "SimpleTestCase.h"

#include "ObjectiveChipmunk.h"

@interface ShapeTest : SimpleTestCase {}
@end

@implementation ShapeTest

#define TestAccessors(o, p, v) o.p = v; GHAssertEquals(o.p, v, nil);

static void
testPropertiesHelper(id self, ChipmunkBody *body, ChipmunkShape *shape)
{
	GHAssertNotNULL(shape.shape, nil);
	GHAssertEquals(body, shape.body, nil);
	GHAssertNil(shape.data, nil);
	GHAssertFalse(shape.sensor, nil);
	GHAssertEquals(shape.elasticity, (cpFloat)0, nil);
	GHAssertEquals(shape.friction, (cpFloat)0, nil);
	GHAssertEquals(shape.surfaceVel, cpvzero, nil);
	GHAssertNil(shape.collisionType, nil);
	GHAssertNil(shape.group, nil);
	GHAssertEquals(shape.layers, CP_ALL_LAYERS, nil);
	
	cpBB bb = [shape cacheBB];
	GHAssertEquals(shape.bb, bb, nil);
	
	TestAccessors(shape, data, @"object");
	TestAccessors(shape, sensor, YES);
	TestAccessors(shape, elasticity, (cpFloat)0);
	TestAccessors(shape, friction, (cpFloat)0);
	TestAccessors(shape, surfaceVel, cpv(5,6));
	TestAccessors(shape, collisionType, @"type");
	TestAccessors(shape, group, @"group");
	TestAccessors(shape, layers, (cpLayers)123);
}

-(void)testProperties {
	ChipmunkBody *body = [ChipmunkBody bodyWithMass:1 andMoment:1];
	
	ChipmunkCircleShape *circle = [ChipmunkCircleShape circleWithBody:body radius:1 offset:cpv(1,2)];
	testPropertiesHelper(self, body, circle);
	GHAssertEquals(circle.radius, (cpFloat)1, nil);
	GHAssertEquals(circle.offset, cpv(1,2), nil);
	
	GHAssertTrue([circle nearestPointQuery:cpv(1,2)].dist <= 0.0f, nil);
	GHAssertTrue([circle nearestPointQuery:cpv(1,2.9)].dist <= 0.0f, nil);
	GHAssertFalse([circle nearestPointQuery:cpv(1,3.1)].dist <= 0.0f, nil);
	
	
	ChipmunkSegmentShape *segment = [ChipmunkSegmentShape segmentWithBody:body from:cpvzero to:cpv(1,0) radius:1];
	testPropertiesHelper(self, body, segment);
	GHAssertEquals(segment.a, cpvzero, nil);
	GHAssertEquals(segment.b, cpv(1,0), nil);
	GHAssertEquals(segment.normal, cpv(-0.0,1), nil);
	
	GHAssertTrue([segment nearestPointQuery:cpvzero].dist <= 0.0f, nil);
	GHAssertTrue([segment nearestPointQuery:cpv(1,0)].dist <= 0.0f, nil);
	GHAssertTrue([segment nearestPointQuery:cpv(0.5, 0.5)].dist <= 0.0f, nil);
	GHAssertFalse([segment nearestPointQuery:cpv(0,3)].dist <= 0.0f, nil);
	
	ChipmunkPolyShape *poly = [ChipmunkPolyShape boxWithBody:body width:10 height:10];
	testPropertiesHelper(self, body, poly);
	GHAssertTrue([poly nearestPointQuery:cpv(0,0)].dist <= 0.0f, nil);
	GHAssertTrue([poly nearestPointQuery:cpv(3,3)].dist <= 0.0f, nil);
	GHAssertFalse([poly nearestPointQuery:cpv(-10,0)].dist <= 0.0f, nil);
	
	// TODO should add segment query tests
}

-(void)testSpace {
	// TODO
}

@end