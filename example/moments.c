#include "chipmunk.h"

int main(void){
// Moment for a solid circle with a mass of 2 and radius 5.
cpFloat circle1 = cpMomentForCircle(2, 0, 5, cpvzero);

// Moment for a hollow circle with a mass of 1, inner radius of 2 and outer radius of 6.
cpFloat circle2 = cpMomentForCircle(1, 2, 6, cpvzero);

// Moment for a solid circle with a mass of 1, radius of 3 and
// centered 3 units along the x axis from the center of gravity.
cpFloat circle3 = cpMomentForCircle(2, 0, 5, cpv(3, 0));

// Composite object. 1x4 box centered on the center of gravity and a circle sitting on top.
// Just add the moments together.
cpFloat composite = cpMomentForBox(boxMass, 1, 4) + cpMomentForCircle(circleMass, 0, 1, cpv(0, 3));
	
	return 0;
}