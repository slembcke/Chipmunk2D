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

#define CP_ALLOW_PRIVATE_ACCESS 1
#import "ObjectiveChipmunk.h"

@interface ChipmunkSpace(DoubleDispatch)

- (ChipmunkShape *)addShape:(ChipmunkShape *)obj;
- (ChipmunkShape *)removeShape:(ChipmunkShape *)obj;

@end

@implementation ChipmunkShape

@synthesize userData = _userData;

+(ChipmunkShape *)shapeFromCPShape:(cpShape *)shape
{
	ChipmunkShape *obj = shape->userData;
	cpAssertHard([obj isKindOfClass:[ChipmunkShape class]], "'shape->data' is not a pointer to a ChipmunkShape object.");
	
	return obj;
}

- (void) dealloc {
	[self.body release];
	cpShapeDestroy(self.shape);
	[super dealloc];
}


- (cpShape *)shape {
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}

- (ChipmunkBody *)body {
	cpBody *body = self.shape->body;
	return (body ? body->userData : nil);
}

- (void)setBody:(ChipmunkBody *)value {
	if(self.body != value){
		[self.body release];
		self.shape->body = [[value retain] body];
	}
}

-(cpFloat)mass {return cpShapeGetMass(self.shape);}
-(void)setMass:(cpFloat)mass {cpShapeSetMass(self.shape, mass);}

-(cpFloat)density {return cpShapeGetDensity(self.shape);}
-(void)setDensity:(cpFloat)density {cpShapeSetDensity(self.shape, density);}

-(cpFloat)moment {return cpShapeGetMoment(self.shape);}
-(cpFloat)area {return cpShapeGetArea(self.shape);}
-(cpVect)centerOfGravity {return cpShapeGetCenterOfGravity(self.shape);}

// accessor macros
#define getter(type, lower, upper, member) \
- (type)lower {return self.shape->member;}
#define setter(type, lower, upper, member) \
- (void)set##upper:(type)value {self.shape->member = value;};
#define both(type, lower, upper, member) \
getter(type, lower, upper, member) \
setter(type, lower, upper, member)

getter(cpBB, bb, BB, bb)
both(BOOL, sensor, Sensor, sensor)
both(cpFloat, elasticity, Elasticity, e)
both(cpFloat, friction, Friction, u)
both(cpVect, surfaceVelocity, SurfaceVelocity, surfaceV)
both(cpCollisionType, collisionType, CollisionType, type)
both(cpShapeFilter, filter, Filter, filter)

-(ChipmunkSpace *)space {
	cpSpace *space = cpShapeGetSpace(self.shape);
	return (ChipmunkSpace *)(space ? cpSpaceGetUserData(space) : nil);
}

- (cpBB)cacheBB {return cpShapeCacheBB(self.shape);}

- (ChipmunkPointQueryInfo *)pointQuery:(cpVect)point
{
	cpPointQueryInfo info;
	cpShapePointQuery(self.shape, point, &info);
	return (info.shape ? [[[ChipmunkPointQueryInfo alloc] initWithInfo:&info] autorelease] : nil);
}

- (ChipmunkSegmentQueryInfo *)segmentQueryFrom:(cpVect)start to:(cpVect)end radius:(cpFloat)radius
{
	cpSegmentQueryInfo info;
	if(cpShapeSegmentQuery(self.shape, start, end, radius, &info)){
		return [[[ChipmunkSegmentQueryInfo alloc] initWithInfo:&info start:start end:end] autorelease];
	} else {
		return nil;
	}
}


- (NSArray *)chipmunkObjects {return [NSArray arrayWithObject:self];}
- (void)addToSpace:(ChipmunkSpace *)space {[space addShape:self];}
- (void)removeFromSpace:(ChipmunkSpace *)space {[space removeShape:self];}

@end


@implementation ChipmunkPointQueryInfo

- (id)initWithInfo:(cpPointQueryInfo *)info
{
	if((self = [super init])){
		_info = (*info);
		[self.shape retain];
	}
	
	return self;
}

- (cpPointQueryInfo *)info {return &_info;}
- (ChipmunkShape *)shape {return (_info.shape ? _info.shape->userData : nil);}
- (cpVect)point {return _info.point;}
- (cpFloat)distance {return _info.distance;}
- (cpVect)gradient {return _info.gradient;}

- (void)dealloc
{
	[self.shape release];
	[super dealloc];
}


@end


@implementation ChipmunkSegmentQueryInfo

- (id)initWithInfo:(cpSegmentQueryInfo *)info start:(cpVect)start end:(cpVect)end
{
	if((self = [super init])){
		_info = (*info);
		_start = start;
		_end = end;
		
		[self.shape retain];
	}
	
	return self;
}

- (cpSegmentQueryInfo *)info {return &_info;}
- (ChipmunkShape *)shape {return (_info.shape ? _info.shape->userData : nil);}
- (cpFloat)t {return _info.alpha;}
- (cpVect)normal {return _info.normal;}
- (cpVect)point {return _info.point;}
- (cpFloat)dist {return cpvdist(_start, _end)*_info.alpha;}
- (cpVect)start {return _start;}
- (cpVect)end {return _end;}

- (void)dealloc
{
	[self.shape release];
	[super dealloc];
}


@end


@implementation ChipmunkShapeQueryInfo

@synthesize shape = _shape;
- (cpContactPointSet *)contactPoints {return &_contactPoints;}

- (id)initWithShape:(ChipmunkShape *)shape andPoints:(cpContactPointSet *)set
{
	if((self = [super init])){
		_shape = [shape retain];
		_contactPoints = *set;
	}
	
	return self;
}

- (void)dealloc {
	[_shape release];
	[super dealloc];
}

@end

@implementation ChipmunkCircleShape {
	cpCircleShape _shape;
}


+ (ChipmunkCircleShape *)circleWithBody:(ChipmunkBody *)body radius:(cpFloat)radius offset:(cpVect)offset
{
	return [[[self alloc] initWithBody:body radius:radius offset:offset] autorelease];
}

- (cpShape *)shape {return (cpShape *)&_shape;}

- (id)initWithBody:(ChipmunkBody *)body radius:(cpFloat)radius offset:(cpVect)offset {
	if((self = [super init])){
		[body retain];
		cpCircleShapeInit(&_shape, body.body, radius, offset);
		self.shape->userData = self;
	}
	
	return self;
}

- (cpFloat)radius {return cpCircleShapeGetRadius((cpShape *)&_shape);}
- (cpVect)offset {return cpCircleShapeGetOffset((cpShape *)&_shape);}

@end


@implementation ChipmunkSegmentShape {
	cpSegmentShape _shape;
}

+ (ChipmunkSegmentShape *)segmentWithBody:(ChipmunkBody *)body from:(cpVect)a to:(cpVect)b radius:(cpFloat)radius
{
	return [[[self alloc] initWithBody:body from:a to:b radius:radius] autorelease];
}

- (cpShape *)shape {return (cpShape *)&_shape;}

- (id)initWithBody:(ChipmunkBody *)body from:(cpVect)a to:(cpVect)b radius:(cpFloat)radius {
	if((self = [super init])){
		[body retain];
		cpSegmentShapeInit(&_shape, body.body, a, b, radius);
		self.shape->userData = self;
	}
	
	return self;
}

- (void)setPrevNeighbor:(cpVect)prev nextNeighbor:(cpVect)next
{
	cpSegmentShapeSetNeighbors((cpShape *)&_shape, prev, next);
}

- (cpVect)a {return cpSegmentShapeGetA((cpShape *)&_shape);}
- (cpVect)b {return cpSegmentShapeGetB((cpShape *)&_shape);}
- (cpVect)normal {return cpSegmentShapeGetNormal((cpShape *)&_shape);}
- (cpFloat)radius {return cpSegmentShapeGetRadius((cpShape *)&_shape);}

@end


@implementation ChipmunkPolyShape {
	cpPolyShape _shape;
}

+ (id)polyWithBody:(ChipmunkBody *)body count:(int)count verts:(const cpVect *)verts transform:(cpTransform)transform radius:(cpFloat)radius
{
	return [[[self alloc] initWithBody:body count:count verts:verts transform:transform radius:radius] autorelease];
}

+ (id)boxWithBody:(ChipmunkBody *)body width:(cpFloat)width height:(cpFloat)height radius:(cpFloat)radius
{
	return [[[self alloc] initBoxWithBody:body width:width height:height radius:radius] autorelease];
}

+ (id)boxWithBody:(ChipmunkBody *)body bb:(cpBB)bb radius:(cpFloat)radius
{
	return [[[self alloc] initBoxWithBody:body bb:bb radius:radius] autorelease];
}

- (cpShape *)shape {return (cpShape *)&_shape;}

- (id)initWithBody:(ChipmunkBody *)body count:(int)count verts:(const cpVect *)verts transform:(cpTransform)transform radius:(cpFloat)radius
{
	if((self = [super init])){
		[body retain];
		cpPolyShapeInit(&_shape, body.body, count, verts, transform, radius);
		self.shape->userData = self;
	}
	
	return self;
}

- (id)initBoxWithBody:(ChipmunkBody *)body width:(cpFloat)width height:(cpFloat)height radius:(cpFloat)radius
{
	if((self = [super init])){
		[body retain];
		cpBoxShapeInit(&_shape, body.body, width, height, radius);
		self.shape->userData = self;
	}
	
	return self;
}

- (id)initBoxWithBody:(ChipmunkBody *)body bb:(cpBB)bb radius:(cpFloat)radius
{
	if((self = [super init])){
		[body retain];
		cpBoxShapeInit2(&_shape, body.body, bb, radius);
		self.shape->userData = self;
	}
	
	return self;
}

- (int)count {return cpPolyShapeGetCount((cpShape *)&_shape);}
- (cpFloat)radius {return cpPolyShapeGetRadius((cpShape *)&_shape);}
- (cpVect)getVertex:(int)index {return cpPolyShapeGetVert((cpShape *)&_shape, index);}

@end
