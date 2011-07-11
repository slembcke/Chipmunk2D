// Code snippet to check if all bodies in the space are sleeping

// This function is called once for each body in the space.
static void EachBody(cpBody *body, cpBool *allSleeping){
	if(!cpBodyIsSleeping(body)) *allSleeping = cpFalse;
}

// Then in your tick method, do this:
cpBool allSleeping = true;
cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)EachBody, &allSleeping);
printf("All are sleeping: %s\n", allSleeping ? "true" : "false");
