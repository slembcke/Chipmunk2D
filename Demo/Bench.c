#include <stdlib.h>
#include <math.h>

#include "chipmunk.h"
#include "drawSpace.h"
#include "ChipmunkDemo.h"

static cpSpace *space;

static cpVect simple_terrain_verts[] = {
	{350.00, 425.07}, {336.00, 436.55}, {272.00, 435.39}, {258.00, 427.63}, {225.28, 420.00}, {202.82, 396.00},
	{191.81, 388.00}, {189.00, 381.89}, {173.00, 380.39}, {162.59, 368.00}, {150.47, 319.00}, {128.00, 311.55},
	{119.14, 286.00}, {126.84, 263.00}, {120.56, 227.00}, {141.14, 178.00}, {137.52, 162.00}, {146.51, 142.00},
	{156.23, 136.00}, {158.00, 118.27}, {170.00, 100.77}, {208.43,  84.00}, {224.00,  69.65}, {249.30,  68.00},
	{257.00,  54.77}, {363.00,  45.94}, {374.15,  54.00}, {386.00,  69.60}, {413.00,  70.73}, {456.00,  84.89},
	{468.09,  99.00}, {467.09, 123.00}, {464.92, 135.00}, {469.00, 141.03}, {497.00, 148.67}, {513.85, 180.00},
	{509.56, 223.00}, {523.51, 247.00}, {523.00, 277.00}, {497.79, 311.00}, {478.67, 348.00}, {467.90, 360.00},
	{456.76, 382.00}, {432.95, 389.00}, {417.00, 411.32}, {373.00, 433.19}, {361.00, 430.02}, {350.00, 425.07},
};
static int simple_terrain_count = sizeof(simple_terrain_verts)/sizeof(cpVect);

static cpVect frand_unit_circle(){
	cpVect v = cpv(frand()*2.0f - 1.0f, frand()*2.0f - 1.0f);
	return (cpvlengthsq(v) < 1.0f ? v : frand_unit_circle());
}

//cpBody bodies[1000] = {};
//cpCircleShape circles[1000] = {};

static void add_circle(int i, cpFloat radius){
	cpFloat mass = radius*radius/25.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
//	cpBody *body = cpSpaceAddBody(space, cpBodyInit(&bodies[i], mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	body->p = cpvmult(frand_unit_circle(), 180.0f);
	
	
	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
//	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeInit(&circles[i], body, radius, cpvzero));
	shape->e = 0.0f; shape->u = 0.9f;
}

static void add_box(int i, cpFloat size){
	cpFloat mass = size*size/100.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, size, size)));
//	cpBody *body = cpSpaceAddBody(space, cpBodyInit(&bodies[i], mass, cpMomentForBox(mass, size, size)));
	body->p = cpvmult(frand_unit_circle(), 180.0f);
	
	
	cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size));
	shape->e = 0.0f; shape->u = 0.9f;
}


static void setupSpace_simpleTerrain(){
	space = cpSpaceNew();
	space->iterations = 10;
	cpSpaceResizeStaticHash(space, 10.0f, 1000);
	cpSpaceResizeActiveHash(space, 10.0f, 5000);
	space->gravity = cpv(0, -100);
	
	cpVect offset = cpv(-320, -240);
	for(int i=0; i<(simple_terrain_count - 1); i++){
		cpVect a = simple_terrain_verts[i], b = simple_terrain_verts[i+1];
		cpSpaceAddShape(space, cpSegmentShapeNew(&space->staticBody, cpvadd(a, offset), cpvadd(b, offset), 0.0f));
	}
}


// SimpleTerrain constant sized objects
static cpSpace *init_SimpleTerrainCircles_1000(){
	setupSpace_simpleTerrain();
	for(int i=0; i<1000; i++) add_circle(i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainCircles_500(){
	setupSpace_simpleTerrain();
	for(int i=0; i<500; i++) add_circle(i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainCircles_100(){
	setupSpace_simpleTerrain();
	for(int i=0; i<100; i++) add_circle(i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainBoxes_1000(){
	setupSpace_simpleTerrain();
	for(int i=0; i<1000; i++) add_box(i, 10.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainBoxes_500(){
	setupSpace_simpleTerrain();
	for(int i=0; i<500; i++) add_box(i, 10.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainBoxes_100(){
	setupSpace_simpleTerrain();
	for(int i=0; i<100; i++) add_box(i, 10.0f);
	
	return space;
}


// SimpleTerrain variable sized objects
static float rand_size(){
	float unit = frand()*2.0f - 1.0f;
	return pow(1.5f, unit*2.0f);
}

static cpSpace *init_SimpleTerrainVCircles_500(){
	setupSpace_simpleTerrain();
	for(int i=0; i<500; i++) add_circle(i, 5.0f*rand_size());
	
	return space;
}

static cpSpace *init_SimpleTerrainVBoxes_500(){
	setupSpace_simpleTerrain();
	for(int i=0; i<500; i++) add_box(i, 10.0f*rand_size());
	
	return space;
}


// Build benchmark list
static void update(int ticks){
	cpSpaceStep(space, 1.0f/60.0f);
}

static void destroy(void){
	cpSpaceFreeChildren(space);
	cpSpaceFree(space);
}

#define BENCH(n) {"Bench: " #n, NULL, init_##n, update, destroy}
chipmunkDemo bench_list[] = {
	BENCH(SimpleTerrainCircles_1000),
	BENCH(SimpleTerrainCircles_500),
	BENCH(SimpleTerrainCircles_100),
	BENCH(SimpleTerrainBoxes_1000),
	BENCH(SimpleTerrainBoxes_500),
	BENCH(SimpleTerrainBoxes_100),
	BENCH(SimpleTerrainVCircles_500),
	BENCH(SimpleTerrainVBoxes_500),
};

int bench_count = sizeof(bench_list)/sizeof(chipmunkDemo);
