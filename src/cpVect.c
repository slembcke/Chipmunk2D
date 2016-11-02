
#include "chipmunk/cpVect.h"

cpVect cpv(const cpFloat x, const cpFloat y);
cpBool cpveql(const cpVect v1, const cpVect v2);
cpVect cpvadd(const cpVect v1, const cpVect v2);
cpVect cpvsub(const cpVect v1, const cpVect v2);
cpVect cpvneg(const cpVect v);
cpVect cpvmult(const cpVect v, const cpFloat s);
cpFloat cpvdot(const cpVect v1, const cpVect v2);
cpFloat cpvcross(const cpVect v1, const cpVect v2);
cpVect cpvperp(const cpVect v);
cpVect cpvrperp(const cpVect v);
cpVect cpvproject(const cpVect v1, const cpVect v2);
cpVect cpvforangle(const cpFloat a);
cpFloat cpvtoangle(const cpVect v);
cpVect cpvrotate(const cpVect v1, const cpVect v2);
cpVect cpvunrotate(const cpVect v1, const cpVect v2);
cpFloat cpvlengthsq(const cpVect v);
cpFloat cpvlength(const cpVect v);
cpVect cpvlerp(const cpVect v1, const cpVect v2, const cpFloat t);
cpVect cpvnormalize(const cpVect v);
cpVect cpvslerp(const cpVect v1, const cpVect v2, const cpFloat t);
cpVect cpvslerpconst(const cpVect v1, const cpVect v2, const cpFloat a);
cpVect cpvclamp(const cpVect v, const cpFloat len);
cpVect cpvlerpconst(cpVect v1, cpVect v2, cpFloat d);
cpFloat cpvdist(const cpVect v1, const cpVect v2);
cpFloat cpvdistsq(const cpVect v1, const cpVect v2);
cpBool cpvnear(const cpVect v1, const cpVect v2, const cpFloat dist);

