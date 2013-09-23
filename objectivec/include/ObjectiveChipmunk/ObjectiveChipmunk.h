// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#import <Foundation/Foundation.h>

// Override some Chipmunk types for Objective-Chipmunk
#define CP_USE_CGTYPES 1

#if __has_feature(objc_arc)
	#define CP_DATA_POINTER_TYPE __unsafe_unretained id
	#define CP_GROUP_TYPE __unsafe_unretained id
	#define CP_COLLISION_TYPE_TYPE __unsafe_unretained id
#else
	#define CP_DATA_POINTER_TYPE id
	#define CP_GROUP_TYPE id
	#define CP_COLLISION_TYPE_TYPE id
#endif

#ifdef CP_ALLOW_PRIVATE_ACCESS
	#undef CP_ALLOW_PRIVATE_ACCESS
	#import "chipmunk/chipmunk_private.h"
#else
	#import "chipmunk/chipmunk.h"
#endif

#import "ChipmunkObject.h"

#import "ChipmunkBody.h"
#import "ChipmunkShape.h"
#import "ChipmunkConstraint.h"
#import "ChipmunkSpace.h"
#import "ChipmunkMultiGrab.h"
