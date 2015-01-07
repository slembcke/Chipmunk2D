// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#import "ChipmunkTileCache.h"


@interface ChipmunkCachedTile : NSObject {
	cpBB _bb;
	bool _dirty;
	
	ChipmunkCachedTile *_next, *_prev;
	
	NSArray *_shapes;
}

@property(nonatomic, readonly) cpBB bb;
@property(nonatomic, assign) bool dirty;

@property(nonatomic, assign) ChipmunkCachedTile *next;
@property(nonatomic, assign) ChipmunkCachedTile *prev;

@property(nonatomic, retain) NSArray *shapes;

@end



@implementation ChipmunkCachedTile

@synthesize bb = _bb, dirty = _dirty, shapes = _shapes, next = _next, prev = _prev;

static cpBB
ChipmunkCachedTileBB(ChipmunkCachedTile *tile)
{
	return tile->_bb;
}

static void
ChipmunkCachedTileQuery(cpVect *pos, ChipmunkCachedTile *tile, cpCollisionID id, ChipmunkCachedTile **out)
{
	if(cpBBContainsVect(tile->_bb, *pos)) (*out) = tile;
}


-(id)initWithBB:(cpBB)bb
{
	if((self = [super init])) _bb = bb;
	return self;
}

-(void)dealloc
{
	self.shapes = nil;
	[super dealloc];
}

@end



@implementation ChipmunkAbstractTileCache

@synthesize marchHard = _marchHard, sampler = _sampler, tileOffset = _tileOffset;

-(id)initWithSampler:(ChipmunkAbstractSampler *)sampler space:(ChipmunkSpace *)space tileSize:(cpFloat)tileSize samplesPerTile:(NSUInteger)samplesPerTile cacheSize:(NSUInteger)cacheSize
{
	if((self = [super init])){
		_sampler = [sampler retain];
		_space = [space retain];
		
		_tileSize = tileSize;
		_samplesPerTile =samplesPerTile;
		_tileOffset = cpvzero;
		
		_cacheSize = cacheSize;
		[self resetCache];
	}

	return self;
}

-(void)removeShapesForTile:(ChipmunkCachedTile *)tile
{
	for(ChipmunkShape *shape in tile.shapes) [_space remove:shape];
}

-(void)dealloc
{
	for(ChipmunkCachedTile *tile = _cacheTail; tile; tile = tile.next){
		[tile autorelease];
	}
	
	[_sampler release];
	[_space release];
	
	cpSpatialIndexFree(_tileIndex);
	
	[super dealloc];
}

-(void)resetCache
{
	_ensuredDirty = TRUE;
	
	// Reset the spatial index.
	if(_tileIndex) cpSpatialIndexFree(_tileIndex);
	_tileIndex = cpSpaceHashNew(_tileSize, (int)_cacheSize, (cpSpatialIndexBBFunc)ChipmunkCachedTileBB, NULL);
	
	// Remove all the shapes and release all the tiles.
	for(ChipmunkCachedTile *tile = _cacheTail; tile; tile = tile.next){
		[self removeShapesForTile:tile];
		[tile autorelease];
	}
	
	// Clear out the tile list.
	_cacheHead = _cacheTail = nil;
	_tileCount = 0;
}

-(void)marchTile:(ChipmunkCachedTile *)tile
{
	// Remove old shapes for this tile.
	for(ChipmunkShape *shape in tile.shapes) [_space remove:shape];
	cpPolylineSet *set = cpPolylineSetNew();
	
	(_marchHard ? cpMarchHard : cpMarchSoft)(
		tile.bb, _samplesPerTile, _samplesPerTile, _sampler.marchThreshold,
		(cpMarchSegmentFunc)cpPolylineSetCollectSegment, set,
		_sampler.sampleFunc, _sampler
	);
	
	if(set->count){
		ChipmunkBody *staticBody = [ChipmunkBody staticBody];
		NSMutableArray *shapes = [NSMutableArray array];
		
		for(int i=0; i<set->count; i++){
			cpPolyline *simplified = [self simplify:set->lines[i]];
			
			for(int i=0; i<simplified->count - 1; i++){
				ChipmunkSegmentShape *segment = [self makeSegmentFor:staticBody from:simplified->verts[i] to:simplified->verts[i+1]];
				[shapes addObject:segment];
				[_space add:segment];
			}
			
			cpPolylineFree(simplified);
		}
		
		tile.shapes = shapes;
	} else {
		tile.shapes = nil;
	}
	
	cpPolylineSetFree(set, TRUE);
	tile.dirty = FALSE;
}

static inline ChipmunkCachedTile *
GetTileAt(cpSpatialIndex *index, int i, int j, cpFloat size, cpVect offset)
{
	// Cannot directly get spatial hash cells, so we'll point query at the centers.
	cpVect point = cpv((i + 0.5)*size + offset.x, (j + 0.5)*size + offset.y);
	ChipmunkCachedTile *tile = nil;
	cpSpatialIndexQuery(index, &point, cpBBNewForCircle(point, 0.0f), (cpSpatialIndexQueryFunc)ChipmunkCachedTileQuery, &tile);
	
	return tile;
}

struct TileRect {int l, b, r, t;};

static inline cpBB
BBForTileRect(struct TileRect rect, cpFloat size, cpVect offset)
{
	return cpBBNew(rect.l*size + offset.x, rect.b*size + offset.y, rect.r*size + offset.x, rect.t*size + offset.y);
}

static inline struct TileRect
TileRectForBB(cpBB bb, cpFloat size, cpVect offset, cpFloat spt_inv)
{
	return (struct TileRect){
		(int)cpffloor((bb.l - offset.x)/size - spt_inv),
		(int)cpffloor((bb.b - offset.x)/size - spt_inv),
		(int) cpfceil((bb.r - offset.y)/size + spt_inv),
		(int) cpfceil((bb.t - offset.y)/size + spt_inv),
	};
}

-(void)markDirtyRect:(cpBB)bounds
{
	cpFloat size = _tileSize;
	cpVect offset = _tileOffset;
	struct TileRect rect = TileRectForBB(bounds, size, offset, 1.0/(cpFloat)_samplesPerTile);
	
	if(!_ensuredDirty && cpBBContainsBB(_ensuredBB, BBForTileRect(rect, size, offset))){
		_ensuredDirty = TRUE;
	}
	
	for(int i=rect.l; i<rect.r; i++){
		for(int j=rect.b; j<rect.t; j++){
			ChipmunkCachedTile *tile = GetTileAt(_tileIndex, i, j, size, offset);
			if(tile) tile.dirty = TRUE;
		}	
	}
}

-(void)pushTile:(ChipmunkCachedTile *)tile
{
	[tile retain];
	
	_cacheHead.next = tile;
	tile.prev = _cacheHead;
	
	_cacheHead = tile;
	_cacheTail = _cacheTail ?: tile;
}

-(void)removeTile:(ChipmunkCachedTile *)tile
{
	if(tile.prev == nil && _cacheTail == tile) _cacheTail = tile.next;
	if(tile.next == nil && _cacheHead == tile) _cacheHead = tile.prev;
	
	tile.prev.next = tile.next;
	tile.next.prev = tile.prev;
	tile.prev = tile.next = nil;
	
	[tile autorelease];
}

#define LOADING_FACTOR 1.0

//extern double GetMilliseconds();

-(void)ensureRect:(cpBB)bounds
{
//	double time = GetMilliseconds();
	
	cpFloat size = _tileSize;
	cpVect offset = _tileOffset;
	struct TileRect rect = TileRectForBB(bounds, size, offset, 1.0/(cpFloat)_samplesPerTile);
	
	cpBB ensure = BBForTileRect(rect, size, offset);
	if(!_ensuredDirty && cpBBContainsBB(_ensuredBB, ensure)) return;
	
//	int count = 0;
//	printf("Marching tiles in (% 4d, % 4d) - (% 4d, % 4d):\n", l, b, r, t);
	for(int i=rect.l; i<rect.r; i++){
		for(int j=rect.b; j<rect.t; j++){
//			printf("Marching tile (% 4d, % 4d)\n", i, j);
			
			ChipmunkCachedTile *tile = GetTileAt(_tileIndex, i, j, size, offset);
			
			if(!tile){
				// Tile does not exist yet, make a new dirty tile.
				// Let the code below push it into the tile list.
				tile = [[ChipmunkCachedTile alloc] initWithBB:BBForTileRect((struct TileRect){i, j, i+1, j+1}, size, offset)];
				tile.dirty = TRUE;
				
				cpSpatialIndexInsert(_tileIndex, tile, (cpHashValue)tile);
				_tileCount++;
			}
			
			if(tile.dirty) [self marchTile:tile];
			
			// Move tile to the front of the cache. (or add it for the first time)
			[self removeTile:tile];
			[self pushTile:tile];
		}
	}
	
	_ensuredBB = ensure;
	_ensuredDirty = FALSE;
	
	// Remove tiles used the longest ago if over the cache count;
	NSInteger removeCount = _tileCount - _cacheSize;
	for(int i=0; i<removeCount; i++){
		cpSpatialIndexRemove(_tileIndex, _cacheTail, (cpHashValue)_cacheTail);
		[self removeShapesForTile:_cacheTail];
		[self removeTile:_cacheTail];
		
		_tileCount--;
	}
	
//	NSLog(@"Updated %3d tiles in %6.3fms", count, GetMilliseconds() - time);
}

-(cpPolyline *)simplify:(cpPolyline *)polyline
{
	@throw [NSException
		exceptionWithName:NSInternalInconsistencyException
		reason:[NSString stringWithFormat:@"You must override %@ in a subclass", NSStringFromSelector(_cmd)]
		userInfo:nil
	];
}

-(ChipmunkSegmentShape *)makeSegmentFor:(ChipmunkBody *)staticBody from:(cpVect)a to:(cpVect)b
{
	@throw [NSException
		exceptionWithName:NSInternalInconsistencyException
		reason:[NSString stringWithFormat:@"You must override %@ in a subclass", NSStringFromSelector(_cmd)]
		userInfo:nil
	];
}

@end



@implementation ChipmunkBasicTileCache

@synthesize simplifyThreshold = _simplifyThreshold;

@synthesize segmentRadius = _segmentRadius;

@synthesize segmentFriction = _segmentFriction;
@synthesize segmentElasticity = _segmentElasticity;

@synthesize segmentFilter = _segmentFilter;

@synthesize segmentCollisionType = _segmentCollisionType;

-(id)initWithSampler:(ChipmunkAbstractSampler *)sampler space:(ChipmunkSpace *)space tileSize:(cpFloat)tileSize samplesPerTile:(NSUInteger)samplesPerTile cacheSize:(NSUInteger)cacheSize
{
	if((self = [super initWithSampler:sampler space:space tileSize:tileSize samplesPerTile:samplesPerTile cacheSize:cacheSize])){
		_simplifyThreshold = 2.0;
		
		_segmentRadius = 0.0;
		
		_segmentFriction = 1.0;
		_segmentElasticity = 1.0;
		
		_segmentFilter = CP_SHAPE_FILTER_ALL;
		
		_segmentCollisionType = (cpCollisionType)0;
	}
	
	return self;
}

-(cpPolyline *)simplify:(cpPolyline *)polyline
{
	return cpPolylineSimplifyCurves(polyline, _simplifyThreshold);
}

-(ChipmunkSegmentShape *)makeSegmentFor:(ChipmunkBody *)staticBody from:(cpVect)a to:(cpVect)b
{
	ChipmunkSegmentShape *segment = [ChipmunkSegmentShape segmentWithBody:staticBody from:a to:b radius:_segmentRadius];
	segment.friction = _segmentFriction;
	segment.elasticity = _segmentElasticity;
	segment.filter = _segmentFilter;
	segment.collisionType = _segmentCollisionType;
	
	return segment;
}

@end
