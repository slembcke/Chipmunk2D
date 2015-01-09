// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#import "ObjectiveChipmunk/ObjectiveChipmunk.h"

#import "chipmunk/cpMarch.h"
#import "chipmunk/cpPolyline.h"

@class ChipmunkPolylineSet;

/// Wrapper for the cpPolyline type.
@interface ChipmunkPolyline : NSObject {
@private
	cpPolyline *_line;
	cpFloat _area;
}

-(id)initWithPolyline:(cpPolyline *)line;
+(ChipmunkPolyline *)fromPolyline:(cpPolyline *)line;

/// Returns true if the first and last vertex are equal.
@property(nonatomic, readonly) bool isClosed;

/// Returns the signed area of the polyline calculated by cpAreaForPoly.
/// Non-closed polylines return an area of 0.
@property(nonatomic, readonly) cpFloat area;

/// Centroid of the polyline calculated by cpCentroidForPoly.
/// It is an error to call this on a non-closed polyline.
@property(nonatomic, readonly) cpVect centroid;

/// Calculates the moment of inertia for a closed polyline with the given mass and offset.
-(cpFloat)momentForMass:(cpFloat)mass offset:(cpVect)offset;


/// Vertex count.
@property(nonatomic, readonly) NSUInteger count;

/// Array of vertexes.
@property(nonatomic, readonly) const cpVect *verts;

/**
	Returns a copy of a polyline simplified by using the Douglas-Peucker algorithm.
	This works very well on smooth or gently curved shapes, but not well on straight edged or angular shapes.
*/
-(ChipmunkPolyline *)simplifyCurves:(cpFloat)tolerance;

/**
	Returns a copy of a polyline simplified by discarding "flat" vertexes.
	This works well on straigt edged or angular shapes, not as well on smooth shapes.
*/
-(ChipmunkPolyline *)simplifyVertexes:(cpFloat)tolerance;

/// Generate a convex hull that contains a polyline. (closed or not)
-(ChipmunkPolyline *)toConvexHull;

/// Generate an approximate convex hull that contains a polyline. (closed or not)
-(ChipmunkPolyline *)toConvexHull:(cpFloat)tolerance;

/// Generate a set of convex hulls for a polyline.
/// See the note on cpPolylineConvexDecomposition_BETA() for more information.
-(ChipmunkPolylineSet *)toConvexHulls_BETA:(cpFloat)tolerance;

/// Create an array of segments for each segment in this polyline.
-(NSArray *)asChipmunkSegmentsWithBody:(ChipmunkBody *)body radius:(cpFloat)radius offset:(cpVect)offset;

/// Create a ChipmunkPolyShape from this polyline. (Must be convex!)
-(ChipmunkPolyShape *)asChipmunkPolyShapeWithBody:(ChipmunkBody *)body transform:(cpTransform)transform radius:(cpFloat)radius;

@end


/// Wrapper for the cpPolylineSet type.
@interface ChipmunkPolylineSet : NSObject<NSFastEnumeration> {
@private
	NSMutableArray *_lines;
}

-(id)initWithPolylineSet:(cpPolylineSet *)set;
+(ChipmunkPolylineSet *)fromPolylineSet:(cpPolylineSet *)set;

@property(nonatomic, readonly) NSUInteger count;

-(ChipmunkPolyline *)lineAtIndex:(NSUInteger)index;

@end


/**
	A sampler is an object that provides a basis function to build shapes from.
	This can be from a block of pixel data (loaded from a file, or dumped from the screen), or even a mathematical function such as Perlin noise.
*/
@interface ChipmunkAbstractSampler : NSObject {
@protected
	cpFloat _marchThreshold;
	cpMarchSampleFunc _sampleFunc;
}

/// The threshold passed to the cpMarch*() functions.
/// The value of the contour you want to extract.
@property(nonatomic, assign) cpFloat marchThreshold;

/// Get the primitive cpMarchSampleFunc used by this sampler.
@property(nonatomic, readonly) cpMarchSampleFunc sampleFunc;

/// Designated initializer.
-(id)initWithSamplingFunction:(cpMarchSampleFunc)sampleFunc;

/// Sample at a specific point.
-(cpFloat)sample:(cpVect)pos;

/// March a certain area of the sampler.
-(ChipmunkPolylineSet *)march:(cpBB)bb xSamples:(NSUInteger)xSamples ySamples:(NSUInteger)ySamples hard:(bool)hard;

@end



/// A simple sampler type that wraps a block as it's sampling function.
typedef cpFloat (^ChipmunkMarchSampleBlock)(cpVect point);

@interface ChipmunkBlockSampler : ChipmunkAbstractSampler {
	ChipmunkMarchSampleBlock _block;
}

/// Initializes the sampler using a copy of the passed block.
-(id)initWithBlock:(ChipmunkMarchSampleBlock)block;
+(ChipmunkBlockSampler *)samplerWithBlock:(ChipmunkMarchSampleBlock)block;

@end



#import "ChipmunkImageSampler.h"
#import "ChipmunkPointCloudSampler.h"
#import "ChipmunkTileCache.h"
