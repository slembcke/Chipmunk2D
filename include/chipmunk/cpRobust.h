#include "chipmunk/cpVect.h"

// This is a private header for functions (currently just one) that need strict floating point results.
// It was easier to put this in it's own file than to fiddle with 4 different compiler specific pragmas or attributes.
// "Fast math" should be disabled here.

// Check that point c is to the left of the segment (a, b)
cpBool cpCheckPointGreater(const cpVect a, const cpVect b, const cpVect c);
