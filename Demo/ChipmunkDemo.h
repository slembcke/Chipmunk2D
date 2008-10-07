struct chipmunkDemo;

typedef cpSpace *(*demoInitFunc)(void);
typedef void (*demoUpdateFunc)(int ticks);
typedef void (*demoDestroyFunc)(void);

typedef struct chipmunkDemo {
	char *name;

	drawSpaceOptions *drawOptions;
	
	demoInitFunc initFunc;
	demoUpdateFunc updateFunc;
	demoDestroyFunc destroyFunc;
} chipmunkDemo;