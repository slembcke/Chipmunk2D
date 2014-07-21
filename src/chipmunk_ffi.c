/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
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

#ifdef CHIPMUNK_FFI

#include "chipmunk/chipmunk_private.h"

// Use a single translation unit to provide all of the inlined procedures that should be part
// of the public API.

// NOTE: functions can now be referenced directly from a dynamic library, no need to use
// function pointers anymore!

extern inline cpVect cpClosetPointOnSegment(const cpVect p, const cpVect a, const cpVect b);
extern inline struct cpArbiterThread * cpArbiterThreadForBody(cpArbiter *arb, cpBody *body);
extern inline cpBool cpShapeActive(cpShape *shape);
extern inline void CircleSegmentQuery(cpShape *shape, cpVect center, cpFloat r1, cpVect a, cpVect b, cpFloat r2, cpSegmentQueryInfo *info);
extern inline cpBool cpShapeFilterReject(cpShapeFilter a, cpShapeFilter b);
extern inline void cpConstraintActivateBodies(cpConstraint *constraint);
extern inline cpVect relative_velocity(cpBody *a, cpBody *b, cpVect r1, cpVect r2);
extern inline cpFloat normal_relative_velocity(cpBody *a, cpBody *b, cpVect r1, cpVect r2, cpVect n);
extern inline void apply_impulse(cpBody *body, cpVect j, cpVect r);
extern inline void apply_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j);
extern inline void apply_bias_impulse(cpBody *body, cpVect j, cpVect r);
extern inline void apply_bias_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j);
extern inline cpFloat k_scalar_body(cpBody *body, cpVect r, cpVect n);
extern inline cpFloat k_scalar(cpBody *a, cpBody *b, cpVect r1, cpVect r2, cpVect n);
extern inline cpMat2x2 k_tensor(cpBody *a, cpBody *b, cpVect r1, cpVect r2);
extern inline cpFloat bias_coef(cpFloat errorBias, cpFloat dt);
extern inline void cpSpaceUncacheArbiter(cpSpace *space, cpArbiter *arb);
extern inline cpArray * cpSpaceArrayForBodyType(cpSpace *space, cpBodyType type);
extern inline cpConstraint * cpConstraintNext(cpConstraint *node, cpBody *body);
extern inline cpArbiter * cpArbiterNext(cpArbiter *node, cpBody *body);
extern inline cpFloat cpfmax(cpFloat a, cpFloat b);
extern inline cpFloat cpfmin(cpFloat a, cpFloat b);
extern inline cpFloat cpfabs(cpFloat f);
extern inline cpFloat cpfclamp(cpFloat f, cpFloat min, cpFloat max);
extern inline cpFloat cpfclamp01(cpFloat f);
extern inline cpFloat cpflerp(cpFloat f1, cpFloat f2, cpFloat t);
extern inline cpFloat cpflerpconst(cpFloat f1, cpFloat f2, cpFloat d);
extern inline cpBB cpBBNew(const cpFloat l, const cpFloat b, const cpFloat r, const cpFloat t);
extern inline cpBB cpBBNewForExtents(const cpVect c, const cpFloat hw, const cpFloat hh);
extern inline cpBB cpBBNewForCircle(const cpVect p, const cpFloat r);
extern inline cpBool cpBBIntersects(const cpBB a, const cpBB b);
extern inline cpBool cpBBContainsBB(const cpBB bb, const cpBB other);
extern inline cpBool cpBBContainsVect(const cpBB bb, const cpVect v);
extern inline cpBB cpBBMerge(const cpBB a, const cpBB b);
extern inline cpVect cpBBCenter(cpBB bb);
extern inline cpFloat cpBBArea(cpBB bb);
extern inline cpFloat cpBBMergedArea(cpBB a, cpBB b);
extern inline cpFloat cpBBSegmentQuery(cpBB bb, cpVect a, cpVect b);
extern inline cpBool cpBBIntersectsSegment(cpBB bb, cpVect a, cpVect b);
extern inline cpVect cpBBClampVect(const cpBB bb, const cpVect v);
extern inline cpVect cpBBWrapVect(const cpBB bb, const cpVect v);
extern inline cpBB cpBBOffset(const cpBB bb, const cpVect v);
extern inline cpShapeFilter cpShapeFilterNew(cpGroup group, cpBitmask categories, cpBitmask mask);
extern inline void cpSpatialIndexDestroy(cpSpatialIndex *index);
extern inline int cpSpatialIndexCount(cpSpatialIndex *index);
extern inline void cpSpatialIndexEach(cpSpatialIndex *index, cpSpatialIndexIteratorFunc func, void *data);
extern inline cpBool cpSpatialIndexContains(cpSpatialIndex *index, void *obj, cpHashValue hashid);
extern inline void cpSpatialIndexInsert(cpSpatialIndex *index, void *obj, cpHashValue hashid);
extern inline void cpSpatialIndexRemove(cpSpatialIndex *index, void *obj, cpHashValue hashid);
extern inline void cpSpatialIndexReindex(cpSpatialIndex *index);
extern inline void cpSpatialIndexReindexObject(cpSpatialIndex *index, void *obj, cpHashValue hashid);
extern inline void cpSpatialIndexQuery(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data);
extern inline void cpSpatialIndexSegmentQuery(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data);
extern inline void cpSpatialIndexReindexQuery(cpSpatialIndex *index, cpSpatialIndexQueryFunc func, void *data);
extern inline cpTransform cpTransformNew(cpFloat a, cpFloat b, cpFloat c, cpFloat d, cpFloat tx, cpFloat ty);
extern inline cpTransform cpTransformNewTranspose(cpFloat a, cpFloat c, cpFloat tx, cpFloat b, cpFloat d, cpFloat ty);
extern inline cpTransform cpTransformInverse(cpTransform t);
extern inline cpTransform cpTransformMult(cpTransform t1, cpTransform t2);
extern inline cpVect cpTransformPoint(cpTransform t, cpVect p);
extern inline cpVect cpTransformVect(cpTransform t, cpVect v);
extern inline cpBB cpTransformbBB(cpTransform t, cpBB bb);
extern inline cpTransform cpTransformTranslate(cpVect translate);
extern inline cpTransform cpTransformScale(cpFloat scaleX, cpFloat scaleY);
extern inline cpTransform cpTransformRotate(cpFloat radians);
extern inline cpTransform cpTransformRigid(cpVect translate, cpFloat radians);
extern inline cpTransform cpTransformRigidInverse(cpTransform t);
extern inline cpTransform cpTransformWrap(cpTransform outer, cpTransform inner);
extern inline cpTransform cpTransformWrapInverse(cpTransform outer, cpTransform inner);
extern inline cpTransform cpTransformOrtho(cpBB bb);
extern inline cpTransform cpTransformBoneScale(cpVect v0, cpVect v1);
extern inline cpTransform cpTransformAxialScale(cpVect axis, cpVect pivot, cpFloat scale);
extern inline cpVect cpv(const cpFloat x, const cpFloat y);
extern inline cpBool cpveql(const cpVect v1, const cpVect v2);
extern inline cpVect cpvadd(const cpVect v1, const cpVect v2);
extern inline cpVect cpvsub(const cpVect v1, const cpVect v2);
extern inline cpVect cpvneg(const cpVect v);
extern inline cpVect cpvmult(const cpVect v, const cpFloat s);
extern inline cpFloat cpvdot(const cpVect v1, const cpVect v2);
extern inline cpFloat cpvcross(const cpVect v1, const cpVect v2);
extern inline cpVect cpvperp(const cpVect v);
extern inline cpVect cpvrperp(const cpVect v);
extern inline cpVect cpvproject(const cpVect v1, const cpVect v2);
extern inline cpVect cpvforangle(const cpFloat a);
extern inline cpFloat cpvtoangle(const cpVect v);
extern inline cpVect cpvrotate(const cpVect v1, const cpVect v2);
extern inline cpVect cpvunrotate(const cpVect v1, const cpVect v2);
extern inline cpFloat cpvlengthsq(const cpVect v);
extern inline cpFloat cpvlength(const cpVect v);
extern inline cpVect cpvlerp(const cpVect v1, const cpVect v2, const cpFloat t);
extern inline cpVect cpvnormalize(const cpVect v);
extern inline cpVect cpvslerp(const cpVect v1, const cpVect v2, const cpFloat t);
extern inline cpVect cpvslerpconst(const cpVect v1, const cpVect v2, const cpFloat a);
extern inline cpVect cpvclamp(const cpVect v, const cpFloat len);
extern inline cpVect cpvlerpconst(cpVect v1, cpVect v2, cpFloat d);
extern inline cpFloat cpvdist(const cpVect v1, const cpVect v2);
extern inline cpFloat cpvdistsq(const cpVect v1, const cpVect v2);
extern inline cpBool cpvnear(const cpVect v1, const cpVect v2, const cpFloat dist);
extern inline cpMat2x2 cpMat2x2New(cpFloat a, cpFloat b, cpFloat c, cpFloat d);
extern inline cpVect cpMat2x2Transform(cpMat2x2 m, cpVect v);

// You should avoid making use of these bindings. Most modern compilers will keep a separate
// copy of the inlined functions defined above..

#ifdef CHIPMUNK_LEGACY_FFI
#ifdef _MSC_VER
 #if _MSC_VER >= 1600
  #define MAKE_REF(name) decltype(name) *_##name = name
 #else
  #define MAKE_REF(name)
 #endif
#else
 #define MAKE_REF(name) __typeof__(name) *_##name = name
#endif

#define MAKE_PROPERTIES_REF(struct, property) \
	MAKE_REF(struct##Get##property); MAKE_REF(struct##Set##property)

MAKE_REF(cpv); // makes a variable named _cpv that contains the function pointer for cpv()
MAKE_REF(cpveql);
MAKE_REF(cpvadd);
MAKE_REF(cpvneg);
MAKE_REF(cpvsub);
MAKE_REF(cpvmult);
MAKE_REF(cpvdot);
MAKE_REF(cpvcross);
MAKE_REF(cpvperp);
MAKE_REF(cpvrperp);
MAKE_REF(cpvproject);
MAKE_REF(cpvforangle);
MAKE_REF(cpvtoangle);
MAKE_REF(cpvrotate);
MAKE_REF(cpvunrotate);
MAKE_REF(cpvlengthsq);
MAKE_REF(cpvlength);
MAKE_REF(cpvlerp);
MAKE_REF(cpvnormalize);
MAKE_REF(cpvclamp);
MAKE_REF(cpvlerpconst);
MAKE_REF(cpvdist);
MAKE_REF(cpvdistsq);
MAKE_REF(cpvnear);

MAKE_REF(cpfmax);
MAKE_REF(cpfmin);
MAKE_REF(cpfabs);
MAKE_REF(cpfclamp);
MAKE_REF(cpflerp);
MAKE_REF(cpflerpconst);

MAKE_REF(cpBBNew);
MAKE_REF(cpBBNewForCircle);
MAKE_REF(cpBBIntersects);
MAKE_REF(cpBBContainsBB);
MAKE_REF(cpBBContainsVect);
MAKE_REF(cpBBMerge);
MAKE_REF(cpBBExpand);
MAKE_REF(cpBBArea);
MAKE_REF(cpBBMergedArea);
MAKE_REF(cpBBSegmentQuery);
MAKE_REF(cpBBIntersectsSegment);
MAKE_REF(cpBBClampVect);

MAKE_REF(cpSpatialIndexDestroy);
MAKE_REF(cpSpatialIndexCount);
MAKE_REF(cpSpatialIndexEach);
MAKE_REF(cpSpatialIndexContains);
MAKE_REF(cpSpatialIndexInsert);
MAKE_REF(cpSpatialIndexRemove);
MAKE_REF(cpSpatialIndexReindex);
MAKE_REF(cpSpatialIndexReindexObject);
MAKE_REF(cpSpatialIndexSegmentQuery);
MAKE_REF(cpSpatialIndexQuery);
MAKE_REF(cpSpatialIndexReindexQuery);

#endif

#endif
