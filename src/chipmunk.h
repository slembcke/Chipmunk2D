/* Copyright (c) 2007 Scott Lembcke
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

#ifndef CHIPMUNK_HEADER
#define CHIPMUNK_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define CP_ALLOW_DEPRECATED_API_4

typedef double cpFloat;
#define cpfsqrt sqrt
#define cpfsin sin
#define cpfcos cos
#define cpfatan2 atan2
#define cpfmod fmod
#define cpfexp exp
#define cpfpow pow
#define cpffloor floor
#define cpfceil ceil
	
//typedef float cpFloat;
//#define cpfsqrt sqrtf
//#define cpfsin sinf
//#define cpfcos cosf
//#define cpfatan2 atan2f
//#define cpfmod fmodf
//#define cpfexp expf
//#define cpfpow powf
//#define cpffloor floorf
//#define cpfceil ceilf
	
static inline cpFloat
cpfmax(cpFloat a, cpFloat b)
{
	return (a > b) ? a : b;
}

static inline cpFloat
cpfmin(cpFloat a, cpFloat b)
{
	return (a < b) ? a : b;
}

static inline cpFloat
cpfabs(cpFloat n)
{
	return (n < 0) ? -n : n;
}

static inline cpFloat
cpfclamp(cpFloat f, cpFloat min, cpFloat max){
	return cpfmin(cpfmax(f, min), max);
}

#ifndef INFINITY
	#ifdef _MSC_VER
		union MSVC_EVIL_FLOAT_HACK
		{
			unsigned __int8 Bytes[4];
			float Value;
		};
		static union MSVC_EVIL_FLOAT_HACK INFINITY_HACK = {{0x00, 0x00, 0x80, 0x7F}};
		#define INFINITY (INFINITY_HACK.Value)
	#else
		#define INFINITY (1e1000)
	#endif
#endif

#include "cpVect.h"
#include "cpBB.h"
#include "cpBody.h"
#include "cpArray.h"
#include "cpHashSet.h"
#include "cpSpaceHash.h"

#include "cpShape.h"
#include "cpPolyShape.h"

#include "cpArbiter.h"
#include "cpCollision.h"
	
#include "constraints/cpConstraint.h"

#include "cpSpace.h"

#define CP_HASH_COEF (3344921057ul)
#define CP_HASH_PAIR(A, B) ((size_t)(A)*CP_HASH_COEF ^ (size_t)(B)*CP_HASH_COEF)

void cpInitChipmunk(void);

cpFloat cpMomentForCircle(cpFloat m, cpFloat r1, cpFloat r2, cpVect offset);
cpFloat cpMomentForSegment(cpFloat m, cpVect a, cpVect b);
cpFloat cpMomentForPoly(cpFloat m, int numVerts, cpVect *verts, cpVect offset);

#ifdef __cplusplus
}
#endif

#endif
