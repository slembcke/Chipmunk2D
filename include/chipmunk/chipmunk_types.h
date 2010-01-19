#ifdef __APPLE__
   #import "TargetConditionals.h"
#endif

// Use single precision floats on the iPhone.
#ifdef TARGET_OS_IPHONE
	#define CP_USE_FLOATS
#endif

#ifdef CP_USE_FLOATS // use doubles by default for higher precision
	typedef float cpFloat;
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
#else
	#error Using doubles as floating point type

	typedef double cpFloat;
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
#endif

#if TARGET_OS_IPHONE
	// CGPoints are structurally the same, and allow
	// easy interoperability with other iPhone libraries
	#import <CoreGraphics/CGGeometry.h>
	typedef CGPoint cpVect;
#else
	typedef struct cpVect{cpFloat x,y;} cpVect;
#endif

typedef size_t cpHashValue;
typedef void * cpDataPointer;
typedef unsigned int cpCollisionType;
typedef unsigned int cpLayers;
typedef unsigned int cpGroup;
