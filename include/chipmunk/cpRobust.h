#include "chipmunk/cpVect.h"

// This is a private header for functions (currently just one) that need strict floating point results.
// It was easier to put this in it's own file than to fiddle with 4 different compiler specific pragmas or attributes.
// "Fast math" should be disabled here.

// Check that the signed area of the triangle a, b, c is positive.
// Compiler optimizations for associativity break certain edge cases (ex: when a or b equals c) that lead to excessive EPA iteration.
cpBool cpCheckSignedArea(const cpVect a, const cpVect b, const cpVect c);

// Check if p is behind the segment (v0, v1) with normal n.
cpBool cpCheckAxis(cpVect v0, cpVect v1, cpVect p, cpVect n);
