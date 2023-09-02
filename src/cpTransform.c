
#include "chipmunk/cpTransform.h"

// declarations of inline functions in cpTransform.h

cpTransform cpTransformNew(cpFloat a, cpFloat b, cpFloat c, cpFloat d, cpFloat tx, cpFloat ty);
cpTransform cpTransformNewTranspose(cpFloat a, cpFloat c, cpFloat tx, cpFloat b, cpFloat d, cpFloat ty);
cpTransform cpTransformInverse(cpTransform t);
cpTransform cpTransformMult(cpTransform t1, cpTransform t2);
cpVect cpTransformPoint(cpTransform t, cpVect p);
cpVect cpTransformVect(cpTransform t, cpVect v);
cpBB cpTransformbBB(cpTransform t, cpBB bb);
cpTransform cpTransformTranslate(cpVect translate);
cpTransform cpTransformScale(cpFloat scaleX, cpFloat scaleY);
cpTransform cpTransformRotate(cpFloat radians);
cpTransform cpTransformRigid(cpVect translate, cpFloat radians);
cpTransform cpTransformRigidInverse(cpTransform t);
cpTransform cpTransformWrap(cpTransform outer, cpTransform inner);
cpTransform cpTransformWrapInverse(cpTransform outer, cpTransform inner);
cpTransform cpTransformOrtho(cpBB bb);
cpTransform cpTransformBoneScale(cpVect v0, cpVect v1);
cpTransform cpTransformAxialScale(cpVect axis, cpVect pivot, cpFloat scale);
