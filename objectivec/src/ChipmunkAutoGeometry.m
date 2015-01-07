// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#import <malloc/malloc.h>

#import "ChipmunkAutoGeometry.h"

@implementation ChipmunkPolyline

-(id)initWithPolyline:(cpPolyline *)line
{
	if((self = [super init])){
		_line = line;
	}
	
	return self;
}

-(void)dealloc
{
	cpPolylineFree(_line);
	
	[super dealloc];
}

+(ChipmunkPolyline *)fromPolyline:(cpPolyline *)line
{
	return [[[self alloc] initWithPolyline:line] autorelease];
}

-(bool)isClosed
{
	return cpPolylineIsClosed(_line);
}

-(cpFloat)area
{
	if(_area == 0.0 && [self isClosed]){
		_area = cpAreaForPoly(_line->count - 1, _line->verts, 0.0);
	}
	
	return _area;
}

-(cpVect)centroid
{
	cpAssertHard([self isClosed], "Cannot compute the centroid of a non-looped polyline.");
	return cpCentroidForPoly(_line->count - 1, _line->verts);
}

-(cpFloat)momentForMass:(cpFloat)mass offset:(cpVect)offset
{
	cpAssertHard([self isClosed], "Cannot compute the moment of a non-looped polyline.");
	return cpMomentForPoly(mass, _line->count - 1, _line->verts, offset, 0.0);
}

-(NSUInteger)count {return _line->count;}
-(const cpVect *)verts {return _line->verts;}

-(ChipmunkPolyline *)simplifyCurves:(cpFloat)tolerance
{
	return [ChipmunkPolyline fromPolyline:cpPolylineSimplifyCurves(_line, tolerance)];
}

-(ChipmunkPolyline *)simplifyVertexes:(cpFloat)tolerance
{
	return [ChipmunkPolyline fromPolyline:cpPolylineSimplifyVertexes(_line, tolerance)];
}

-(ChipmunkPolyline *)toConvexHull:(cpFloat)tolerance
{
	return [ChipmunkPolyline fromPolyline:cpPolylineToConvexHull(_line, tolerance)];
}

-(ChipmunkPolyline *)toConvexHull
{
	return [self toConvexHull:0.0];
}

-(ChipmunkPolylineSet *)toConvexHulls_BETA:(cpFloat)tolerance
{
	cpPolylineSet *set = cpPolylineConvexDecomposition_BETA(_line, tolerance);
	ChipmunkPolylineSet *value = [ChipmunkPolylineSet fromPolylineSet:set];
	cpPolylineSetFree(set, FALSE);
	
	return value;
}

-(NSArray *)asChipmunkSegmentsWithBody:(ChipmunkBody *)body radius:(cpFloat)radius offset:(cpVect)offset
{
	NSMutableArray *arr = [NSMutableArray arrayWithCapacity:_line->count];
	
	
	cpVect a = cpvadd(_line->verts[0], offset);
	for(int i=1; i<_line->count; i++){
		cpVect b = cpvadd(_line->verts[i], offset);
		[arr addObject:[ChipmunkSegmentShape segmentWithBody:body from:a to:b radius:radius]];
		a = b;
	}
	
	return arr;
}

-(ChipmunkPolyShape *)asChipmunkPolyShapeWithBody:(ChipmunkBody *)body transform:(cpTransform)transform radius:(cpFloat)radius
{
	cpAssertHard([self isClosed], "Cannot create a poly shape for a non-closed polyline.");
	return [ChipmunkPolyShape polyWithBody:body count:_line->count - 1 verts:_line->verts transform:transform radius:radius];
}

@end



@implementation ChipmunkPolylineSet

-(id)initWithPolylineSet:(cpPolylineSet *)set
{
	if((self = [super init])){
		_lines = [[NSMutableArray alloc] initWithCapacity:set->count];
		for(int i=0; i<set->count; i++) [_lines addObject:[ChipmunkPolyline fromPolyline:set->lines[i]]];
	}
	
	return self;
}

-(void)dealloc
{
	[_lines release];
	
	[super dealloc];
}

+(ChipmunkPolylineSet *)fromPolylineSet:(cpPolylineSet *)set
{
	return [[[self alloc] initWithPolylineSet:set] autorelease];
}

-(NSUInteger)count {return _lines.count;}

-(ChipmunkPolyline *)lineAtIndex:(NSUInteger)index
{
	return [_lines objectAtIndex:index];
}

- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id *)stackbuf count:(NSUInteger)len
{
	return [_lines countByEnumeratingWithState:state objects:stackbuf count:len];
}

@end



@implementation ChipmunkAbstractSampler

@synthesize marchThreshold = _marchThreshold;
@synthesize sampleFunc = _sampleFunc;

-(id)init {
	@throw [NSException
		exceptionWithName:NSInternalInconsistencyException
		reason:[NSString stringWithFormat:@"Use designated initializer initWithSamplingFunction: to initialize a sampler."]
		userInfo:nil
	];
}

-(id)initWithSamplingFunction:(cpMarchSampleFunc)sampleFunc
{
	if((self = [super init])){
		_sampleFunc = sampleFunc;
		_marchThreshold = 0.5;
	}
	
	return self;
}

-(cpFloat)sample:(cpVect)pos
{
	return _sampleFunc(pos, self);
}


-(ChipmunkPolylineSet *)march:(cpBB)bb xSamples:(NSUInteger)xSamples ySamples:(NSUInteger)ySamples hard:(bool)hard
{
	cpPolylineSet set;
	cpPolylineSetInit(&set);
	
	(hard ? cpMarchHard : cpMarchSoft)(
		bb, xSamples, ySamples, _marchThreshold,
		(cpMarchSegmentFunc)cpPolylineSetCollectSegment, &set,
		_sampleFunc, self
	);
	
	ChipmunkPolylineSet *value = [ChipmunkPolylineSet fromPolylineSet:&set];
	
	cpPolylineSetDestroy(&set, FALSE);
	return value;
}

@end



@implementation ChipmunkBlockSampler

static cpFloat
SampleFromBlock(cpVect point, ChipmunkBlockSampler *self)
{
	return self->_block(point);
}

-(id)initWithBlock:(ChipmunkMarchSampleBlock)block
{
	if((self = [super initWithSamplingFunction:(cpMarchSampleFunc)SampleFromBlock])){
		_block = [block copy];
	}
	
	return self;
}

+(ChipmunkBlockSampler *)samplerWithBlock:(ChipmunkMarchSampleBlock)block
{
	return [[[self alloc] initWithBlock:block] autorelease];
}

-(void)dealloc
{
	[_block release];
	[super dealloc];
}

@end
