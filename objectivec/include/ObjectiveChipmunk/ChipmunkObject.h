// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

@class ChipmunkSpace;

/**
	Allows you to add composite objects to a space in a single method call.
	The easiest way to implement the ChipmunkObject protocol is to add a @c chipmunkObjects instance variable with a type of @c NSArray* to your class,
	create a synthesized property for it, and initialize it with the ChipmunkObjectFlatten() function.
*/
@protocol ChipmunkObject

/// Returns a set of ChipmunkBaseObject objects.
- (id <NSFastEnumeration>)chipmunkObjects;

@end


/// Have NSArray implement ChipmunkObject so that you can easily use them as containers.
@interface NSArray(ChipmunkObject) <ChipmunkObject>
@end


/// @deprecated since 6.0.2 Use [NSArray arrayWithObjects:] or similar instead.
NSSet * ChipmunkObjectFlatten(id <ChipmunkObject> firstObject, ...) __attribute__((deprecated));


/**
	This protocol is implemented by objects that know how to add themselves to a space.
	It's used internally as part of the ChipmunkObject protocol. You should never need to implement it yourself.
*/
@protocol ChipmunkBaseObject <ChipmunkObject>

@property(nonatomic, assign) id data;

- (void)addToSpace:(ChipmunkSpace *)space;
- (void)removeFromSpace:(ChipmunkSpace *)space;

@end
