#include "chipmunk/cpRobust.h"


cpBool
cpCheckPointGreater(const cpVect a, const cpVect b, const cpVect c)
{
	return (b.y - a.y)*(a.x + b.x - 2*c.x) > (b.x - a.x)*(a.y + b.y - 2*c.y);
}
