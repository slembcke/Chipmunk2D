#include "chipmunk.h"

int main(void){
// Construct a pile of boxes.
// Force them to sleep until the first time they are touched.
// Group them together so that touching any box wakes all of them.
cpFloat size = 20;
cpFloat mass = 1;
cpFloat moment = cpMomentForBox(mass, size, size);

cpBody *lastBody = NULL;

for(int i=0; i<5; i++){
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(body, cpv(0, i*size));
	
	cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size));
	cpShapeSetFriction(shape, 0.7);
	
	// You can use any sleeping body as a group identifier.
	// Here we just keep a reference to the last body we initialized.
	// Passing NULL as the group starts a new sleeping group.
	// You MUST do this after completely initializing the object.
	// Attaching shapes or calling setter functions will wake the body back up.
	cpBodySleepWithGroup(body, lastBody);
	lastBody = body;
}

	return 0;
}