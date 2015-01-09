// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#import "ObjectiveChipmunk/ObjectiveChipmunk.h"
#import "ChipmunkAutoGeometry.h"

#import <TargetConditionals.h>

#if TARGET_OS_IPHONE == 1
	#import <CoreGraphics/CoreGraphics.h>
#endif


/**
	Generic sampler used with bitmap data.
	Currently limited to 8 bit per component data.
	Bitmap samplers currently provide no filtering, but could be easily extended to do so.
*/
@interface ChipmunkBitmapSampler : ChipmunkAbstractSampler {
@private
	NSUInteger _width, _height, _stride;
	NSUInteger _bytesPerPixel, _component;
	
	bool _flip;
	const uint8_t *_pixels;
	NSData *_pixelData;
	
	cpFloat _borderValue;
	
	cpBB _outputRect;
}

/// Width of the bitmap in pixels.
@property(nonatomic, readonly) NSUInteger width;

/// Height of the bitmap in pixels.
@property(nonatomic, readonly) NSUInteger height;

/// Bytes per pixel of the bitmap. (ex: RGBA8888 would be 4)
@property(nonatomic, readonly) NSUInteger bytesPerPixel;

/// Zero-based ndex of the component to sample. (ex: alpha of RGBA would be 3)
@property(nonatomic, assign) NSUInteger component;

/// NSData object holding the pixel data.
@property(nonatomic, readonly) NSData *pixelData;

/// Rect that the image maps to.
/// Defaults to (0.5, 0.5, width - 0.5, height - 0.5) so that pixel centers will be cleanly sampled.
@property(nonatomic, assign) cpBB outputRect;

/**
	Init a sampler from bitmap data.
	Stride refers to the length of a row of pixels in bytes. (Generally just w*h*bytesPerPixel unless there is padding)
	Image must use one byte per component, but can have any number of components.
	@c component refers to the 0-based index of the component to sample. (i.e. 3 would sample the alpha in an RGBA bitmap)
	@c flip allows you to flip the image vertically to match how it migh be drawn.
	@c pixelData can be either a NSData or NSMutableData (i.e. for deformable terrain) that contains the bitmap data.
*/
-(id)initWithWidth:(NSUInteger)width height:(NSUInteger)height stride:(NSUInteger)stride bytesPerPixel:(NSUInteger)bytesPerPixel component:(NSUInteger)component flip:(bool)flip pixelData:(NSData *)pixelData;

/// Set the border of the bitmap to repeat the edge pixels.
-(void)setBorderRepeat;

/// Set the border of the bitmap to be a specific value.
-(void)setBorderValue:(cpFloat)borderValue;

/// March the entire image.
-(ChipmunkPolylineSet *)marchAllWithBorder:(bool)bordered hard:(bool)hard;

@end



/// Sampler built on top of a CGBitmapContext to allow deformable geometry.
/// Very efficient when paired with a ChipmunkTileCache.
@interface ChipmunkCGContextSampler : ChipmunkBitmapSampler {
@private
	CGContextRef _context;
}

/// CGBitmapContext for this sampler.
@property(nonatomic, readonly) CGContextRef context;

/// NSMutableData object holding the pixel data.
@property(nonatomic, readonly) NSMutableData *pixelData;

/// Initialize a context based sampler. Must provide options for a valid context.
/// Find out more here in the Quartz 2D Programming Guide.
-(id)initWithWidth:(unsigned long)width height:(unsigned long)height colorSpace:(CGColorSpaceRef)colorSpace bitmapInfo:(CGBitmapInfo)bitmapInfo component:(NSUInteger)component;

@end



/// A CGBitmapContext sampler initialized with an CGImage.
@interface ChipmunkImageSampler : ChipmunkCGContextSampler

/// Helper method to easily load CGImageRefs by path. You are responsible for releasing the CGImage.
+(CGImageRef)loadImage:(NSURL *)url;

/// Initialize an image sampler of a certain size with a CGImage.
/// If isMask is TRUE, the image will be loaded as a black and white image, if FALSE only the image alpha will be loaded.
-(id)initWithImage:(CGImageRef)image isMask:(bool)isMask contextWidth:(NSUInteger)width contextHeight:(NSUInteger)height;

/// Initialize an image sampler with an image file.
/// If isMask is TRUE, the image will be loaded as a black and white image, if FALSE only the image alpha will be loaded.
-(id)initWithImageFile:(NSURL *)url isMask:(bool)isMask;

/// Return an autoreleased image sampler initialized with an image file.
/// If isMask is TRUE, the image will be loaded as a black and white image, if FALSE only the image alpha will be loaded.
+(ChipmunkImageSampler *)samplerWithImageFile:(NSURL *)url isMask:(bool)isMask;

@end
