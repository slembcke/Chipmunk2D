#include "chipmunk.h"
#include "ChipmunkDemo.h"

#if 1
	#define BENCH_SPACE_NEW cpSpaceNew
	#define BENCH_SPACE_FREE cpSpaceFree
	#define BENCH_SPACE_STEP cpSpaceStep
#else
	#import "cpHastySpace.h"
	
	static cpSpace *
	MakeHastySpace()
	{
		cpSpace *space = cpHastySpaceNew();
		cpHastySpaceSetThreads(space, 0);
		return space;
	}
	
	#define BENCH_SPACE_NEW MakeHastySpace
	#define BENCH_SPACE_FREE cpHastySpaceFree
	#define BENCH_SPACE_STEP cpHastySpaceStep
#endif

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

static void add_circle(cpSpace *space, int index, cpFloat radius){
	cpFloat mass = radius*radius/25.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
//	cpBody *body = cpSpaceAddBody(space, cpBodyInit(&bodies[i], mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
	body->p = cpvmult(frand_unit_circle(), 180.0f);
	
	
	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
//	cpShape *shape = cpSpaceAddShape(space, cpCircleShapeInit(&circles[i], body, radius, cpvzero));
	shape->e = 0.0f; shape->u = 0.9f;
}

static void add_box(cpSpace *space, int index, cpFloat size){
	cpFloat mass = size*size/100.0f;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForBox(mass, size, size)));
//	cpBody *body = cpSpaceAddBody(space, cpBodyInit(&bodies[i], mass, cpMomentForBox(mass, size, size)));
	body->p = cpvmult(frand_unit_circle(), 180.0f);
	
	
	cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, size, size));
	shape->e = 0.0f; shape->u = 0.9f;
}

static void add_hexagon(cpSpace *space, int index, cpFloat radius){
	cpVect hexagon[6] = {};
	for(int i=0; i<6; i++){
		cpFloat angle = -M_PI*2.0f*i/6.0f;
		hexagon[i] = cpvmult(cpv(cos(angle), sin(angle)), radius);
	}
	
	cpFloat mass = radius*radius;
	cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, 6, hexagon, cpvzero)));
	body->p = cpvmult(frand_unit_circle(), 180.0f);
	
	cpShape *shape = cpSpaceAddShape(space, cpPolyShapeNew(body, 6, hexagon, cpvzero));
	shape->e = 0.0f; shape->u = 0.9f;
}


static cpSpace *
SetupSpace_simpleTerrain(){
	cpSpace *space = BENCH_SPACE_NEW();
	space->iterations = 10;
	space->gravity = cpv(0, -100);
	space->collisionSlop = 0.5f;
	
	cpVect offset = cpv(-320, -240);
	for(int i=0; i<(simple_terrain_count - 1); i++){
		cpVect a = simple_terrain_verts[i], b = simple_terrain_verts[i+1];
		cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpvadd(a, offset), cpvadd(b, offset), 0.0f));
	}
	
	return space;
}


// SimpleTerrain constant sized objects
static cpSpace *init_SimpleTerrainCircles_1000(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<1000; i++) add_circle(space, i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainCircles_500(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<500; i++) add_circle(space, i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainCircles_100(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<100; i++) add_circle(space, i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainBoxes_1000(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<1000; i++) add_box(space, i, 10.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainBoxes_500(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<500; i++) add_box(space, i, 10.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainBoxes_100(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<100; i++) add_box(space, i, 10.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainHexagons_1000(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<1000; i++) add_hexagon(space, i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainHexagons_500(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<500; i++) add_hexagon(space, i, 5.0f);
	
	return space;
}

static cpSpace *init_SimpleTerrainHexagons_100(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<100; i++) add_hexagon(space, i, 5.0f);
	
	return space;
}


// SimpleTerrain variable sized objects
static cpFloat rand_size(){
	return cpfpow(1.5, cpflerp(-1.5, 3.5, frand()));
}

static cpSpace *init_SimpleTerrainVCircles_200(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<200; i++) add_circle(space, i, 5.0f*rand_size());
	
	return space;
}

static cpSpace *init_SimpleTerrainVBoxes_200(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<200; i++) add_box(space, i, 8.0f*rand_size());
	
	return space;
}

static cpSpace *init_SimpleTerrainVHexagons_200(){
	cpSpace *space = SetupSpace_simpleTerrain();
	for(int i=0; i<200; i++) add_hexagon(space, i, 5.0f*rand_size());
	
	return space;
}


// ComplexTerrain
static cpVect complex_terrain_verts[] = {
	{ 46.78, 479.00}, { 35.00, 475.63}, { 27.52, 469.00}, { 23.52, 455.00}, { 23.78, 441.00}, { 28.41, 428.00}, { 49.61, 394.00}, { 59.00, 381.56}, { 80.00, 366.03}, { 81.46, 358.00}, { 86.31, 350.00}, { 77.74, 320.00},
	{ 70.26, 278.00}, { 67.51, 270.00}, { 58.86, 260.00}, { 57.19, 247.00}, { 38.00, 235.60}, { 25.76, 221.00}, { 24.58, 209.00}, { 27.63, 202.00}, { 31.28, 198.00}, { 40.00, 193.72}, { 48.00, 193.73}, { 55.00, 196.70},
	{ 62.10, 204.00}, { 71.00, 209.04}, { 79.00, 206.55}, { 88.00, 206.81}, { 95.88, 211.00}, {103.00, 220.49}, {131.00, 220.51}, {137.00, 222.66}, {143.08, 228.00}, {146.22, 234.00}, {147.08, 241.00}, {145.45, 248.00},
	{142.31, 253.00}, {132.00, 259.30}, {115.00, 259.70}, {109.28, 270.00}, {112.91, 296.00}, {119.69, 324.00}, {129.00, 336.26}, {141.00, 337.59}, {153.00, 331.57}, {175.00, 325.74}, {188.00, 325.19}, {235.00, 317.46},
	{250.00, 317.19}, {255.00, 309.12}, {262.62, 302.00}, {262.21, 295.00}, {248.00, 273.59}, {229.00, 257.93}, {221.00, 255.48}, {215.00, 251.59}, {210.79, 246.00}, {207.47, 234.00}, {203.25, 227.00}, {179.00, 205.90},
	{148.00, 189.54}, {136.00, 181.45}, {120.00, 180.31}, {110.00, 181.65}, { 95.00, 179.31}, { 63.00, 166.96}, { 50.00, 164.23}, { 31.00, 154.49}, { 19.76, 145.00}, { 15.96, 136.00}, { 16.65, 127.00}, { 20.57, 120.00},
	{ 28.00, 114.63}, { 40.00, 113.67}, { 65.00, 127.22}, { 73.00, 128.69}, { 81.96, 120.00}, { 77.58, 103.00}, { 78.18,  92.00}, { 59.11,  77.00}, { 52.00,  67.29}, { 31.29,  55.00}, { 25.67,  47.00}, { 24.65,  37.00},
	{ 27.82,  29.00}, { 35.00,  22.55}, { 44.00,  20.35}, { 49.00,  20.81}, { 61.00,  25.69}, { 79.00,  37.81}, { 88.00,  49.64}, { 97.00,  56.65}, {109.00,  49.61}, {143.00,  38.96}, {197.00,  37.27}, {215.00,  35.30},
	{222.00,  36.65}, {228.42,  41.00}, {233.30,  49.00}, {234.14,  57.00}, {231.00,  65.80}, {224.00,  72.38}, {218.00,  74.50}, {197.00,  76.62}, {145.00,  78.81}, {123.00,  87.41}, {117.59,  98.00}, {117.79, 104.00},
	{119.00, 106.23}, {138.73, 120.00}, {148.00, 129.50}, {158.50, 149.00}, {203.93, 175.00}, {229.00, 196.60}, {238.16, 208.00}, {245.20, 221.00}, {275.45, 245.00}, {289.00, 263.24}, {303.60, 287.00}, {312.00, 291.57},
	{339.25, 266.00}, {366.33, 226.00}, {363.43, 216.00}, {364.13, 206.00}, {353.00, 196.72}, {324.00, 181.05}, {307.00, 169.63}, {274.93, 156.00}, {256.00, 152.48}, {228.00, 145.13}, {221.09, 142.00}, {214.87, 135.00},
	{212.67, 127.00}, {213.81, 119.00}, {219.32, 111.00}, {228.00, 106.52}, {236.00, 106.39}, {290.00, 119.40}, {299.33, 114.00}, {300.52, 109.00}, {300.30,  53.00}, {301.46,  47.00}, {305.00,  41.12}, {311.00,  36.37},
	{317.00,  34.43}, {325.00,  34.81}, {334.90,  41.00}, {339.45,  50.00}, {339.82, 132.00}, {346.09, 139.00}, {350.00, 150.26}, {380.00, 167.38}, {393.00, 166.48}, {407.00, 155.54}, {430.00, 147.30}, {437.78, 135.00},
	{433.13, 122.00}, {410.23,  78.00}, {401.59,  69.00}, {393.48,  56.00}, {392.80,  44.00}, {395.50,  38.00}, {401.00,  32.49}, {409.00,  29.41}, {420.00,  30.84}, {426.92,  36.00}, {432.32,  44.00}, {439.49,  51.00},
	{470.13, 108.00}, {475.71, 124.00}, {483.00, 130.11}, {488.00, 139.43}, {529.00, 139.40}, {536.00, 132.52}, {543.73, 129.00}, {540.47, 115.00}, {541.11, 100.00}, {552.18,  68.00}, {553.78,  47.00}, {559.00,  39.76},
	{567.00,  35.52}, {577.00,  35.45}, {585.00,  39.58}, {591.38,  50.00}, {591.67,  66.00}, {590.31,  79.00}, {579.76, 109.00}, {582.25, 119.00}, {583.66, 136.00}, {586.45, 143.00}, {586.44, 151.00}, {580.42, 168.00},
	{577.15, 173.00}, {572.00, 177.13}, {564.00, 179.49}, {478.00, 178.81}, {443.00, 184.76}, {427.10, 190.00}, {424.00, 192.11}, {415.94, 209.00}, {408.82, 228.00}, {405.82, 241.00}, {411.00, 250.82}, {415.00, 251.50},
	{428.00, 248.89}, {469.00, 246.29}, {505.00, 246.49}, {533.00, 243.60}, {541.87, 248.00}, {547.55, 256.00}, {548.48, 267.00}, {544.00, 276.00}, {534.00, 282.24}, {513.00, 285.46}, {468.00, 285.76}, {402.00, 291.70},
	{392.00, 290.29}, {377.00, 294.46}, {367.00, 294.43}, {356.44, 304.00}, {354.22, 311.00}, {362.00, 321.36}, {390.00, 322.44}, {433.00, 330.16}, {467.00, 332.76}, {508.00, 347.64}, {522.00, 357.67}, {528.00, 354.46},
	{536.00, 352.96}, {546.06, 336.00}, {553.47, 306.00}, {564.19, 282.00}, {567.84, 268.00}, {578.72, 246.00}, {585.00, 240.97}, {592.00, 238.91}, {600.00, 239.72}, {606.00, 242.82}, {612.36, 251.00}, {613.35, 263.00},
	{588.75, 324.00}, {583.25, 350.00}, {572.12, 370.00}, {575.45, 378.00}, {575.20, 388.00}, {589.00, 393.81}, {599.20, 404.00}, {607.14, 416.00}, {609.96, 430.00}, {615.45, 441.00}, {613.44, 462.00}, {610.48, 469.00},
	{603.00, 475.63}, {590.96, 479.00}, 
};
static int complex_terrain_count = sizeof(complex_terrain_verts)/sizeof(cpVect);

static cpSpace *init_ComplexTerrainCircles_1000(){
	cpSpace *space = BENCH_SPACE_NEW();
	space->iterations = 10;
	space->gravity = cpv(0, -100);
	space->collisionSlop = 0.5f;
	
	cpVect offset = cpv(-320, -240);
	for(int i=0; i<(complex_terrain_count - 1); i++){
		cpVect a = complex_terrain_verts[i], b = complex_terrain_verts[i+1];
		cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpvadd(a, offset), cpvadd(b, offset), 0.0f));
	}
	
	for(int i=0; i<1000; i++){
		cpFloat radius = 5.0f;
		cpFloat mass = radius*radius;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
		body->p = cpvadd(cpvmult(frand_unit_circle(), 180.0f), cpv(0.0f, 300.0f));
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
		shape->e = 0.0f; shape->u = 0.0f;
	}
	
	return space;
}

static cpSpace *init_ComplexTerrainHexagons_1000(){
	cpSpace *space = BENCH_SPACE_NEW();
	space->iterations = 10;
	space->gravity = cpv(0, -100);
	space->collisionSlop = 0.5f;
	
	cpVect offset = cpv(-320, -240);
	for(int i=0; i<(complex_terrain_count - 1); i++){
		cpVect a = complex_terrain_verts[i], b = complex_terrain_verts[i+1];
		cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpvadd(a, offset), cpvadd(b, offset), 0.0f));
	}
	
	cpFloat radius = 5.0f;
	cpVect hexagon[6] = {};
	for(int i=0; i<6; i++){
		cpFloat angle = -M_PI*2.0f*i/6.0f;
		hexagon[i] = cpvmult(cpv(cos(angle), sin(angle)), radius);
	}
	
	for(int i=0; i<1000; i++){
		cpFloat mass = radius*radius;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, 6, hexagon, cpvzero)));
		body->p = cpvadd(cpvmult(frand_unit_circle(), 180.0f), cpv(0.0f, 300.0f));
		
		cpShape *shape = cpSpaceAddShape(space, cpPolyShapeNew(body, 6, hexagon, cpvzero));
		shape->e = 0.0f; shape->u = 0.0f;
	}
	
	return space;
}


// BouncyTerrain
static cpVect bouncy_terrain_verts[] = {
	{537.18,  23.00}, {520.50,  36.00}, {501.53,  63.00}, {496.14,  76.00}, {498.86,  86.00}, {504.00,  90.51}, {508.00,  91.36}, {508.77,  84.00}, {513.00,  77.73}, {519.00,  74.48}, {530.00,  74.67}, {545.00,  54.65},
	{554.00,  48.77}, {562.00,  46.39}, {568.00,  45.94}, {568.61,  47.00}, {567.94,  55.00}, {571.27,  64.00}, {572.92,  80.00}, {572.00,  81.39}, {563.00,  79.93}, {556.00,  82.69}, {551.49,  88.00}, {549.00,  95.76},
	{538.00,  93.40}, {530.00, 102.38}, {523.00, 104.00}, {517.00, 103.02}, {516.22, 109.00}, {518.96, 116.00}, {526.00, 121.15}, {534.00, 116.48}, {543.00, 116.77}, {549.28, 121.00}, {554.00, 130.17}, {564.00, 125.67},
	{575.60, 129.00}, {573.31, 121.00}, {567.77, 111.00}, {575.00, 106.47}, {578.51, 102.00}, {580.25,  95.00}, {577.98,  87.00}, {582.00,  85.71}, {597.00,  89.46}, {604.80,  95.00}, {609.28, 104.00}, {610.55, 116.00},
	{609.30, 125.00}, {600.80, 142.00}, {597.31, 155.00}, {584.00, 167.23}, {577.86, 175.00}, {583.52, 184.00}, {582.64, 195.00}, {591.00, 196.56}, {597.81, 201.00}, {607.45, 219.00}, {607.51, 246.00}, {600.00, 275.46},
	{588.00, 267.81}, {579.00, 264.91}, {557.00, 264.41}, {552.98, 259.00}, {548.00, 246.18}, {558.00, 247.12}, {565.98, 244.00}, {571.10, 237.00}, {571.61, 229.00}, {568.25, 222.00}, {562.00, 217.67}, {544.00, 213.93},
	{536.73, 214.00}, {535.60, 204.00}, {539.69, 181.00}, {542.84, 171.00}, {550.43, 161.00}, {540.00, 156.27}, {536.62, 152.00}, {534.70, 146.00}, {527.00, 141.88}, {518.59, 152.00}, {514.51, 160.00}, {510.33, 175.00},
	{519.38, 183.00}, {520.52, 194.00}, {516.00, 201.27}, {505.25, 206.00}, {507.57, 223.00}, {519.90, 260.00}, {529.00, 260.48}, {534.00, 262.94}, {538.38, 268.00}, {540.00, 275.00}, {537.06, 284.00}, {530.00, 289.23},
	{520.00, 289.23}, {513.00, 284.18}, {509.71, 286.00}, {501.69, 298.00}, {501.56, 305.00}, {504.30, 311.00}, {512.00, 316.43}, {521.00, 316.42}, {525.67, 314.00}, {535.00, 304.98}, {562.00, 294.80}, {573.00, 294.81},
	{587.52, 304.00}, {600.89, 310.00}, {596.96, 322.00}, {603.28, 327.00}, {606.52, 333.00}, {605.38, 344.00}, {597.65, 352.00}, {606.36, 375.00}, {607.16, 384.00}, {603.40, 393.00}, {597.00, 398.14}, {577.00, 386.15},
	{564.35, 373.00}, {565.21, 364.00}, {562.81, 350.00}, {553.00, 346.06}, {547.48, 338.00}, {547.48, 330.00}, {550.00, 323.30}, {544.00, 321.53}, {537.00, 322.70}, {532.00, 326.23}, {528.89, 331.00}, {527.83, 338.00},
	{533.02, 356.00}, {542.00, 360.73}, {546.68, 369.00}, {545.38, 379.00}, {537.58, 386.00}, {537.63, 388.00}, {555.00, 407.47}, {563.00, 413.52}, {572.57, 418.00}, {582.72, 426.00}, {578.00, 431.12}, {563.21, 440.00},
	{558.00, 449.27}, {549.00, 452.94}, {541.00, 451.38}, {536.73, 448.00}, {533.00, 441.87}, {520.00, 437.96}, {514.00, 429.69}, {490.00, 415.15}, {472.89, 399.00}, {472.03, 398.00}, {474.00, 396.71}, {486.00, 393.61},
	{492.00, 385.85}, {492.00, 376.15}, {489.04, 371.00}, {485.00, 368.11}, {480.00, 376.27}, {472.00, 379.82}, {463.00, 378.38}, {455.08, 372.00}, {446.00, 377.69}, {439.00, 385.24}, {436.61, 391.00}, {437.52, 404.00},
	{440.00, 409.53}, {463.53, 433.00}, {473.80, 441.00}, {455.00, 440.30}, {443.00, 436.18}, {436.00, 431.98}, {412.00, 440.92}, {397.00, 442.46}, {393.59, 431.00}, {393.71, 412.00}, {400.00, 395.10}, {407.32, 387.00},
	{408.54, 380.00}, {407.42, 375.00}, {403.97, 370.00}, {399.00, 366.74}, {393.00, 365.68}, {391.23, 374.00}, {387.00, 380.27}, {381.00, 383.52}, {371.56, 384.00}, {364.98, 401.00}, {362.96, 412.00}, {363.63, 435.00},
	{345.00, 433.55}, {344.52, 442.00}, {342.06, 447.00}, {337.00, 451.38}, {330.00, 453.00}, {325.00, 452.23}, {318.00, 448.17}, {298.00, 453.70}, {284.00, 451.49}, {278.62, 449.00}, {291.47, 408.00}, {291.77, 398.00},
	{301.00, 393.83}, {305.00, 393.84}, {305.60, 403.00}, {310.00, 409.47}, {318.00, 413.07}, {325.00, 412.40}, {332.31, 407.00}, {335.07, 400.00}, {334.40, 393.00}, {329.00, 385.69}, {319.00, 382.79}, {301.00, 389.23},
	{289.00, 389.97}, {265.00, 389.82}, {251.00, 385.85}, {245.00, 389.23}, {239.00, 389.94}, {233.00, 388.38}, {226.00, 382.04}, {206.00, 374.75}, {206.00, 394.00}, {204.27, 402.00}, {197.00, 401.79}, {191.00, 403.49},
	{186.53, 407.00}, {183.60, 412.00}, {183.60, 422.00}, {189.00, 429.31}, {196.00, 432.07}, {203.00, 431.40}, {209.47, 427.00}, {213.00, 419.72}, {220.00, 420.21}, {227.00, 418.32}, {242.00, 408.41}, {258.98, 409.00},
	{250.00, 435.43}, {239.00, 438.78}, {223.00, 448.19}, {209.00, 449.70}, {205.28, 456.00}, {199.00, 460.23}, {190.00, 460.52}, {182.73, 456.00}, {178.00, 446.27}, {160.00, 441.42}, {148.35, 435.00}, {149.79, 418.00},
	{157.72, 401.00}, {161.00, 396.53}, {177.00, 385.00}, {180.14, 380.00}, {181.11, 374.00}, {180.00, 370.52}, {170.00, 371.68}, {162.72, 368.00}, {158.48, 361.00}, {159.56, 349.00}, {154.00, 342.53}, {146.00, 339.85},
	{136.09, 343.00}, {130.64, 351.00}, {131.74, 362.00}, {140.61, 374.00}, {130.68, 387.00}, {120.75, 409.00}, {118.09, 421.00}, {117.92, 434.00}, {100.00, 432.40}, { 87.00, 427.48}, { 81.59, 423.00}, { 73.64, 409.00},
	{ 72.57, 398.00}, { 74.62, 386.00}, { 78.80, 378.00}, { 88.00, 373.43}, { 92.49, 367.00}, { 93.32, 360.00}, { 91.30, 353.00}, {103.00, 342.67}, {109.00, 343.10}, {116.00, 340.44}, {127.33, 330.00}, {143.00, 327.24},
	{154.30, 322.00}, {145.00, 318.06}, {139.77, 311.00}, {139.48, 302.00}, {144.95, 293.00}, {143.00, 291.56}, {134.00, 298.21}, {118.00, 300.75}, {109.40, 305.00}, { 94.67, 319.00}, { 88.00, 318.93}, { 81.00, 321.69},
	{ 67.24, 333.00}, { 56.68, 345.00}, { 53.00, 351.40}, { 47.34, 333.00}, { 50.71, 314.00}, { 56.57, 302.00}, { 68.00, 287.96}, { 91.00, 287.24}, {110.00, 282.36}, {133.80, 271.00}, {147.34, 256.00}, {156.47, 251.00},
	{157.26, 250.00}, {154.18, 242.00}, {154.48, 236.00}, {158.72, 229.00}, {166.71, 224.00}, {170.15, 206.00}, {170.19, 196.00}, {167.24, 188.00}, {160.00, 182.67}, {150.00, 182.66}, {143.60, 187.00}, {139.96, 195.00},
	{139.50, 207.00}, {136.45, 221.00}, {136.52, 232.00}, {133.28, 238.00}, {129.00, 241.38}, {119.00, 243.07}, {115.00, 246.55}, {101.00, 253.16}, { 86.00, 257.32}, { 63.00, 259.24}, { 57.00, 257.31}, { 50.54, 252.00},
	{ 47.59, 247.00}, { 46.30, 240.00}, { 47.58, 226.00}, { 50.00, 220.57}, { 58.00, 226.41}, { 69.00, 229.17}, { 79.00, 229.08}, { 94.50, 225.00}, {100.21, 231.00}, {107.00, 233.47}, {107.48, 224.00}, {109.94, 219.00},
	{115.00, 214.62}, {122.57, 212.00}, {116.00, 201.49}, {104.00, 194.57}, { 90.00, 194.04}, { 79.00, 198.21}, { 73.00, 198.87}, { 62.68, 191.00}, { 62.58, 184.00}, { 64.42, 179.00}, { 75.00, 167.70}, { 80.39, 157.00},
	{ 68.79, 140.00}, { 61.67, 126.00}, { 61.47, 117.00}, { 64.43, 109.00}, { 63.10,  96.00}, { 56.48,  82.00}, { 48.00,  73.88}, { 43.81,  66.00}, { 43.81,  56.00}, { 50.11,  46.00}, { 59.00,  41.55}, { 71.00,  42.64},
	{ 78.00,  36.77}, { 83.00,  34.75}, { 99.00,  34.32}, {117.00,  38.92}, {133.00,  55.15}, {142.00,  50.70}, {149.74,  51.00}, {143.55,  68.00}, {153.28,  74.00}, {156.23,  79.00}, {157.00,  84.00}, {156.23,  89.00},
	{153.28,  94.00}, {144.58,  99.00}, {151.52, 112.00}, {151.51, 124.00}, {150.00, 126.36}, {133.00, 130.25}, {126.71, 125.00}, {122.00, 117.25}, {114.00, 116.23}, {107.73, 112.00}, {104.48, 106.00}, {104.32,  99.00},
	{106.94,  93.00}, {111.24,  89.00}, {111.60,  85.00}, {107.24,  73.00}, {102.00,  67.57}, { 99.79,  67.00}, { 99.23,  76.00}, { 95.00,  82.27}, { 89.00,  85.52}, { 79.84,  86.00}, { 86.73, 114.00}, { 98.00, 136.73},
	{ 99.00, 137.61}, {109.00, 135.06}, {117.00, 137.94}, {122.52, 146.00}, {122.94, 151.00}, {121.00, 158.58}, {134.00, 160.97}, {153.00, 157.45}, {171.30, 150.00}, {169.06, 142.00}, {169.77, 136.00}, {174.00, 129.73},
	{181.46, 126.00}, {182.22, 120.00}, {182.20, 111.00}, {180.06, 101.00}, {171.28,  85.00}, {171.75,  80.00}, {182.30,  53.00}, {189.47,  50.00}, {190.62,  38.00}, {194.00,  33.73}, {199.00,  30.77}, {208.00,  30.48},
	{216.00,  34.94}, {224.00,  31.47}, {240.00,  30.37}, {247.00,  32.51}, {249.77,  35.00}, {234.75,  53.00}, {213.81,  93.00}, {212.08,  99.00}, {213.00, 101.77}, {220.00,  96.77}, {229.00,  96.48}, {236.28, 101.00},
	{240.00, 107.96}, {245.08, 101.00}, {263.00,  65.32}, {277.47,  48.00}, {284.00,  47.03}, {286.94,  41.00}, {292.00,  36.62}, {298.00,  35.06}, {304.00,  35.77}, {314.00,  43.81}, {342.00,  32.56}, {359.00,  31.32},
	{365.00,  32.57}, {371.00,  36.38}, {379.53,  48.00}, {379.70,  51.00}, {356.00,  52.19}, {347.00,  54.74}, {344.38,  66.00}, {341.00,  70.27}, {335.00,  73.52}, {324.00,  72.38}, {317.00,  65.75}, {313.00,  67.79},
	{307.57,  76.00}, {315.00,  78.62}, {319.28,  82.00}, {322.23,  87.00}, {323.00,  94.41}, {334.00,  92.49}, {347.00,  87.47}, {349.62,  80.00}, {353.00,  75.73}, {359.00,  72.48}, {366.00,  72.32}, {372.00,  74.94},
	{377.00,  81.34}, {382.00,  83.41}, {392.00,  83.40}, {399.00,  79.15}, {404.00,  85.74}, {411.00,  85.06}, {417.00,  86.62}, {423.38,  93.00}, {425.05, 104.00}, {438.00, 110.35}, {450.00, 112.17}, {452.62, 103.00},
	{456.00,  98.73}, {462.00,  95.48}, {472.00,  95.79}, {471.28,  92.00}, {464.00,  84.62}, {445.00,  80.39}, {436.00,  75.33}, {428.00,  68.46}, {419.00,  68.52}, {413.00,  65.27}, {408.48,  58.00}, {409.87,  46.00},
	{404.42,  39.00}, {408.00,  33.88}, {415.00,  29.31}, {429.00,  26.45}, {455.00,  28.77}, {470.00,  33.81}, {482.00,  42.16}, {494.00,  46.85}, {499.65,  36.00}, {513.00,  25.95}, {529.00,  22.42}, {537.18,  23.00}, 
};
static int bouncy_terrain_count = sizeof(bouncy_terrain_verts)/sizeof(cpVect);

static cpSpace *init_BouncyTerrainCircles_500(){
	cpSpace *space = BENCH_SPACE_NEW();
	space->iterations = 10;
	
	cpVect offset = cpv(-320, -240);
	for(int i=0; i<(bouncy_terrain_count - 1); i++){
		cpVect a = bouncy_terrain_verts[i], b = bouncy_terrain_verts[i+1];
		cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpvadd(a, offset), cpvadd(b, offset), 0.0f));
		shape->e = 1.0f;
	}
	
	for(int i=0; i<500; i++){
		cpFloat radius = 5.0f;
		cpFloat mass = radius*radius;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
		body->p = cpvadd(cpvmult(frand_unit_circle(), 130.0f), cpvzero);
		body->v = cpvmult(frand_unit_circle(), 50.0f);
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
		shape->e = 1.0f;
	}
	
	return space;
}

static cpSpace *init_BouncyTerrainHexagons_500(){
	cpSpace *space = BENCH_SPACE_NEW();
	space->iterations = 10;
	
	cpVect offset = cpv(-320, -240);
	for(int i=0; i<(bouncy_terrain_count - 1); i++){
		cpVect a = bouncy_terrain_verts[i], b = bouncy_terrain_verts[i+1];
		cpShape *shape = cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpvadd(a, offset), cpvadd(b, offset), 0.0f));
		shape->e = 1.0f;
	}
	
	cpFloat radius = 5.0f;
	cpVect hexagon[6] = {};
	for(int i=0; i<6; i++){
		cpFloat angle = -M_PI*2.0f*i/6.0f;
		hexagon[i] = cpvmult(cpv(cos(angle), sin(angle)), radius);
	}
	
	for(int i=0; i<500; i++){
		cpFloat mass = radius*radius;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, 6, hexagon, cpvzero)));
		body->p = cpvadd(cpvmult(frand_unit_circle(), 130.0f), cpvzero);
		body->v = cpvmult(frand_unit_circle(), 50.0f);
		
		cpShape *shape = cpSpaceAddShape(space, cpPolyShapeNew(body, 6, hexagon, cpvzero));
		shape->e = 1.0f;
	}
	
	return space;
}


// No collisions

static cpBool NoCollide_begin(cpArbiter *arb, cpSpace *space, void *data){
	abort();
	
	return cpTrue;
}


static cpSpace *init_NoCollide(){
	cpSpace *space = BENCH_SPACE_NEW();
	space->iterations = 10;
	
	cpSpaceAddCollisionHandler(space, 2, 2, NoCollide_begin, NULL, NULL, NULL, NULL);
	
	float radius = 4.5f;
	
	cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpv(-330-radius, -250-radius), cpv( 330+radius, -250-radius), 0.0f))->e = 1.0f;
	cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpv( 330+radius,  250+radius), cpv( 330+radius, -250-radius), 0.0f))->e = 1.0f;
	cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpv( 330+radius,  250+radius), cpv(-330-radius,  250+radius), 0.0f))->e = 1.0f;
	cpSpaceAddShape(space, cpSegmentShapeNew(space->staticBody, cpv(-330-radius, -250-radius), cpv(-330-radius,  250+radius), 0.0f))->e = 1.0f;
	
	for(int x=-320; x<=320; x+=20){
		for(int y=-240; y<=240; y+=20){
			cpSpaceAddShape(space, cpCircleShapeNew(space->staticBody, radius, cpv(x, y)));
		}
	}
	
	for(int y=10-240; y<=240; y+=40){
		cpFloat mass = 7.0f;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
		body->p = cpv(-320.0f, y);
		body->v = cpv(100.0f, 0.0f);
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
		shape->e = 1.0f;
		shape->collision_type = 2;
	}
	
	for(int x=30-320; x<=320; x+=40){
		cpFloat mass = 7.0f;
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, radius, cpvzero)));
		body->p = cpv(x, -240.0f);
		body->v = cpv(0.0f, 100.0f); 
		
		cpShape *shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
		shape->e = 1.0f;
		shape->collision_type = 2;
	}
	
	return space;
}


// TODO ideas:
// addition/removal
// Memory usage? (too small to matter?)
// http://forums.tigsource.com/index.php?topic=18077.msg518578#msg518578


// Build benchmark list
static void update(cpSpace *space){
	BENCH_SPACE_STEP(space, 1.0f/60.0f);
}

static void destroy(cpSpace *space){
	ChipmunkDemoFreeSpaceChildren(space);
	BENCH_SPACE_FREE(space);
}

// Make a second demo declaration for this demo to use in the regular demo set.
ChipmunkDemo BouncyHexagons = {
	"Bouncy Hexagons",
	init_BouncyTerrainHexagons_500,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};

#define BENCH(n) {"benchmark - " #n, init_##n, update, 	ChipmunkDemoDefaultDrawImpl, destroy}
ChipmunkDemo bench_list[] = {
	BENCH(SimpleTerrainCircles_1000),
	BENCH(SimpleTerrainCircles_500),
	BENCH(SimpleTerrainCircles_100),
	BENCH(SimpleTerrainBoxes_1000),
	BENCH(SimpleTerrainBoxes_500),
	BENCH(SimpleTerrainBoxes_100),
	BENCH(SimpleTerrainHexagons_1000),
	BENCH(SimpleTerrainHexagons_500),
	BENCH(SimpleTerrainHexagons_100),
	BENCH(SimpleTerrainVCircles_200),
	BENCH(SimpleTerrainVBoxes_200),
	BENCH(SimpleTerrainVHexagons_200),
	BENCH(ComplexTerrainCircles_1000),
	BENCH(ComplexTerrainHexagons_1000),
	BENCH(BouncyTerrainCircles_500),
	BENCH(BouncyTerrainHexagons_500),
	BENCH(NoCollide),
};

int bench_count = sizeof(bench_list)/sizeof(ChipmunkDemo);
