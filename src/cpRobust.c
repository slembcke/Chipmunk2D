#include "chipmunk/cpRobust.h"


cpBool
cpCheckSignedArea(const cpVect a, const cpVect b, const cpVect c)
{
	const cpVect v0 = cpvsub(b, a);
	const cpVect v1 = cpvadd(cpvsub(c, a), cpvsub(c, b));
	return (v0.x*v1.y) > (v1.x*v0.y);
}

cpBool
cpCheckAxis(cpVect v0, cpVect v1, cpVect p, cpVect n){
	return cpvdot(p, n) <= cpfmax(cpvdot(v0, n), cpvdot(v1, n));
}
