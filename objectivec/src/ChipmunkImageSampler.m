// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

//
//  ChipmunkImageSampler.m
//  DeformableChipmunk
//
//  Created by Scott Lembcke on 8/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <TargetConditionals.h>

#if TARGET_OS_IPHONE == 1
	#import <ImageIO/ImageIO.h>
#endif

#import "ChipmunkImageSampler.h"

@implementation ChipmunkBitmapSampler

@synthesize width = _width, height = _height, bytesPerPixel = _bytesPerPixel, component = _component, pixelData = _pixelData, outputRect = _outputRect;

// Much faster than (int)floor(f)
// Profiling showed floor() to be a sizable performance hog
static inline int
floor_int(cpFloat f)
{
	int i = (int)f;
	return (f < 0.0f && f != i ? i - 1 : i);
}

// TODO finish this?
//static inline cpFloat
//SampleFunc4444(cpVect point, ChipmunkImageSampler *self)
//{
//	int x = (int)point.x;
//	int y = (int)point.y - (self->_flip ? self->_height - 1 : 0);
//	
//	int com = self->_component;
//	int byte = self->_pixels[y*self->_stride + x*self->_bytesPerPixel + com/2];
//	int value = 
//	return (cpFloat)(byte>>())/15.0;
//}

static cpFloat
SampleFunc8Clamp(cpVect point, ChipmunkBitmapSampler *self)
{
	unsigned long w = self->_width;
	unsigned long h = self->_height;
	
	cpBB bb = self->_outputRect;
	cpVect clamped = cpBBClampVect(bb, point);
	
	unsigned long x = floor_int((w - 1)*(clamped.x - bb.l)/(bb.r - bb.l) + 0.5);
	unsigned long y = floor_int((h - 1)*(clamped.y - bb.b)/(bb.t - bb.b) + 0.5);
	
	if(self->_flip) y = h - 1 - y;
	
//	printf("(%6.2f, %6.2f) -> (% 4d, % 4d) : %d\n", point.x, point.y, x, y, self->_pixels[y*self->_stride + x*self->_bytesPerPixel + self->_component]);
	return (cpFloat)self->_pixels[y*self->_stride + x*self->_bytesPerPixel + self->_component]/255.0;
}

static cpFloat
SampleFunc8Border(cpVect point, ChipmunkBitmapSampler *self)
{
	unsigned long w = self->_width;
	unsigned long h = self->_height;
	
	cpBB bb = self->_outputRect;
	if(cpBBContainsVect(bb, point)){
		unsigned long x = floor_int((w - 1)*(point.x - bb.l)/(bb.r - bb.l) + 0.5);
		unsigned long y = floor_int((h - 1)*(point.y - bb.b)/(bb.t - bb.b) + 0.5);
		
		if(self->_flip) y = h - 1 - y;
		
//		printf("(%6.2f, %6.2f) -> (% 4d, % 4d)\n", point.x, point.y, x, y);
		return (cpFloat)self->_pixels[y*self->_stride + x*self->_bytesPerPixel + self->_component]/255.0;
	} else {
		return self->_borderValue;
	}
}

-(id)initWithWidth:(NSUInteger)width height:(NSUInteger)height stride:(NSUInteger)stride bytesPerPixel:(NSUInteger)bytesPerPixel component:(NSUInteger)component flip:(bool)flip pixelData:(NSData *)pixelData
{
	if((self = [super initWithSamplingFunction:(cpMarchSampleFunc)SampleFunc8Clamp])){
		_width = width;
		_height = height;
		_stride = stride;
		
		_bytesPerPixel = bytesPerPixel;
		_component = component;
		
		_flip = flip;
		_pixelData = [pixelData retain];
		_pixels = [pixelData bytes];
		
		_outputRect = cpBBNew(0.5, 0.5, self.width - 0.5, self.height - 0.5);
	}

	return self;
}


- (void)dealloc
{
	[_pixelData release];
	
	[super dealloc];
}

-(void)setBorderRepeat
{
	_sampleFunc = (cpMarchSampleFunc)SampleFunc8Clamp;
}

-(void)setBorderValue:(cpFloat)borderValue
{
	_sampleFunc = (cpMarchSampleFunc)SampleFunc8Border;
	_borderValue = borderValue;
}

static cpBB
BorderedBB(cpBB bb, NSUInteger width, NSUInteger height)
{
	cpFloat xBorder = (bb.r - bb.l)/(cpFloat)(width - 1);
	cpFloat yBorder = (bb.t - bb.b)/(cpFloat)(height - 1);
	
	return cpBBNew(bb.l - xBorder, bb.b - yBorder, bb.r + xBorder, bb.t + yBorder);
}

-(ChipmunkPolylineSet *)marchAllWithBorder:(bool)bordered hard:(bool)hard
{
	NSUInteger width = self.width;
	NSUInteger height = self.height;
	cpBB bb = self.outputRect;
	
	if(bordered){
		return [self march:BorderedBB(bb, width, height) xSamples:width+2 ySamples:height+2 hard:hard];
	} else {
		return [self march:bb xSamples:width ySamples:height hard:hard];
	}
}

@end



@implementation ChipmunkCGContextSampler

@synthesize context = _context;

-(NSMutableData *)pixelData {return (NSMutableData *)super.pixelData;}

-(id)initWithWidth:(unsigned long)width height:(unsigned long)height colorSpace:(CGColorSpaceRef)colorSpace bitmapInfo:(CGBitmapInfo)bitmapInfo component:(NSUInteger)component
{
	// Need to create a context to get info about the context.
	// If you let the context allocate it's own memory it seems to move it around. O_o
	CGContextRef temp = CGBitmapContextCreate(NULL, width, height, 8, 0, colorSpace, bitmapInfo);
	cpAssertHard(temp, "Failed to create temporary CGBitmapContext");
	
	unsigned long bpc = CGBitmapContextGetBitsPerComponent(temp);
	unsigned long bpp = CGBitmapContextGetBitsPerPixel(temp)/8;
	cpAssertHard(bpc == 8, "Cannot handle non-8bit-per-pixel bitmap data!");
	
	CGContextRelease(temp);	
	
	unsigned long stride = width*bpp;
	NSMutableData *pixelData = [NSMutableData dataWithLength:stride*height];
	_context = CGBitmapContextCreate([pixelData mutableBytes], width, height, bpc, stride, colorSpace, bitmapInfo);
	
	return [self initWithWidth:width height:height stride:stride bytesPerPixel:bpp component:component flip:TRUE pixelData:pixelData];
}

-(void)dealloc
{
	CGContextRelease(_context);
	
	[super dealloc];
}

@end



@implementation ChipmunkImageSampler

+(CGImageRef)loadImage:(NSURL *)url
{
	CGImageSourceRef image_source = CGImageSourceCreateWithURL((CFURLRef)url, NULL);
	CGImageRef image = CGImageSourceCreateImageAtIndex(image_source, 0, NULL);
	cpAssertHard(image, "Image %s could not be loaded.", [[url description] UTF8String]);
	
	CFRelease(image_source);
	return image;
}

-(id)initWithImage:(CGImageRef)image isMask:(bool)isMask contextWidth:(NSUInteger)width contextHeight:(NSUInteger)height
{
	if(width == 0) width = CGImageGetWidth(image);
	if(height == 0)  height = CGImageGetHeight(image);
	
	CGColorSpaceRef colorSpace = (isMask ? CGColorSpaceCreateDeviceGray() : NULL);
	CGBitmapInfo bitmapInfo = (CGBitmapInfo)(isMask ? kCGImageAlphaNone : kCGImageAlphaOnly);
	
	if((self = [super initWithWidth:width height:height colorSpace:colorSpace bitmapInfo:bitmapInfo component:0])){
		CGContextDrawImage(self.context, CGRectMake(0, 0, width, height), image);
	}
	
	CGColorSpaceRelease(colorSpace);
	
	return self;
}

-(id)initWithImageFile:(NSURL *)url isMask:(bool)isMask
{
	CGImageRef image = [[self class] loadImage:url];
	unsigned long width = CGImageGetWidth(image);
	unsigned long height = CGImageGetHeight(image);
	
	self = [self initWithImage:image isMask:isMask contextWidth:width contextHeight:height];
	
	CGImageRelease(image);
	
	return self;
}

+(ChipmunkImageSampler *)samplerWithImageFile:(NSURL *)url isMask:(bool)isMask
{
	return [[[self alloc] initWithImageFile:url	isMask:isMask] autorelease];
}

@end
