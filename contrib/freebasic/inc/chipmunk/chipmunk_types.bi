#include "crt/stdint.bi"

#ifndef CHIPMUNK_TYPES
#define CHIPMUNK_TYPES	1

#ifndef CP_USE_DOUBLES
'' use doubles by default for higher precision
#define CP_USE_DOUBLES 1
#endif



#if CP_USE_DOUBLES

''/ Chipmunk's floating point type.

''/ Can be reconfigured at compile time.

	type as double cpFloat

	#define cpfsqrt sqrt

	#define cpfsin sin

	#define cpfcos cos

	#define cpfacos acos

	#define cpfatan2 atan2

	#define cpfmod fmod

	#define cpfexp exp

	#define cpfpow pow

	#define cpffloor floor

	#define cpfceil ceil

#else

	type as single cpFloat

	#define cpfsqrt sqrtf

	#define cpfsin sinf

	#define cpfcos cosf

	#define cpfacos acosf

	#define cpfatan2 atan2f

	#define cpfmod fmodf

	#define cpfexp expf

	#define cpfpow powf

	#define cpffloor floorf

	#define cpfceil ceilf

#endif



#ifdef INFINITY
#	undef INFINITY
#	define INFINITY (1e31)
#endif

#ifndef M_PI
	#define M_PI 3.14159265358979323846264338327950288
#endif



#ifndef M_E
	#define M_E 2.71828182845904523536028747135266250
#endif

''/ Return the max of two cpFloats.
#ifndef cpfmax
#define cpfmax( a, b )		iif( a > b, a, b )
#endif



''/ Return the min of two cpFloats.
#ifndef cpfmin
#define cpfmin( a, b )		iif( a < b, a, b )
#endif



''/ Return the absolute value of a cpFloat.
#ifndef cpfabs
#define cpfabs( f )		iif( f < 0, -(f), (f) )
#endif


''/ Clamp @c f to be between @c min and @c max.
#ifndef cpfclamp
#define cpfclamp( f, min, max )		cpfmin( cpfmax( f, min ), max )
#endif


''/ Clamp @c f to be between 0 and 1.
#ifndef cpfclamp01
#define cpfclamp01( f )	cpfmax( 0.0, cpfmin( f, 1.0 ) )
#endif


''/ Linearly interpolate (or extrapolate) between @c f1 and @c f2 by @c t percent.
#ifndef cpflerp
#define cpflerp( f1, f2, t )	( f1 * (1.0 - t) + f2*t )
#endif

''/ Linearly interpolate from @c f1 to @c f2 by no more than @c d.
#ifndef cpflerpconst
#define cpflerpconst( f1, f2, d )	( f1 + cpfclamp( f2 - f1, -(d), (d) ) )
#endif


''/ Hash value type.
type cpHashValue as unsigned integer ptr



'' Oh C, how we love to define our own boolean types to get compiler compatibility

''/ Chipmunk's boolean type.

#ifdef CP_BOOL_TYPE
	type as CP_BOOL_TYPE cpBool
#else
	type as integer cpBool
#endif

#ifndef cpTrue
	''/ true value.
	#define cpTrue 1
#endif

#ifndef cpFalse
	''/ false value.
	#define cpFalse 0
#endif

#ifdef CP_DATA_POINTER_TYPE
	type as CP_DATA_POINTER_TYPE cpDataPointer
#else
	''/ Type used for user data pointers.
	type as any ptr cpDataPointer
#endif

#ifdef CP_COLLISION_TYPE_TYPE
	type as CP_COLLISION_TYPE_TYPE cpCollisionType
#else
	''/ Type used for cpSpace.collision_type.
	type as uintptr_t cpCollisionType
#endif

#ifdef CP_GROUP_TYPE
	type as CP_GROUP_TYPE cpGroup
#else
	''/ Type used for cpShape.group.
	type as uintptr_t cpGroup
#endif

#ifdef CP_LAYERS_TYPE
	type as CP_LAYERS_TYPE cpLayers
#else
	''/ Type used for cpShape.layers.
	type as uinteger cpLayers
#endif

#ifdef CP_TIMESTAMP_TYPE
	type as CP_TIMESTAMP_TYPE cpTimestamp
#else
	''/ Type used for various timestamps in Chipmunk.
	type as uinteger cpTimestamp
#endif

#ifndef CP_NO_GROUP
	''/ Value for cpShape.group signifying that a shape is in no group.
	#define CP_NO_GROUP ((cpGroup)0)
#endif

#ifndef CP_ALL_LAYERS
	''/ Value for cpShape.layers signifying that a shape is in every layer.
	#define CP_ALL_LAYERS ( not (cpLayers)0)
#endif

''/ @}


type cpVect : as cpFloat x,y : end type



type cpMat2x2

	'' Row major [[a, b][c d]]

	as cpFloat a, b, c, d

end type

#endif