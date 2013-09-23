#include "SimpleTestCase.h"

#define CP_ALLOW_PRIVATE_ACCESS
#include "ObjectiveChipmunk.h"

@interface SpaceTest : SimpleTestCase {}
@end

@implementation SpaceTest

#define TestAccessors(o, p, v) o.p = v; GHAssertEquals(o.p, v, nil);
#define AssertRetainCount(obj, count) GHAssertEquals([obj retainCount], (NSUInteger)count, nil)

-(void)testProperties {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	GHAssertEquals(space.gravity, cpvzero, nil);
	GHAssertEquals(space.damping, (cpFloat)1.0, nil);
	GHAssertEquals(space.idleSpeedThreshold, (cpFloat)0, nil);
	GHAssertEquals(space.sleepTimeThreshold, (cpFloat)INFINITY, nil);
	
	GHAssertNotNULL(space.space, nil);
	GHAssertNotNil(space.staticBody, nil);
	
	TestAccessors(space, iterations, 50);
	TestAccessors(space, gravity, cpv(1,2));
	TestAccessors(space, damping, (cpFloat)5);
	TestAccessors(space, idleSpeedThreshold, (cpFloat)5);
	TestAccessors(space, sleepTimeThreshold, (cpFloat)5);
	
	[space release];
}

static NSSet *
nearestPointQueryInfoToShapes(NSArray *arr)
{
	NSMutableSet *set = [NSMutableSet setWithCapacity:[arr count]];
	for(ChipmunkNearestPointQueryInfo *info in arr)[set addObject:info.shape];
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
	ChipmunkShape *box = [space add:[[ChipmunkPolyShape alloc] initBoxWithBody:body width:1 height:1]];
	
	NSSet *set;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	// Point queries
	set = nearestPointQueryInfoToShapes([space nearestPointQueryAll:cpvzero maxDistance:0.0 layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:segment, box, nil]), nil);
	
	set = nearestPointQueryInfoToShapes([space nearestPointQueryAll:cpv(1,1) maxDistance:0.0 layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, nil]), nil);
	
	set = nearestPointQueryInfoToShapes([space nearestPointQueryAll:cpv(0.4, 0.4) maxDistance:0.0 layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), nil);
	
	set = nearestPointQueryInfoToShapes([space nearestPointQueryAll:cpv(-0.5, -0.5) maxDistance:0.0 layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:segment, nil]), nil);
	
	set = nearestPointQueryInfoToShapes([space nearestPointQueryAll:cpv(-1,-1) maxDistance:0.0 layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:nil]), nil);
	
	cpSpaceNearestPointQuery_b(space.space, cpv(-0.6, -0.6), 0.0, CP_ALL_LAYERS, CP_NO_GROUP, ^(cpShape *shape, cpFloat d, cpVect p){
		GHAssertEquals(shape, segment.shape, nil);
		GHAssertEqualsWithAccuracy(cpvdist(p, cpvnormalize(cpv(-1, -1))), (cpFloat)0.0, 1e-5, nil);
		GHAssertEqualsWithAccuracy(d, cpfsqrt(2*0.6*0.6) - 1.0f, 1e-5, nil);
	});
	
	// Segment queries
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(-2,-2) to:cpv(4,4) layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), nil);
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(2,-2) to:cpv(-2,2) layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:segment, box, nil]), nil);
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(3,-1) to:cpv(-1,3) layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, nil]), nil);
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(2.4,-1.6) to:cpv(-1.6,2.4) layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), nil);
	
	set = segmentQueryInfoToShapes([space segmentQueryAllFrom:cpv(2,2) to:cpv(3,3) layers:CP_ALL_LAYERS group:CP_NO_GROUP]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:nil]), nil);
	
	ChipmunkSegmentQueryInfo *info;
	info = [space segmentQueryFirstFrom:cpv(-2,-2) to:cpv(1,1) layers:CP_ALL_LAYERS group:CP_NO_GROUP];
	GHAssertEqualObjects(info.shape, segment, nil, nil);
	
	info = [space segmentQueryFirstFrom:cpv(-2,-2) to:cpv(-1,-1) layers:CP_ALL_LAYERS group:CP_NO_GROUP];
	GHAssertEqualObjects(info.shape, nil, nil, nil);
	
	cpSpaceSegmentQuery_b(space.space, cpv(-1.0, -0.6), cpv(1.0, -0.6), CP_ALL_LAYERS, CP_NO_GROUP, ^(cpShape *shape, cpFloat t, cpVect n){
		GHAssertEquals(shape, segment.shape, nil);
		GHAssertEqualsWithAccuracy(cpvlength(n), 1.0f, 1e-5, nil);
		GHAssertEqualsWithAccuracy(n.y, -0.6f, 1e-5, nil);
		GHAssertEqualsWithAccuracy(cpvdist(cpv(-1.0, -0.6), n)/2.0f, t, 1e-5, nil);
	});
	
	// Segment queries starting from inside a shape
	info = [space segmentQueryFirstFrom:cpvzero to:cpv(1,1) layers:CP_ALL_LAYERS group:CP_NO_GROUP];
	GHAssertEquals(info.t, 0.0f, @"Starting inside a shape should return t=0.");
	
	info = [space segmentQueryFirstFrom:cpv(1,1) to:cpvzero layers:CP_ALL_LAYERS group:CP_NO_GROUP];
	GHAssertEquals(info.t, 0.0f, @"Starting inside a shape should return t=0.");
	
	info = [space segmentQueryFirstFrom:cpv(-0.6, -0.6) to:cpvzero layers:CP_ALL_LAYERS group:CP_NO_GROUP];
	GHAssertEquals(info.t, 0.0f, @"Starting inside a shape should return t=0.");
	GHAssertEquals(info.shape, segment, @"Should have picked the segment shape.");
	
	// Shape queries
	ChipmunkBody *queryBody = [ChipmunkBody bodyWithMass:1 andMoment:1];
	ChipmunkShape *queryShape = [ChipmunkCircleShape circleWithBody:queryBody radius:1 offset:cpvzero];
	
	queryBody.pos = cpvzero;
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), nil);
	
	queryBody.pos = cpv(1,1);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, box, nil]), nil);
	
	queryBody.pos = cpv(0,-1);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:segment, box, nil]), nil);
	
	queryBody.pos = cpv(0,-1.6);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:segment, nil]), nil);
	
	cpSpaceShapeQuery_b(space.space, queryShape.shape, ^(cpShape *shape, cpContactPointSet *points){
		GHAssertEquals(shape, segment.shape, nil);
		GHAssertEquals(points->count, 1, nil);
	});
	
	queryBody.pos = cpv(2,2);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:circle, segment, nil]), nil);
	
	queryBody.pos = cpv(4,4);
	set = shapeQueryInfoToShapes([space shapeQueryAll:queryShape]);
	GHAssertEqualObjects(set, ([NSSet setWithObjects:nil]), nil);
	
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
	
	// With regular bodies.
	testPointQueries_helper(self, space, [ChipmunkBody bodyWithMass:1 andMoment:1]);
	testPointQueries_helper(self, space, [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]]);
	
	[space release];
}

-(void)testShapes {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	ChipmunkShape *shape;
	
	// Check that static shapes get added correctly
	shape = [space add:[ChipmunkCircleShape circleWithBody:space.staticBody radius:1 offset:cpvzero]];
	GHAssertTrue(cpSpatialIndexContains(space.space->staticShapes, shape.shape, shape.shape->hashid), nil);
	
	shape = [space add:[ChipmunkCircleShape circleWithBody:[ChipmunkBody staticBody] radius:1 offset:cpvzero]];
	GHAssertTrue(cpSpatialIndexContains(space.space->staticShapes, shape.shape, shape.shape->hashid), nil);
	
	// Check that normal shapes get added correctly
	ChipmunkBody *rogue = [ChipmunkBody bodyWithMass:1 andMoment:1];
	shape = [space add:[ChipmunkCircleShape circleWithBody:rogue radius:1 offset:cpvzero]];
	GHAssertTrue(cpSpatialIndexContains(space.space->activeShapes, shape.shape, shape.shape->hashid), nil);
	
	ChipmunkBody *normal = [space add:[ChipmunkBody bodyWithMass:1 andMoment:1]];
	shape = [space add:[ChipmunkCircleShape circleWithBody:normal radius:1 offset:cpvzero]];
	GHAssertTrue(cpSpatialIndexContains(space.space->activeShapes, shape.shape, shape.shape->hashid), nil);
	
	[space release];
}

-(void)testBasicSimulation {
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	space.gravity = cpv(0, -100);
	
	[space addBounds:CGRectMake(-50, 0, 100, 100) thickness:1 elasticity:1 friction:1 layers:CP_ALL_LAYERS group:CP_NO_GROUP collisionType:nil];
	
	ChipmunkBody *ball = [space add:[ChipmunkBody bodyWithMass:1 andMoment:cpMomentForCircle(1, 0, 1, cpvzero)]];
	ball.pos = cpv(-10, 10);
	[space add:[ChipmunkCircleShape circleWithBody:ball radius:1 offset:cpvzero]];
	
	ChipmunkBody *box = [space add:[ChipmunkBody bodyWithMass:1 andMoment:cpMomentForBox(1, 2, 2)]];
	box.pos = cpv(10, 10);
	[space add:[ChipmunkPolyShape boxWithBody:box width:2 height:2]];
	
	for(int i=0; i<100; i++) [space step:0.01];
	
	space.collisionSlop = 0.5f;
	GHAssertEqualsWithAccuracy(ball.pos.y, (cpFloat)1, 1.1*space.collisionSlop, nil);
	GHAssertEqualsWithAccuracy(box.pos.y, (cpFloat)1, 1.1*space.collisionSlop, nil);
	
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
	
	GHAssertFalse(body1.isSleeping, nil);
	GHAssertFalse(body2.isSleeping, nil);
	
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
	[space addPostStepRemoval:b];
	
	return FALSE;
}

static void
VerifyContactGraph(id self, ChipmunkBody *body1, ChipmunkBody *body2)
{
	__block int counter = 0;
	
	[body1 eachArbiter:^(cpArbiter *arb){
		CHIPMUNK_ARBITER_GET_BODIES(arb, a, b);
		GHAssertTrue(a == body1 && b == body2, @"Unexpected contact graph");
		counter++;
	}];
	
	[body2 eachArbiter:^(cpArbiter *arb){
		CHIPMUNK_ARBITER_GET_BODIES(arb, a, b);
		GHAssertTrue(a == body2 && b == body1, @"Unexpected contact graph");
		counter++;
	}];
	
	GHAssertEquals(counter, 2, @"Wrong number of arbiters in contact graph.");
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
	body2.pos = cpv(0, 1.9);
	
	for(int i=0; !body1.isSleeping; i++){
		[space step:0.01];
		GHAssertLessThan(i, 100, @"body1 failed to fall asleep");
	}
	
	GHAssertTrue(body2.isSleeping, @"body2 is not sleeping");
	VerifyContactGraph(self, body1, body2);
	
	// Objects are now sleeping and have their contact graph data all set up carefully.
	
	NSString *type = @"woo";
	ChipmunkBody *body3 = [space add:[ChipmunkBody bodyWithMass:1 andMoment:INFINITY]];
	ChipmunkShape *shape = [space add:[ChipmunkCircleShape circleWithBody:body3 radius:1 offset:cpv(-0.5, 0)]];
	shape.collisionType = type;
	
	[space addCollisionHandler:self typeA:nil typeB:type begin:@selector(beginSleepSensorRemoveBug:space:) preSolve:nil postSolve:nil separate:nil];
	
	// Now step again and shape should get removed.
	[space step:0.01];
	
	GHAssertFalse([space contains:shape], @"'shape' did not get removed.");
	VerifyContactGraph(self, body1, body2);
}

-(bool)beginSleepActivateOnImpact:(cpArbiter *)arb space:(ChipmunkSpace*)space
{
	// 'b' is the shape we are using to trigger the callback.
	CHIPMUNK_ARBITER_GET_BODIES(arb, sleeping, awake);
	
	GHAssertTrue(sleeping.isSleeping, @"Body 'sleeping' should be sleeping.");
	GHAssertFalse(awake.isSleeping, @"Body 'awake' should not be sleeping.");
	
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
	
	GHAssertFalse(body1.isSleeping, @"body1 did not awake.");
	GHAssertFalse(body2.isSleeping, @"body2 did not awake.");
}


-(void)testAddBounds
{
	ChipmunkSpace *space = [[ChipmunkSpace alloc] init];
	NSArray *objs = [space addBounds:CGRectMake(0, 0, 10, 10) thickness:5 elasticity:0 friction:1 layers:CP_ALL_LAYERS group:CP_NO_GROUP collisionType:nil];
	
	[space release];
}
// TODO more sleeping tests

@end
