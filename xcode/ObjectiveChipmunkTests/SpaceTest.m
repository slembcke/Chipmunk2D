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

@interface SpaceTest : XCTestCase {}
@end

@implementation SpaceTest

#define TestAccessors(o, p, v) o.p = v; XCTAssertEqual(o.p, v, @"");
#define AssertRetainCount(obj, count) XCTAssertEqual([obj retainCount], (NSUInteger)count, @"")

-(void)testProperties {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	XCTAssertEqual(space.gravity, cpvzero, @"");
	XCTAssertEqual(space.damping, (cpFloat)1.0, @"");
	XCTAssertEqual(space.idleSpeedThreshold, (cpFloat)0, @"");
	XCTAssertEqual(space.sleepTimeThreshold, (cpFloat)INFINITY, @"");
	
	XCTAssertNotEqual(space.space, NULL, @"");
	XCTAssertNotNil(space.staticBody, @"");
	
	TestAccessors(space, iterations, 50);
	TestAccessors(space, gravity, cpv(1,2));
	TestAccessors(space, damping, (cpFloat)5);
	TestAccessors(space, idleSpeedThreshold, (cpFloat)5);
	TestAccessors(space, sleepTimeThreshold, (cpFloat)5);
	
	[space release];
}

static NSSet *
pointQueryInfoToShapes(NSArray *arr)
{
	NSMutableSet *set = [NSMutableSet setWithCapacity:[arr count]];
	for(ChipmunkPointQueryInfo *info in arr)[set addObject:info.shape];
	return set;
}

static NSSet *
segmentQueryInfoToShapes(NSArray *arr)
{
	NSMutableSet *set = [NSMutableSet setWithCapacity:[arr count]];
	for(ChipmunkSegmentQueryInfo *info in arr)[set addObject:info.shape];
	return set;
}

static NSSet *
shapeQueryInfoToShapes(NSArray *arr)
{
	NSMutableSet *set = [NSMutableSet setWithCapacity:[arr count]];
	for(ChipmunkShapeQueryInfo *info in arr)[set addObject:info.shape];
	return set;
}

static void
testPointQueries_helper(id self, ChipmunkSpace *space, ChipmunkBody *body)
{
	ChipmunkShape *circle = [space add:[[ChipmunkCircleShape alloc] initWithBody:body radius:1 offset:cpv(1,1)]];
	ChipmunkShape *segment = [space add:[[ChipmunkSegmentShape alloc] initWithBody:body from:cpvzero to:cpv(1,1) radius:1]];
	ChipmunkShape *box = [space add:[[ChipmunkPolyShape alloc] initBoxWithBody:body width:1 height:1 radius:0]];
	
	NSSet *set;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	// Point queries
	set = pointQueryInfoToShapes([space pointQueryAll:cpvzero maxDistance:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:segment, box, nil]), @"");
	
	set = pointQueryInfoToShapes([space pointQueryAll:cpv(1,1) maxDistance:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, nil]), @"");
	
	set = pointQueryInfoToShapes([space pointQueryAll:cpv(0.4, 0.4) maxDistance:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), @"");
	
	set = pointQueryInfoToShapes([space pointQueryAll:cpv(-0.5, -0.5) maxDistance:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:segment, nil]), @"");
	
	set = pointQueryInfoToShapes([space pointQueryAll:cpv(-1,-1) maxDistance:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:nil]), @"");
	
	cpSpacePointQuery_b(space.space, cpv(-0.6, -0.6), 0.0, CP_SHAPE_FILTER_ALL, ^(cpShape *shape, cpVect p, cpFloat d, cpVect g){
		XCTAssertEqual(shape, segment.shape, @"");
		XCTAssertEqualWithAccuracy(cpvdist(p, cpvnormalize(cpv(-1, -1))), (cpFloat)0.0, 1e-5, @"");
		XCTAssertEqualWithAccuracy(d, cpfsqrt(2*0.6*0.6) - 1.0f, 1e-5, @"");
	});
	
	ChipmunkPointQueryInfo *pointInfo = nil;
	pointInfo = [space pointQueryNearest:cpv(-3, -3) maxDistance:10 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertEqual(pointInfo.shape, segment, @"");
	
	// A point query that misses should return nil.
	pointInfo = [space pointQueryNearest:cpv(-3, -3) maxDistance:0 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertNil(pointInfo, @"");
	
	// Segment queries
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(-2,-2) to:cpv(4,4) radius:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), @"");
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(2,-2) to:cpv(-2,2) radius:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:segment, box, nil]), @"");
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(3,-1) to:cpv(-1,3) radius:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, nil]), @"");
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(2.4,-1.6) to:cpv(-1.6,2.4) radius:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), @"");
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(2,2) to:cpv(3,3) radius:0.0 filter:CP_SHAPE_FILTER_ALL]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:nil]), @"");
	
	ChipmunkSegmentQueryInfo *segmentInfo = nil;
	segmentInfo = [space segmentQueryFirstFrom:cpv(-2,-2) to:cpv(1,1) radius:0.0 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertEqual(segmentInfo.shape, segment, @"");
	
	segmentInfo = [space segmentQueryFirstFrom:cpv(-2,-2) to:cpv(-1,-1) radius:0.0 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertEqualObjects(segmentInfo.shape, nil, @"");
	
	cpSpaceSegmentQuery_b(space.space, cpv(-1.0, -0.6), cpv(1.0, -0.6), 0.0f, CP_SHAPE_FILTER_ALL, ^(cpShape *shape, cpVect p, cpVect n, cpFloat t){
		XCTAssertEqual(shape, segment.shape, @"");
		XCTAssertEqualWithAccuracy(cpvlength(n), 1.0f, 1e-5, @"");
		XCTAssertEqualWithAccuracy(n.y, -0.6f, 1e-5, @"");
		XCTAssertEqualWithAccuracy(cpvdist(cpv(-1.0, -0.6), n)/2.0f, t, 1e-5, @"");
	});
	
	// Segment queries starting from inside a shape
	segmentInfo = [space segmentQueryFirstFrom:cpvzero to:cpv(1,1) radius:0.0 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertEqual(segmentInfo.t, (cpFloat)0, @"Starting inside a shape should return t=0.");
	
	segmentInfo = [space segmentQueryFirstFrom:cpv(1,1) to:cpvzero radius:0.0 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertEqual(segmentInfo.t, (cpFloat)0, @"Starting inside a shape should return t=0.");
	
	segmentInfo = [space segmentQueryFirstFrom:cpv(-0.6, -0.6) to:cpvzero radius:0.0 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertEqual(segmentInfo.t, (cpFloat)0, @"Starting inside a shape should return t=0.");
	XCTAssertEqual(segmentInfo.shape, segment, @"Should have picked the segment shape.");
	
	// A segment query that misses should return nil.
	segmentInfo = [space segmentQueryFirstFrom:cpv(-10, 0) to:cpv(-10, -10) radius:0.0 filter:CP_SHAPE_FILTER_ALL];
	XCTAssertNil(segmentInfo, @"");
	
	// Shape queries
	ChipmunkBody *queryBody = [ChipmunkBody bodyWithMass:1 andMoment:1];
	ChipmunkShape *queryShape = [ChipmunkCircleShape circleWithBody:queryBody radius:1 offset:cpvzero];
	
	queryBody.position = cpvzero;
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), @"");
	
	queryBody.position = cpv(1,1);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), @"");
	
	queryBody.position = cpv(0,-1);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:segment, box, nil]), @"");
	
	queryBody.position = cpv(0,-1.6);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:segment, nil]), @"");
	
	cpSpaceShapeQuery_b(space.space, queryShape.shape, ^(cpShape *shape, cpContactPointSet *points){
		XCTAssertEqual(shape, segment.shape, @"");
		XCTAssertEqual(points->count, 1, @"");
	});
	
	queryBody.position = cpv(2,2);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, nil]), @"");
	
	queryBody.position = cpv(4,4);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	XCTAssertEqualObjects(set, ([NSSet setWithObjects:nil]), @"");
	
	[space remove:circle];
	[space remove:segment];
	[space remove:box];
	[pool release];
	
	AssertRetainCount(circle, 1);
	AssertRetainCount(segment, 1);
	AssertRetainCount(box, 1);
	
	[circle release];
	[segment release];
	[box release];
}

-(void)testPointQueries {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	
	// With static bodies
	testPointQueries_helper(self, space, space.staticBody);
	testPointQueries_helper(self, space, [ChipmunkBody staticBody]);
	
	// Kinematic bodies
	testPointQueries_helper(self, space, [space add:[ChipmunkBody kinematicBody]]);
	
	// With regular bodies.
	testPointQueries_helper(self, space, [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]]);
	
	// Rogue bodies
	testPointQueries_helper(self, space, [ChipmunkBody bodyWithMass:1 andMoment:1]);
	
	[space release];
}

-(void)testShapes {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	ChipmunkShape *shape;
	
	// Check that static shapes get added correctly
	shape = [space add:[ChipmunkCircleShape circleWithBody:space.staticBody radius:1 offset:cpvzero]];
	XCTAssertTrue(cpSpatialIndexContains(space.space->staticShapes, shape.shape, shape.shape->hashid), @"");
	
	shape = [space add:[ChipmunkCircleShape circleWithBody:[ChipmunkBody staticBody] radius:1 offset:cpvzero]];
	XCTAssertTrue(cpSpatialIndexContains(space.space->staticShapes, shape.shape, shape.shape->hashid), @"");
	
	// Check that normal shapes get added correctly
	ChipmunkBody *rogue = [ChipmunkBody bodyWithMass:1 andMoment:1];
	shape = [space add:[ChipmunkCircleShape circleWithBody:rogue radius:1 offset:cpvzero]];
	XCTAssertTrue(cpSpatialIndexContains(space.space->dynamicShapes, shape.shape, shape.shape->hashid), @"");
	
	ChipmunkBody *normal = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	shape = [space add:[ChipmunkCircleShape circleWithBody:normal radius:1 offset:cpvzero]];
	XCTAssertTrue(cpSpatialIndexContains(space.space->dynamicShapes, shape.shape, shape.shape->hashid), @"");
	
	[space release];
}

-(void)testBasicSimulation {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.gravity = cpv(0, -100);
	
	[space addBounds:cpBBNew(-50, 0, 50, 100) thickness:1 elasticity:1 friction:1 filter:CP_SHAPE_FILTER_ALL collisionType:nil];
	
	ChipmunkBody *ball = [space add:[ChipmunkBody bodyWithMass:1 andMoment:cpMomentForCircle(1, 0, 1, cpvzero)]];
	ball.position = cpv(-10, 10);
	[space add:[ChipmunkCircleShape circleWithBody:ball radius:1 offset:cpvzero]];
	
	ChipmunkBody *box = [space add:[ChipmunkBody bodyWithMass:1 andMoment:cpMomentForBox(1, 2, 2)]];
	box.position = cpv(10, 10);
	[space add:[ChipmunkPolyShape boxWithBody:box width:2 height:2 radius:0]];
	
	for(int i=0; i<100; i++) [space step:0.01];
	
	XCTAssertTrue(cpfabs(ball.position.y - 1) < 1.1*space.collisionSlop, @"");
	XCTAssertTrue(cpfabs(box.position.y - 1) < 1.1*space.collisionSlop, @"");
	
	[space release];
}

- (void)testInitialSleepingObjects
{
	// http://www.cocos2d-iphone.org/forum/topic/28896#post-142428
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.sleepTimeThreshold = 10.0;
	
	ChipmunkBody *body1 = [space add:[ChipmunkBody bodyWithMass:1.0 andMoment:1.0]];
	[space add:[ChipmunkCircleShape circleWithBody:body1 radius:1.0 offset:cpvzero]];
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody bodyWithMass:1.0 andMoment:1.0]];
	[space add:[ChipmunkCircleShape circleWithBody:body2 radius:1.0 offset:cpvzero]];
	
	[body1 sleep];
	[space step:1.0];
	
	XCTAssertFalse(body1.isSleeping, @"");
	XCTAssertFalse(body2.isSleeping, @"");
	
	[space release];
}

-(void)testInitStepFree
{
	// This was crashing on GCC.
	// Possible uninitialized field?
	cpSpace *space = cpSpaceNew();
	cpSpaceStep(space, 1);
	cpSpaceFree(space);
}

-(bool)beginSleepSensorRemoveBug:(cpArbiter *)arb space:(ChipmunkSpace*)space
{
	// 'b' is the shape we are using to trigger the callback.
	CHIPMUNK_ARBITER_GET_SHAPES(arb, a, b);
	NSLog(@"space: %p, arb: %p, a: %p, b: %p", space.space, arb, a.shape, a.body);
	[space addPostStepRemoval:b];
	
	return FALSE;
}

static void
VerifyContactGraph(id self, ChipmunkBody *body1, ChipmunkBody *body2)
{
	__block int counter = 0;
	
	[body1 eachArbiter:^(cpArbiter *arb){
		CHIPMUNK_ARBITER_GET_BODIES(arb, a, b);
		XCTAssertTrue(a == body1 && b == body2, @"Unexpected contact graph");
		counter++;
	}];
	
	[body2 eachArbiter:^(cpArbiter *arb){
		CHIPMUNK_ARBITER_GET_BODIES(arb, a, b);
		XCTAssertTrue(a == body2 && b == body1, @"Unexpected contact graph");
		counter++;
	}];
	
	XCTAssertEqual(counter, 2, @"Wrong number of arbiters in contact graph.");
}

// This one was a doozy to find. :-\
// http://chipmunk-physics.net/forum/viewtopic.php?f=1&t=2472&start=40#p10924
-(void)testSleepSensorRemoveBug
{
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.sleepTimeThreshold = 0.1;
	
	ChipmunkBody *body1 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:INFINITY]];
	[space add:[ChipmunkCircleShape circleWithBody:body1 radius:1 offset:cpvzero]];
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:INFINITY]];
	[space add:[ChipmunkCircleShape circleWithBody:body2 radius:1 offset:cpvzero]];
	body2.position = cpv(0, 1.9);
	
	for(int i=0; !body1.isSleeping; i++){
		[space step:0.01];
		XCTAssertTrue(i < 100, @"body1 failed to fall asleep");
	}
	
	XCTAssertTrue(body2.isSleeping, @"body2 is not sleeping");
	VerifyContactGraph(self, body1, body2);
	
	// Objects are now sleeping and have their contact graph data all set up carefully.
	
	NSString *type = @"woo";
	ChipmunkBody *body3 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:INFINITY]];
	ChipmunkShape *shape = [space add:[ChipmunkCircleShape circleWithBody:body3 radius:1 offset:cpv(-0.5, 0)]];
	shape.collisionType = type;
	
	[space addCollisionHandler:self typeA:nil typeB:type begin:@selector(beginSleepSensorRemoveBug:space:) preSolve:nil postSolve:nil separate:nil];
	
	// Now step again and shape should get removed.
	[space step:0.01];
	
	XCTAssertFalse([space contains:shape], @"'shape' did not get removed.");
	VerifyContactGraph(self, body1, body2);
}

-(bool)beginSleepActivateOnImpact:(cpArbiter *)arb space:(ChipmunkSpace*)space
{
	// 'b' is the shape we are using to trigger the callback.
	CHIPMUNK_ARBITER_GET_BODIES(arb, sleeping, awake);
	
	XCTAssertTrue(sleeping.isSleeping, @"Body 'sleeping' should be sleeping.");
	XCTAssertFalse(awake.isSleeping, @"Body 'awake' should not be sleeping.");
	
	[sleeping activate];
	
	return TRUE;
}

// Causing a sleeping body to be activated from a pre-solve callback causes issues with the deffered waking.
// http://chipmunk-physics.net/forum/viewtopic.php?f=1&t=2477
-(void)testSleepActivateOnImpact
{
	NSString *sleepType = @"sleeping";
	NSString *awakeType = @"awake";
	
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.sleepTimeThreshold = 0.1;
	
	ChipmunkBody *body1 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:INFINITY]];
	ChipmunkShape *shape1 = [space add:[ChipmunkCircleShape circleWithBody:body1 radius:1 offset:cpvzero]];
	shape1.collisionType = sleepType;
	[body1 sleep];
	
	ChipmunkBody *body2 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:INFINITY]];
	ChipmunkShape *shape2 = [space add:[ChipmunkCircleShape circleWithBody:body2 radius:1 offset:cpvzero]];
	shape2.collisionType = awakeType;
	
	[space addCollisionHandler:self typeA:sleepType typeB:awakeType begin:@selector(beginSleepActivateOnImpact:space:) preSolve:nil postSolve:nil separate:nil];
	[space step:0.01];
	
	XCTAssertFalse(body1.isSleeping, @"body1 did not awake.");
	XCTAssertFalse(body2.isSleeping, @"body2 did not awake.");
}


-(void)testAddBounds
{
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	NSArray *objs = [space addBounds:cpBBNew(0, 0, 10, 10) thickness:5 elasticity:0 friction:1 filter:CP_SHAPE_FILTER_ALL collisionType:nil];
	XCTAssertTrue([space contains:objs], @"");
	
	[space release];
}
// TODO more sleeping tests

@end
