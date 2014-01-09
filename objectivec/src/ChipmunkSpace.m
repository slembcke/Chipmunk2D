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

#define CP_ALLOW_PRIVATE_ACCESS
#import "ObjectiveChipmunk.h"

#import <TargetConditionals.h>

#ifdef CHIPMUNK_PRO_TRIAL
#if TARGET_OS_IPHONE == 1
	#import <UIKit/UIKit.h>
#else
	#import <AppKit/AppKit.h>
#endif
#endif

// Just in case the user doesn't have -ObjC in their linker flags.
// Annoyingly, this is the case more often than not.
@interface NSArrayChipmunkObject : NSArray<ChipmunkObject>

@property(nonatomic, retain) NSArray *chipmunkObjects;

@end

@implementation NSArrayChipmunkObject

@synthesize chipmunkObjects = _chipmunkObjects;

-(id)initWithArray:(NSArray *)objects {
	if((self = [super init])){
		self.chipmunkObjects = objects;
	}
	
	return self;
}

-(NSUInteger)count
{
	return [_chipmunkObjects count];
}

-(id)objectAtIndex:(NSUInteger)index
{
	return [_chipmunkObjects objectAtIndex:index];
}

@end

@implementation NSArray(ChipmunkObject)

-(id<NSFastEnumeration>)chipmunkObjects
{
	return self;
}

@end


// Private class used to wrap the statically allocated staticBody attached to each space.
@interface _ChipmunkStaticBodySingleton : ChipmunkBody {
	cpBody *_bodyPtr;
	ChipmunkSpace *space; // weak ref
}

@end

typedef struct handlerContext {
	id delegate;
	ChipmunkSpace *space;
	cpCollisionType typeA, typeB;
	SEL beginSelector;
	bool (*beginFunc)(id self, SEL selector, cpArbiter *arb, ChipmunkSpace *space);
	SEL preSolveSelector;
	bool (*preSolveFunc)(id self, SEL selector, cpArbiter *arb, ChipmunkSpace *space);
	SEL postSolveSelector;
	void (*postSolveFunc)(id self, SEL selector, cpArbiter *arb, ChipmunkSpace *space);
	SEL separateSelector;
	void (*separateFunc)(id self, SEL selector, cpArbiter *arb, ChipmunkSpace *space);
} handlerContext;

@implementation ChipmunkSpace

#ifdef CHIPMUNK_PRO_TRIAL
static NSString *dialogTitle = @"Chipmunk Pro Trial";
static NSString *dialogMessage = @"This copy of Chipmunk Pro is a trial, please consider purchasing if you continue using it.";

+(void)initialize
{
	[super initialize];

	static BOOL done = FALSE;
	if(done) return; else done = TRUE;
	
#if TARGET_OS_IPHONE == 1
	UIAlertView *alert = [[UIAlertView alloc]
		initWithTitle:dialogTitle
		message:dialogMessage
		delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil
	];
	
	[alert show];
	[alert release];
#else
	[self performSelectorOnMainThread:@selector(dialog) withObject:nil waitUntilDone:FALSE];
#endif

}

#if TARGET_OS_IPHONE != 1
+(void)dialog
{
	[NSApplication sharedApplication];
	[[NSAlert
		alertWithMessageText:dialogTitle
		defaultButton:@"OK"
		alternateButton:nil
		otherButton:nil
		informativeTextWithFormat:dialogMessage
	] runModal];
}
#endif

#endif

+(ChipmunkSpace *)spaceFromCPSpace:(cpSpace *)space
{	
	ChipmunkSpace *obj = space->userData;
	cpAssertHard([obj isKindOfClass:[ChipmunkSpace class]], "'space->data' is not a pointer to a ChipmunkSpace object.");
	
	return obj;
}

- (id)initWithSpace:(cpSpace *)space
{
	if((self = [super init])){
		_children = [[NSMutableSet alloc] init];
		_handlers = [[NSMutableArray alloc] init];
		
		_space = space;
		cpSpaceSetUserData(_space, self);
		
		_staticBody = [[ChipmunkBody alloc] initWithMass:0.0f andMoment:0.0f];
		_staticBody.type = CP_BODY_TYPE_STATIC;
		cpSpaceSetStaticBody(_space, _staticBody.body);
	}
	
	return self;
}

- (id)init {
	// Use a fast space instead if the class is available.
	// However if you don't specify -ObjC as a linker flag the dynamic substitution won't work.
	Class hastySpace = NSClassFromString(@"ChipmunkHastySpace");
	if(hastySpace && [self isMemberOfClass:[ChipmunkSpace class]]){
		[self release];
		return [[hastySpace alloc] init];
	} else {
		return [self initWithSpace:cpSpaceNew()];
	}
}

-(void)freeSpace
{
	cpSpaceFree(_space);
}

- (void) dealloc {
	[self freeSpace];
	[_staticBody release];
	
	[_children release];
	[_handlers release];
	
	[super dealloc];
}

- (cpSpace *)space {return _space;}

@synthesize userData = _userData;

// accessor macros
#define getter(type, lower, upper) \
- (type)lower {return cpSpaceGet##upper(_space);}
#define setter(type, lower, upper) \
- (void)set##upper:(type)value {cpSpaceSet##upper(_space, value);};
#define both(type, lower, upper) \
getter(type, lower, upper) \
setter(type, lower, upper)

both(int, iterations, Iterations);
both(cpVect, gravity, Gravity);
both(cpFloat, damping, Damping);
both(cpFloat, idleSpeedThreshold, IdleSpeedThreshold);
both(cpFloat, sleepTimeThreshold, SleepTimeThreshold);
both(cpFloat, collisionSlop, CollisionSlop);
both(cpFloat, collisionBias, CollisionBias);
both(cpTimestamp, collisionPersistence, CollisionPersistence);
getter(cpFloat, currentTimeStep, CurrentTimeStep);

- (ChipmunkBody *)staticBody {return _staticBody;}

//static bool Begin(cpArbiter *arb, struct cpSpace *space, handlerContext *ctx){return ctx->beginFunc(ctx->delegate, ctx->beginSelector, arb, ctx->space);}
//static bool PreSolve(cpArbiter *arb, struct cpSpace *space, handlerContext *ctx){return ctx->preSolveFunc(ctx->delegate, ctx->preSolveSelector, arb, ctx->space);}
//static void PostSolve(cpArbiter *arb, struct cpSpace *space, handlerContext *ctx){return ctx->postSolveFunc(ctx->delegate, ctx->postSolveSelector, arb, ctx->space);}
//static void Separate(cpArbiter *arb, struct cpSpace *space, handlerContext *ctx){return ctx->separateFunc(ctx->delegate, ctx->separateSelector, arb, ctx->space);}
//
//
////#define HFUNC(fname, Fname) (fname ? (cpCollision##Fname##Func)fname : NULL)
////#define HFUNCS() \
////HFUNC(begin, Begin), \
////HFUNC(preSolve, PreSolve), \
////HFUNC(postSolve, PostSolve), \
////HFUNC(separate, Separate)
//
//// Free collision handler delegates for the given type pair
//static void
//FilterHandlers(NSMutableArray **handlers, cpCollisionType typeA, cpCollisionType typeB)
//{
//	NSMutableArray *newHandlers = [[NSMutableArray alloc] initWithCapacity:[(*handlers) count]];
//	
//	for(NSData *data in (*handlers)){
//		const handlerContext *context = [data bytes];
//		if(
//			!(context->typeA == typeA && context->typeB == typeB) &&
//			!(context->typeA == typeB && context->typeB == typeA)
//		){
//			[newHandlers addObject:data];
//		}
//	}
//	
//	[(*handlers) release];
//	(*handlers) = newHandlers;
//}
//
//- (void)setDefaultCollisionHandler:(id)delegate
//	begin:(SEL)begin
//	preSolve:(SEL)preSolve
//	postSolve:(SEL)postSolve
//	separate:(SEL)separate
//{
//	cpCollisionType sentinel = (cpCollisionType)@"DEFAULT";
//	FilterHandlers(&_handlers, sentinel, sentinel);
//	
//	handlerContext handler = {
//		delegate, self, sentinel, sentinel,
//		begin, (void *)(begin ? [delegate methodForSelector:begin] : NULL),
//		preSolve, (void *)(preSolve ? [delegate methodForSelector:preSolve] : NULL),
//		postSolve, (void *)(postSolve ? [delegate methodForSelector:postSolve] : NULL),
//		separate, (void *)(separate ? [delegate methodForSelector:separate] : NULL),
//	};
//	NSData *data = [NSData dataWithBytes:&handler length:sizeof(handler)];
//	[_handlers addObject:data];
//	
//	cpSpaceSetDefaultCollisionHandler(_space,
//		(begin ? (cpCollisionBeginFunc)Begin : NULL),
//		(preSolve ? (cpCollisionPreSolveFunc)PreSolve : NULL),
//		(postSolve ? (cpCollisionPostSolveFunc)PostSolve : NULL),
//		(separate ? (cpCollisionSeparateFunc)Separate : NULL),
//		(void *)[data bytes]
//	);
//}
//	
//- (void)addCollisionHandler:(id)delegate
//	typeA:(cpCollisionType)a typeB:(cpCollisionType)b
//	begin:(SEL)begin
//	preSolve:(SEL)preSolve
//	postSolve:(SEL)postSolve
//	separate:(SEL)separate
//{
//	[self removeCollisionHandlerForTypeA:a andB:b];
//	
//	handlerContext handler = {
//		delegate, self, a, b,
//		begin, (void *)(begin ? [delegate methodForSelector:begin] : NULL),
//		preSolve, (void *)(preSolve ? [delegate methodForSelector:preSolve] : NULL),
//		postSolve, (void *)(postSolve ? [delegate methodForSelector:postSolve] : NULL),
//		separate, (void *)(separate ? [delegate methodForSelector:separate] : NULL),
//	};
//	NSData *data = [NSData dataWithBytes:&handler length:sizeof(handler)];
//	
//	cpSpaceAddCollisionHandler(
//		_space, a, b,
//		(begin ? (cpCollisionBeginFunc)Begin : NULL),
//		(preSolve ? (cpCollisionPreSolveFunc)PreSolve : NULL),
//		(postSolve ? (cpCollisionPostSolveFunc)PostSolve : NULL),
//		(separate ? (cpCollisionSeparateFunc)Separate : NULL),
//		(void *)[data bytes]
//	);
//	
//	[_handlers addObject:data];
//}

- (id)add:(NSObject<ChipmunkObject> *)obj
{
	if([obj conformsToProtocol:@protocol(ChipmunkBaseObject)]){
		[(NSObject<ChipmunkBaseObject> *)obj addToSpace:self];
	} else if([obj conformsToProtocol:@protocol(ChipmunkObject)]){
		for(NSObject<ChipmunkBaseObject> *child in [obj chipmunkObjects]) [self add:child];
	} else {
		[NSException raise:@"NSArgumentError" format:@"Attempted to add an object of type %@ to a ChipmunkSpace.", [obj class]];
	}
	
	[_children addObject:obj];
	return obj;
}

- (id)remove:(NSObject<ChipmunkObject> *)obj
{
	if([obj conformsToProtocol:@protocol(ChipmunkBaseObject)]){
		[(NSObject<ChipmunkBaseObject> *)obj removeFromSpace:self];
	} else if([obj conformsToProtocol:@protocol(ChipmunkObject)]){
		for(NSObject<ChipmunkBaseObject> *child in [obj chipmunkObjects]) [self remove:child];
	} else {
		[NSException raise:@"NSArgumentError" format:@"Attempted to remove an object of type %@ from a ChipmunkSpace.", [obj class]];
	}
	
	[_children removeObject:obj];
	return obj;
}

-(BOOL)contains:(NSObject<ChipmunkObject> *)obj
{
	return [_children containsObject:obj];
}

- (NSObject<ChipmunkObject> *)smartAdd:(NSObject<ChipmunkObject> *)obj
{
	if(cpSpaceIsLocked(_space)){
		[self addPostStepAddition:obj];
	} else {
		[self add:obj];
	}
	
	return obj;
}

- (NSObject<ChipmunkObject> *)smartRemove:(NSObject<ChipmunkObject> *)obj
{
	if(cpSpaceIsLocked(_space)){
		[self addPostStepRemoval:obj];
	} else {
		[self remove:obj];
	}
	
	return obj;
}

struct PostStepTargetContext {
	id target;
	SEL selector;
};

static void
postStepPerform(cpSpace *unused, id key, struct PostStepTargetContext *context)
{
	[context->target performSelector:context->selector withObject:key];
	
	[context->target release];
	cpfree(context);
	[key release];
}

- (BOOL)addPostStepCallback:(id)target selector:(SEL)selector key:(id)key
{
	if(!cpSpaceGetPostStepCallback(_space, key)){
		struct PostStepTargetContext *context = cpcalloc(1, sizeof(struct PostStepTargetContext));
		(*context) = (struct PostStepTargetContext){target, selector};
		cpSpaceAddPostStepCallback(_space, (cpPostStepFunc)postStepPerform, key, context);
		
		[target retain];
		[key retain];
		
		return TRUE;
	} else {
		return FALSE;
	}
}

static void
postStepPerformBlock(cpSpace *unused, id key, ChipmunkPostStepBlock block)
{
	block();
	
	[block release];
	[key release];
}

- (BOOL)addPostStepBlock:(ChipmunkPostStepBlock)block key:(id)key
{
	if(!cpSpaceGetPostStepCallback(_space, key)){
		cpSpaceAddPostStepCallback(_space, (cpPostStepFunc)postStepPerformBlock, key, [block copy]);
		
		[key retain];
		
		return TRUE;
	} else {
		return FALSE;
	}
}

- (void)addPostStepAddition:(NSObject<ChipmunkObject> *)obj
{
	[self addPostStepCallback:self selector:@selector(add:) key:obj];
}

- (void)addPostStepRemoval:(NSObject<ChipmunkObject> *)obj
{
	[self addPostStepCallback:self selector:@selector(remove:) key:obj];
}

- (NSArray *)pointQueryAll:(cpVect)point maxDistance:(cpFloat)maxDistance filter:(cpShapeFilter)filter
{
	NSMutableArray *array = [NSMutableArray array];
	cpSpacePointQuery_b(_space, point, maxDistance, filter, ^(cpShape *shape, cpVect p, cpFloat d, cpVect g){
		ChipmunkPointQueryInfo *info = [[ChipmunkPointQueryInfo alloc] initWithInfo:&(cpPointQueryInfo){shape, p, d, g}];
		[array addObject:info];
		[info release];
	});
	
	return array;
}

- (ChipmunkPointQueryInfo *)pointQueryNearest:(cpVect)point maxDistance:(cpFloat)maxDistance filter:(cpShapeFilter)filter
{
	cpPointQueryInfo info;
	cpSpacePointQueryNearest(_space, point, maxDistance, filter, &info);
	return (info.shape ? [[[ChipmunkPointQueryInfo alloc] initWithInfo:&info] autorelease] : nil);
}

typedef struct segmentQueryContext {
	cpVect start, end;
	NSMutableArray *array;
} segmentQueryContext;

- (NSArray *)segmentQueryAllFrom:(cpVect)start to:(cpVect)end radius:(cpFloat)radius filter:(cpShapeFilter)filter
{
	NSMutableArray *array = [NSMutableArray array];
	cpSpaceSegmentQuery_b(_space, start, end, radius, filter, ^(cpShape *shape, cpVect p, cpVect n, cpFloat t){
		// TODO point
		ChipmunkSegmentQueryInfo *info = [[ChipmunkSegmentQueryInfo alloc] initWithInfo:&(cpSegmentQueryInfo){shape, p, n, t} start:start end:end];
		[array addObject:info];
		[info release];
	});
	
	return array;
}

- (ChipmunkSegmentQueryInfo *)segmentQueryFirstFrom:(cpVect)start to:(cpVect)end radius:(cpFloat)radius filter:(cpShapeFilter)filter
{
	cpSegmentQueryInfo info;
	cpSpaceSegmentQueryFirst(_space, start, end, radius, filter, &info);
	
	return (info.shape ? [[[ChipmunkSegmentQueryInfo alloc] initWithInfo:&info start:start end:end] autorelease] : nil);
}

- (NSArray *)bbQueryAll:(cpBB)bb filter:(cpShapeFilter)filter
{
	NSMutableArray *array = [NSMutableArray array];
	cpSpaceBBQuery_b(_space, bb, filter, ^(cpShape *shape){
		[array addObject:shape->userData];
	});
	
	return array;
}

//static void
//shapeQueryAll(cpShape *shape, cpContactPointSet *points, NSMutableArray *array)
//{
//	ChipmunkShapeQueryInfo *info = [[ChipmunkShapeQueryInfo alloc] initWithShape:shape->userData andPoints:points];
//	[array addObject:info];
//	[info release];
//}

- (NSArray *)shapeQueryAll:(ChipmunkShape *)shape
{
	NSMutableArray *array = [NSMutableArray array];
	cpSpaceShapeQuery_b(_space, shape.shape, ^(cpShape *shape, cpContactPointSet *points){
		ChipmunkShapeQueryInfo *info = [[ChipmunkShapeQueryInfo alloc] initWithShape:shape->userData andPoints:points];
		[array addObject:info];
		[info release];
	});
	
	return array;
}

- (BOOL)shapeTest:(ChipmunkShape *)shape
{
	return cpSpaceShapeQuery(_space, shape.shape, NULL, NULL);
}

static void PushBody(cpBody *body, NSMutableArray *arr){[arr addObject:body->userData];}
- (NSArray *)bodies
{
	NSMutableArray *arr = [NSMutableArray array];
	cpSpaceEachBody(_space, (cpSpaceBodyIteratorFunc)PushBody, arr);
	
	return arr;
}

static void PushShape(cpShape *shape, NSMutableArray *arr){[arr addObject:shape->userData];}
- (NSArray *)shapes
{
	NSMutableArray *arr = [NSMutableArray array];
	cpSpaceEachShape(_space, (cpSpaceShapeIteratorFunc)PushShape, arr);
	
	return arr;
}

static void PushConstraint(cpConstraint *constraint, NSMutableArray *arr){[arr addObject:constraint->userData];}
- (NSArray *)constraints
{
	NSMutableArray *arr = [NSMutableArray array];
	cpSpaceEachConstraint(_space, (cpSpaceConstraintIteratorFunc)PushConstraint, arr);
	
	return arr;
}


- (void)reindexStatic
{
	cpSpaceReindexStatic(_space);
}

- (void)reindexShape:(ChipmunkShape *)shape
{
	cpSpaceReindexShape(_space, shape.shape);
}

- (void)reindexShapesForBody:(ChipmunkBody *)body
{
	cpSpaceReindexShapesForBody(_space, body.body);
}

- (void)step:(cpFloat)dt
{
	cpSpaceStep(_space, dt);
}

//MARK: Extras

- (ChipmunkBody *)addBody:(ChipmunkBody *)obj {
	cpSpaceAddBody(_space, obj.body);
	[_children addObject:obj];
	return obj;
}

- (ChipmunkBody *)removeBody:(ChipmunkBody *)obj {
	cpSpaceRemoveBody(_space, obj.body);
	[_children removeObject:obj];
	return obj;
}


- (ChipmunkShape *)addShape:(ChipmunkShape *)obj {
	cpSpaceAddShape(_space, obj.shape);
	[_children addObject:obj];
	return obj;
}

- (ChipmunkShape *)removeShape:(ChipmunkShape *)obj {
	cpSpaceRemoveShape(_space, obj.shape);
	[_children removeObject:obj];
	return obj;
}

- (ChipmunkConstraint *)addConstraint:(ChipmunkConstraint *)obj {
	cpSpaceAddConstraint(_space, obj.constraint);
	[_children addObject:obj];
	return obj;
}

- (ChipmunkConstraint *)removeConstraint:(ChipmunkConstraint *)obj {
	cpSpaceRemoveConstraint(_space, obj.constraint);
	[_children removeObject:obj];
	return obj;
}

static ChipmunkSegmentShape *
boundSeg(ChipmunkBody *body, cpVect a, cpVect b, cpFloat radius, cpFloat elasticity,cpFloat friction, cpShapeFilter filter, cpCollisionType collisionType)
{
	ChipmunkSegmentShape *seg = [ChipmunkSegmentShape segmentWithBody:body from:a to:b radius:radius];
	seg.elasticity = elasticity;
	seg.friction = friction;
	seg.filter = filter;
	seg.collisionType = collisionType;
	
	return seg;
}

- (NSArray *)addBounds:(CGRect)bounds thickness:(cpFloat)radius
	elasticity:(cpFloat)elasticity friction:(cpFloat)friction
	filter:(cpShapeFilter)filter collisionType:(cpCollisionType)collisionType
{
	cpFloat l = bounds.origin.x - radius;
	cpFloat r = bounds.origin.x + bounds.size.width + radius;
	cpFloat b = bounds.origin.y - radius;
	cpFloat t = bounds.origin.y + bounds.size.height + radius;
	
	NSArray *segs = [[NSArrayChipmunkObject alloc] initWithArray:[NSArray arrayWithObjects:
		boundSeg(_staticBody, cpv(l,b), cpv(l,t), radius, elasticity, friction, filter, collisionType),
		boundSeg(_staticBody, cpv(l,t), cpv(r,t), radius, elasticity, friction, filter, collisionType),
		boundSeg(_staticBody, cpv(r,t), cpv(r,b), radius, elasticity, friction, filter, collisionType),
		boundSeg(_staticBody, cpv(r,b), cpv(l,b), radius, elasticity, friction, filter, collisionType),
		nil
	]];
	
	[self add:segs];
	return [segs autorelease];
}

@end
