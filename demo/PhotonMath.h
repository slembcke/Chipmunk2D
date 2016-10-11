#ifndef PHOTON_MATH_H
#define PHOTON_MATH_H

#include <float.h>

#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif


//MARK: Types.

typedef struct pvec2 {float x, y;} pvec2;
typedef union pvec4 {
	struct {float x, y, z, w;};
	struct {float r, g, b, a;};
} pvec4;
typedef struct paabb2 {pvec2 min, max;} paabb2;
typedef struct pray2 {pvec2 origin, direction;} pray2;
typedef struct pmat3x2 {float a, b, c, d, x, y;} pmat3x2;
typedef struct pmat4 {float m[16];} pmat4;


//MARK: Constants.

static const pvec2 PVEC2_0 = {0, 0};
static const pvec2 PVEC2_1 = {1, 1};
static const pvec2 PVEC2_X = {1, 0};
static const pvec2 PVEC2_Y = {0, 1};

static const pvec4 PVEC4_CLEAR = {{0, 0, 0, 0}};
static const pvec4 PVEC4_WHITE = {{1, 1, 1, 1}};
static const pvec4 PVEC4_BLACK = {{0, 0, 0, 1}};
static const pvec4 PVEC4_RED   = {{1, 0, 0, 1}};
static const pvec4 PVEC4_GREEN = {{0, 1, 0, 1}};
static const pvec4 PVEC4_BLUE  = {{0, 0, 1, 1}};

static const paabb2 PAABB2_0 = {{0, 0}, {0, 0}};
static const paabb2 PAABB2_01 = {{0, 0}, {1, 1}};
static const paabb2 PAABB2_CLIP = {{-1, -1}, {1, 1}};
static const paabb2 PAABB2_INFINITE = {{-INFINITY, -INFINITY}, {INFINITY, INFINITY}};
static const paabb2 PAABB2_NOTHING = {{INFINITY, INFINITY}, {-INFINITY, -INFINITY}};

static const pmat3x2 PMAT3X2_0 = {0, 0, 0, 0, 0, 0};
static const pmat3x2 PMAT3X2_INDENTITY = {1, 0, 0, 1, 0, 0};


//MARK: Float functions:

static inline float pmin(const float a, const float b){
	return (a < b ? a : b);
}

static inline float pmax(const float a, const float b){
	return (a > b ? a : b);
}

static inline float pabs(const float a){
	return (a >= 0 ? a : -a);
}

static inline float pclamp(const float value, const float min, const float max){
	return pmax(min, pmin(value, max));
}

static inline float plerp(const float a, const float b, const float t){
	return (1 - t)*a + t*b;
}

static inline float plerpconst(const float a, const float b, const float max){
	return a + pclamp(b - a, -max, max);
}

static inline float plogerp(const float a, const float b, const float t){
	return a*powf(b/a, t);
}


//MARK: PVec2 functions:

static inline bool pvec2Eql(const pvec2 a, const pvec2 b){
	return (a.x == b.x && a.y == b.y);
}

static inline pvec2 pvec2Add(const pvec2 a, const pvec2 b){
	return (pvec2){a.x + b.x, a.y + b.y};
}

static inline pvec2 pvec2Sub(const pvec2 a, const pvec2 b){
	return (pvec2){a.x - b.x, a.y - b.y};
}

static inline pvec2 pvec2Neg(const pvec2 v){
	return (pvec2){-v.x, -v.y};
}

static inline float pvec2Dot(const pvec2 a, const pvec2 b){
	return a.x*b.x + a.y*b.y;
}

static inline float pvec2Cross(const pvec2 a, const pvec2 b){
	return a.x*b.y - a.y*b.x;
}

static inline pvec2 pvec2Perp(const pvec2 v){
	return (pvec2){-v.y, v.x};
}

static inline pvec2 pvec2RPerp(const pvec2 v){
	return (pvec2){v.y, -v.x};
}

static inline pvec2 pvec2Mult(const pvec2 v, const float s){
	return (pvec2){s*v.x, s*v.y};
}

static inline pvec2 pvec2Lerp(const pvec2 a, const pvec2 b, const float t){
	return pvec2Add(pvec2Mult(a, 1 - t), pvec2Mult(b, t));
}

static inline float pvec2LengthSQ(const pvec2 v){
	return v.x*v.x + v.y*v.y;
}

static inline float pvec2Length(const pvec2 v){
	return sqrtf(pvec2LengthSQ(v));
}

static inline float pvec2Dist(const pvec2 a, const pvec2 b){
	return pvec2Length(pvec2Sub(b, a));
}

static inline bool pvec2Near(const pvec2 a, const pvec2 b, const float dist){
	return (pvec2LengthSQ(pvec2Sub(b, a)) < dist*dist);
}

static inline float pvec2Angle(const pvec2 v){
	return atan2f(v.y, v.x);
}

static inline pvec2 pvec2Normalize(const pvec2 v){
	float coef = 1/(pvec2Length(v) + FLT_MIN);
	return (pvec2){v.x*coef, v.y*coef};
}


//MARK: PVec4 functions:

static inline bool pvec4Eql(const pvec4 a, const pvec4 b){
	return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

static inline pvec4 pvec4Add(const pvec4 a, const pvec4 b){
	return (pvec4){{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}};
}

static inline pvec4 pvec4Sub(const pvec4 a, const pvec4 b){
	return (pvec4){{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}};
}

static inline pvec4 pvec4Neg(const pvec4 v){
	return (pvec4){{-v.x, -v.y, -v.z, -v.w}};
}

static inline pvec4 pvec4Mult(const pvec4 v, const float s){
	return (pvec4){{s*v.x, s*v.y, s*v.z, s*v.w}};
}

static inline pvec4 pvec4Lerp(const pvec4 a, const pvec4 b, const float t){
	return pvec4Add(pvec4Mult(a, 1 - t), pvec4Mult(b, t));
}


//MARK: PAABB2 functions:

static inline bool paabb2Eql(const paabb2 a, const paabb2 b){
	return (pvec2Eql(a.min, b.min) && pvec2Eql(a.max, b.max));
}

static inline paabb2 paabb2Union(const paabb2 a, const paabb2 b){
	return (paabb2){
		{fminf(a.min.x, b.min.x), fminf(a.min.y, b.min.y)},
		{fmaxf(a.max.x, b.max.x), fmaxf(a.max.y, b.max.y)},
	};
}


//MARK: PMat2x3 Functions.

static inline pmat3x2 pmat3x2MakeTranspose(float a, float c, float x, float b, float d, float y){
	return (pmat3x2){a, b, c, d, x, y};
}

static inline pmat3x2 pmat3x2Mult(pmat3x2 m1, pmat3x2 m2){
  return pmat3x2MakeTranspose(
    m1.a*m2.a + m1.c*m2.b, m1.a*m2.c + m1.c*m2.d, m1.a*m2.x + m1.c*m2.y + m1.x,
    m1.b*m2.a + m1.d*m2.b, m1.b*m2.c + m1.d*m2.d, m1.b*m2.x + m1.d*m2.y + m1.y
  );
}

static inline pmat3x2 pmat3x2Inverse(pmat3x2 m){
  float inv_det = 1/(m.a*m.d - m.c*m.b);
  return pmat3x2MakeTranspose(
     m.d*inv_det, -m.c*inv_det, (m.c*m.y - m.d*m.x)*inv_det,
    -m.b*inv_det,  m.a*inv_det, (m.b*m.x - m.a*m.y)*inv_det
  );
}

static inline pmat3x2 pmat3x2TRS(pvec2 t, float r, pvec2 s){
	pvec2 rot = {cosf(r), sinf(r)};
	return pmat3x2MakeTranspose(
		 s.x*rot.x, s.y*rot.y, t.x,
		-s.x*rot.y, s.y*rot.x, t.y
	);
}

static inline pmat3x2 pmat3x2Ortho(const float l, const float r, const float b, const float t){
	float sx = 2/(r - l);
	float sy = 2/(t - b);
	float tx = -(r + l)/(r - l);
	float ty = -(t + b)/(t - b);
	return pmat3x2MakeTranspose(
		sx,  0, tx,
		 0, sy, ty
	);
}


//MARK: Transform functions.

static inline pvec2 pmat3x2Point(pmat3x2 t, pvec2 p){
	return (pvec2){t.a*p.x + t.c*p.y + t.x, t.b*p.x + t.d*p.y + t.y};
}

static inline pvec4 pmat3x2Point4(pmat3x2 t, pvec2 p){
	pvec2 p2 = pmat3x2Point(t, p);
	return (pvec4){{p2.x, p2.y, 0, 1}};
}


//MARK: Utility.

static inline bool PhotonCheckVisibility(const pmat3x2 mvp, pvec2 center, pvec2 extents){
	// Center point in clip coordinates.
	pvec2 csc = pmat3x2Point(mvp, center);
	
	// half width/height in clip space.
	float cshw = fabsf(extents.x*mvp.a) + fabsf(extents.y*mvp.c);
	float cshh = fabsf(extents.x*mvp.b) + fabsf(extents.y*mvp.d);
	
	// Check the bounds against the clip space viewport.
	return ((pabs(csc.x) - cshw < 1.0f) && (pabs(csc.y) - cshh < 1.0f));
}

static inline pmat4 pmat4Make(pmat3x2 m){
	return (pmat4){{
		m.a, m.b, 0, 0,
		m.c, m.d, 0, 0,
		  0,   0, 1, 0,
		m.x, m.y, 0, 1,
	}};
}


#ifdef __cplusplus
}
#endif

#endif
