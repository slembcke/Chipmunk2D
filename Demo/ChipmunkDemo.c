/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
/*
	IMPORTANT - READ ME!
	
	This file sets up a simple interface that the individual demos can use to get
	a Chipmunk space running and draw what's in it. In order to keep the Chipmunk
	examples clean and simple, they contain no graphics code. All drawing is done
	by accessing the Chipmunk structures at a very low level. It is NOT
	recommended to write a game or application this way as it does not scale
	beyond simple shape drawing and is very dependent on implementation details
	about Chipmunk which may change with little to no warning.
*/
 
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "GL/glew.h"
#include "GL/glfw.h"

#include "chipmunk/chipmunk_private.h"
#include "ChipmunkDemo.h"
#include "ChipmunkDemoTextSupport.h"

static ChipmunkDemo *demos;
static int demo_count = 0;
static int demo_index = 'a' - 'a';

static cpBool paused = cpFalse;
static cpBool step = cpFalse;

static cpSpace *space;

static double Accumulator = 0.0;
static double LastTime = 0.0;
int ChipmunkDemoTicks = 0;
double ChipmunkDemoTime;

cpVect ChipmunkDemoMouse;
cpBool ChipmunkDemoRightClick = cpFalse;
cpBool ChipmunkDemoRightDown = cpFalse;
cpVect ChipmunkDemoKeyboard = {};

static cpBody *mouse_body = NULL;
static cpConstraint *mouse_joint = NULL;

char const *ChipmunkDemoMessageString = NULL;

#define GRABBABLE_MASK_BIT (1<<31)
cpShapeFilter GRAB_FILTER = {CP_NO_GROUP, GRABBABLE_MASK_BIT, GRABBABLE_MASK_BIT};
cpShapeFilter NOT_GRABBABLE_FILTER = {CP_NO_GROUP, ~GRABBABLE_MASK_BIT, ~GRABBABLE_MASK_BIT};

cpVect translate = {0, 0};
cpFloat scale = 1.0;

static void ShapeFreeWrap(cpSpace *space, cpShape *shape, void *unused){
	cpSpaceRemoveShape(space, shape);
	cpShapeFree(shape);
}

static void PostShapeFree(cpShape *shape, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ShapeFreeWrap, shape, NULL);
}

static void ConstraintFreeWrap(cpSpace *space, cpConstraint *constraint, void *unused){
	cpSpaceRemoveConstraint(space, constraint);
	cpConstraintFree(constraint);
}

static void PostConstraintFree(cpConstraint *constraint, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ConstraintFreeWrap, constraint, NULL);
}

static void BodyFreeWrap(cpSpace *space, cpBody *body, void *unused){
	cpSpaceRemoveBody(space, body);
	cpBodyFree(body);
}

static void PostBodyFree(cpBody *body, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)BodyFreeWrap, body, NULL);
}

// Safe and future proof way to remove and free all objects that have been added to the space.
void
ChipmunkDemoFreeSpaceChildren(cpSpace *space)
{
	// Must remove these BEFORE freeing the body or you will access dangling pointers.
	cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)PostShapeFree, space);
	cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)PostConstraintFree, space);
	
	cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)PostBodyFree, space);
}

static void
DrawCircle(cpVect p, cpFloat a, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawCircle(p, a, r, outline, fill);}

static void
DrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data)
{ChipmunkDebugDrawSegment(a, b, color);}

static void
DrawFatSegment(cpVect a, cpVect b, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawFatSegment(a, b, r, outline, fill);}

static void
DrawPolygon(int count, const cpVect *verts, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawPolygon(count, verts, r, outline, fill);}

static void
DrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data)
{ChipmunkDebugDrawDot(size, pos, color);}

static cpSpaceDebugColor
ColorForShape(cpShape *shape, cpDataPointer data)
{
	if(cpShapeGetSensor(shape)){
		return LAColor(1.0f, 0.1f);
	} else {
		cpBody *body = cpShapeGetBody(shape);
		
		if(cpBodyIsSleeping(body)){
			return LAColor(0.2f, 1.0f);
		} else if(body->sleeping.idleTime > shape->space->sleepTimeThreshold) {
			return LAColor(0.66f, 1.0f);
		} else {
			uint32_t val = (uint32_t)shape->hashid;
			
			// scramble the bits up using Robert Jenkins' 32 bit integer hash function
			val = (val+0x7ed55d16) + (val<<12);
			val = (val^0xc761c23c) ^ (val>>19);
			val = (val+0x165667b1) + (val<<5);
			val = (val+0xd3a2646c) ^ (val<<9);
			val = (val+0xfd7046c5) + (val<<3);
			val = (val^0xb55a4f09) ^ (val>>16);
			
			GLfloat r = (GLfloat)((val>>0) & 0xFF);
			GLfloat g = (GLfloat)((val>>8) & 0xFF);
			GLfloat b = (GLfloat)((val>>16) & 0xFF);
			
			GLfloat max = (GLfloat)cpfmax(cpfmax(r, g), b);
			GLfloat min = (GLfloat)cpfmin(cpfmin(r, g), b);
			GLfloat intensity = (cpBodyGetType(body) == CP_BODY_TYPE_STATIC ? 0.15f : 0.75f);
			
			// Saturate and scale the color
			if(min == max){
				return RGBAColor(intensity, 0.0f, 0.0f, 1.0f);
			} else {
				GLfloat coef = (GLfloat)intensity/(max - min);
				return RGBAColor(
					(r - min)*coef,
					(g - min)*coef,
					(b - min)*coef,
					1.0f
				);
			}
		}
	}
}


void
ChipmunkDemoDefaultDrawImpl(cpSpace *space)
{
	cpSpaceDebugDrawOptions drawOptions = {
		DrawCircle,
		DrawSegment,
		DrawFatSegment,
		DrawPolygon,
		DrawDot,
		
		(cpSpaceDebugDrawFlags)(CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_CONSTRAINTS | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS),
		
		{200.0f/255.0f, 210.0f/255.0f, 230.0f/255.0f, 1.0f},
		ColorForShape,
		{0.0f, 0.75f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f},
		NULL,
	};
	
	cpSpaceDebugDraw(space, &drawOptions);
}

static void
DrawInstructions()
{
	ChipmunkDemoTextDrawString(cpv(-300, 220),
		"Controls:\n"
		"A - * Switch demos. (return restarts)\n"
		"Use the mouse to grab objects.\n"
	);
}

static int max_arbiters = 0;
static int max_points = 0;
static int max_constraints = 0;

static void
DrawInfo()
{
	int arbiters = space->arbiters->num;
	int points = 0;
	
	for(int i=0; i<arbiters; i++)
		points += ((cpArbiter *)(space->arbiters->arr[i]))->count;
	
	int constraints = (space->constraints->num + points)*space->iterations;
	
	max_arbiters = arbiters > max_arbiters ? arbiters : max_arbiters;
	max_points = points > max_points ? points : max_points;
	max_constraints = constraints > max_constraints ? constraints : max_constraints;
	
	char buffer[1024];
	const char *format = 
		"Arbiters: %d (%d) - "
		"Contact Points: %d (%d)\n"
		"Other Constraints: %d, Iterations: %d\n"
		"Constraints x Iterations: %d (%d)\n"
		"Time:% 5.2fs, KE:% 5.2e";
	
	cpArray *bodies = space->dynamicBodies;
	cpFloat ke = 0.0f;
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody *)bodies->arr[i];
		if(body->m == INFINITY || body->i == INFINITY) continue;
		
		ke += body->m*cpvdot(body->v, body->v) + body->i*body->w*body->w;
	}
	
	sprintf(buffer, format,
		arbiters, max_arbiters,
		points, max_points,
		space->constraints->num, space->iterations,
		constraints, max_constraints,
		ChipmunkDemoTime, (ke < 1e-10f ? 0.0f : ke)
	);
	
	ChipmunkDemoTextDrawString(cpv(0, 220), buffer);
}

static char PrintStringBuffer[1024*8];
static char *PrintStringCursor;

void
ChipmunkDemoPrintString(char const *fmt, ...)
{
	ChipmunkDemoMessageString = PrintStringBuffer;
	
	va_list args;
	va_start(args, fmt);
	// TODO: should use vsnprintf here
	PrintStringCursor += vsprintf(PrintStringCursor, fmt, args);
	va_end(args);
}

static void
Tick(double dt)
{
	if(!paused || step){
		PrintStringBuffer[0] = 0;
		PrintStringCursor = PrintStringBuffer;
		
		// Completely reset the renderer only at the beginning of a tick.
		// That way it can always display at least the last ticks' debug drawing.
		ChipmunkDebugDrawClearRenderer();
		ChipmunkDemoTextClearRenderer();
		
		cpVect new_point = cpvlerp(mouse_body->p, ChipmunkDemoMouse, 0.25f);
		mouse_body->v = cpvmult(cpvsub(new_point, mouse_body->p), 60.0f);
		mouse_body->p = new_point;
		
		demos[demo_index].updateFunc(space, dt);
		
		ChipmunkDemoTicks++;
		ChipmunkDemoTime += dt;
		
		step = cpFalse;
		ChipmunkDemoRightDown = cpFalse;
		
		ChipmunkDemoTextDrawString(cpv(-300, -200), ChipmunkDemoMessageString);
	}
}

static void
Update(void)
{
	double time = glfwGetTime();
	double dt = time - LastTime;
	if(dt > 0.2) dt = 0.2;
	
	double fixed_dt = demos[demo_index].timestep;
	
	for(Accumulator += dt; Accumulator > fixed_dt; Accumulator -= fixed_dt){
		Tick(fixed_dt);
	}
	
	LastTime = time;
}

static void
Display(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef((GLfloat)translate.x, (GLfloat)translate.y, 0.0f);
	glScalef((GLfloat)scale, (GLfloat)scale, 1.0f);
	
	Update();
	
	ChipmunkDebugDrawPushRenderer();
	demos[demo_index].drawFunc(space);
	
//	// Highlight the shape under the mouse because it looks neat.
//	cpShape *nearest = cpSpacePointQueryNearest(space, ChipmunkDemoMouse, 0.0f, CP_ALL_LAYERS, CP_NO_GROUP, NULL);
//	if(nearest) ChipmunkDebugDrawShape(nearest, RGBAColor(1.0f, 0.0f, 0.0f, 1.0f), LAColor(0.0f, 0.0f));
	
	// Draw the renderer contents and reset it back to the last tick's state.
	ChipmunkDebugDrawFlushRenderer();
	ChipmunkDebugDrawPopRenderer();
	
	ChipmunkDemoTextPushRenderer();
	// Now render all the UI text.
	DrawInstructions();
	DrawInfo();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); {
		// Draw the text at fixed positions,
		// but save the drawing matrix for the mouse picking
		glLoadIdentity();
		
		ChipmunkDemoTextFlushRenderer();
		ChipmunkDemoTextPopRenderer();
	} glPopMatrix();
	
	glfwSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
}

static void
Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	
	float scale = (float)cpfmin(width/640.0, height/480.0);
	float hw = width*(0.5f/scale);
	float hh = height*(0.5f/scale);
	
	ChipmunkDebugDrawPointLineScale = scale;
	glLineWidth((GLfloat)scale);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-hw, hw, -hh, hh);
}

static char *
DemoTitle(int index)
{
	static char title[1024];
	sprintf(title, "Demo(%c): %s", 'a' + index, demos[demo_index].name);
	
	return title;
}

static void
RunDemo(int index)
{
	srand(45073);
	
	demo_index = index;
	
	ChipmunkDemoTicks = 0;
	ChipmunkDemoTime = 0.0;
	Accumulator = 0.0;
	LastTime = glfwGetTime();
	
	mouse_joint = NULL;
	ChipmunkDemoMessageString = "";
	max_arbiters = 0;
	max_points = 0;
	max_constraints = 0;
	space = demos[demo_index].initFunc();

	glfwSetWindowTitle(DemoTitle(index));
}

static void
Keyboard(int key, int state)
{
	if(state == GLFW_RELEASE) return;
	
	int index = key - 'a';
	
	if(0 <= index && index < demo_count){
		demos[demo_index].destroyFunc(space);
		RunDemo(index);
	} else if(key == ' '){
		demos[demo_index].destroyFunc(space);
		RunDemo(demo_index);
  } else if(key == '`'){
		paused = !paused;
  } else if(key == '1'){
		step = cpTrue;
	} else if(key == '\\'){
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
	
	GLfloat translate_increment = 50.0f/(GLfloat)scale;
	GLfloat scale_increment = 1.2f;
	if(key == '5'){
		translate.x = 0.0f;
		translate.y = 0.0f;
		scale = 1.0f;
	}else if(key == '4'){
		translate.x += translate_increment;
	}else if(key == '6'){
		translate.x -= translate_increment;
	}else if(key == '2'){
		translate.y += translate_increment;
	}else if(key == '8'){
		translate.y -= translate_increment;
	}else if(key == '7'){
		scale /= scale_increment;
	}else if(key == '9'){
		scale *= scale_increment;
	}
}

static cpVect
MouseToSpace(int x, int y)
{
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	
	GLdouble proj[16];
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
 	
	GLint view[4];
	glGetIntegerv(GL_VIEWPORT, view);
	
	int ww, wh;
	glfwGetWindowSize(&ww, &wh);
	
	GLdouble mx, my, mz;
	gluUnProject(x, wh - y, 0.0f, model, proj, view, &mx, &my, &mz);
	
	return cpv(mx, my);
}

static void
Mouse(int x, int y)
{
	ChipmunkDemoMouse = MouseToSpace(x, y);
}

static void
Click(int button, int state)
{
	if(button == GLFW_MOUSE_BUTTON_1){
		if(state == GLFW_PRESS){
			// give the mouse click a little radius to make it easier to click small shapes.
			cpFloat radius = 5.0;
			
			cpPointQueryInfo info = {};
			cpShape *shape = cpSpacePointQueryNearest(space, ChipmunkDemoMouse, radius, GRAB_FILTER, &info);
			
			if(shape && cpBodyGetMass(cpShapeGetBody(shape)) < INFINITY){
				// Use the closest point on the surface if the click is outside of the shape.
				cpVect nearest = (info.distance > 0.0f ? info.point : ChipmunkDemoMouse);
				
				cpBody *body = cpShapeGetBody(shape);
				mouse_joint = cpPivotJointNew2(mouse_body, body, cpvzero, cpBodyWorldToLocal(body, nearest));
				mouse_joint->maxForce = 50000.0f;
				mouse_joint->errorBias = cpfpow(1.0f - 0.15f, 60.0f);
				cpSpaceAddConstraint(space, mouse_joint);
			}
		} else if(mouse_joint){
			cpSpaceRemoveConstraint(space, mouse_joint);
			cpConstraintFree(mouse_joint);
			mouse_joint = NULL;
		}
	} else if(button == GLFW_MOUSE_BUTTON_2){
		ChipmunkDemoRightDown = ChipmunkDemoRightClick = (state == GLFW_PRESS);
	}
}

static void
SpecialKeyboard(int key, int state)
{
	switch(key){
		case GLFW_KEY_UP   : ChipmunkDemoKeyboard.y += (state == GLFW_PRESS ?  1.0 : -1.0); break;
		case GLFW_KEY_DOWN : ChipmunkDemoKeyboard.y += (state == GLFW_PRESS ? -1.0 :  1.0); break;
		case GLFW_KEY_RIGHT: ChipmunkDemoKeyboard.x += (state == GLFW_PRESS ?  1.0 : -1.0); break;
		case GLFW_KEY_LEFT : ChipmunkDemoKeyboard.x += (state == GLFW_PRESS ? -1.0 :  1.0); break;
		default: break;
	}
}

static int
WindowClose()
{
	glfwTerminate();
	exit(EXIT_SUCCESS);
	
	return GL_TRUE;
}

static void
SetupGL(void)
{
	glewExperimental = GL_TRUE;
	cpAssertHard(glewInit() == GLEW_NO_ERROR, "There was an error initializing GLEW.");
	cpAssertHard(GLEW_ARB_vertex_array_object, "Requires VAO support.");
	
	ChipmunkDebugDrawInit();
	ChipmunkDemoTextInit();
	
	glClearColor(52.0f/255.0f, 62.0f/255.0f, 72.0f/255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

static void
SetupGLFW()
{
	cpAssertHard(glfwInit(), "Error initializing GLFW.");
	
	cpAssertHard(glfwOpenWindow(640, 480, 8, 8, 8, 8, 0, 0, GLFW_WINDOW), "Error opening GLFW window.");
	glfwSetWindowTitle(DemoTitle(demo_index));
	glfwSwapInterval(1);
	
	SetupGL();
	
	glfwSetWindowSizeCallback(Reshape);
	glfwSetWindowCloseCallback(WindowClose);
	
	glfwSetCharCallback(Keyboard);
	glfwSetKeyCallback(SpecialKeyboard);
	
	glfwSetMousePosCallback(Mouse);
	glfwSetMouseButtonCallback(Click);
}

static void
TimeTrial(int index, int count)
{
	space = demos[index].initFunc();
	
	double start_time = glfwGetTime();
	double dt = demos[index].timestep;
	
	for(int i=0; i<count; i++)
		demos[index].updateFunc(space, dt);
	
	double end_time = glfwGetTime();
	
	demos[index].destroyFunc(space);
	
	printf("Time(%c) = %8.2f ms (%s)\n", index + 'a', (end_time - start_time)*1e3f, demos[index].name);
	fflush(stdout);
}

extern ChipmunkDemo LogoSmash;
extern ChipmunkDemo PyramidStack;
extern ChipmunkDemo Plink;
extern ChipmunkDemo BouncyHexagons;
extern ChipmunkDemo Tumble;
extern ChipmunkDemo PyramidTopple;
extern ChipmunkDemo Planet;
extern ChipmunkDemo Springies;
extern ChipmunkDemo Pump;
extern ChipmunkDemo TheoJansen;
extern ChipmunkDemo Query;
extern ChipmunkDemo OneWay;
extern ChipmunkDemo Player;
extern ChipmunkDemo Joints;
extern ChipmunkDemo Tank;
extern ChipmunkDemo Chains;
extern ChipmunkDemo Crane;
extern ChipmunkDemo Buoyancy;
extern ChipmunkDemo ContactGraph;
extern ChipmunkDemo Slice;
extern ChipmunkDemo Convex;
extern ChipmunkDemo Unicycle;
extern ChipmunkDemo Sticky;
extern ChipmunkDemo Shatter;
extern ChipmunkDemo GJK;

extern ChipmunkDemo bench_list[];
extern int bench_count;

int
main(int argc, const char **argv)
{
	ChipmunkDemo demo_list[] = {
		LogoSmash,//A
		PyramidStack,//B
		Plink,//C
		BouncyHexagons,//D
		Tumble,//E
		PyramidTopple,//F
		Planet,//G
		Springies,//H
		Pump,//I
		TheoJansen,//J
		Query,//K
		OneWay,//L
		Joints,//M
		Tank,//N
		Chains,//O
		Crane,//P
		ContactGraph,//Q
		Buoyancy,//R
		Player,//S
		Slice,//T
		Convex,//U
		Unicycle,//V
		Sticky,//W
		Shatter,//X
	};
	
	demos = demo_list;
	demo_count = sizeof(demo_list)/sizeof(ChipmunkDemo);
	int trial = 0;
	
	for(int i=0; i<argc; i++){
		if(strcmp(argv[i], "-bench") == 0){
			demos = bench_list;
			demo_count = bench_count;
		} else if(strcmp(argv[i], "-trial") == 0){
			trial = 1;
		}
	}
	
	if(trial){
		cpAssertHard(glfwInit(), "Error initializing GLFW.");
//		sleep(1);
		for(int i=0; i<demo_count; i++) TimeTrial(i, 1000);
//		time_trial('d' - 'a', 10000);
		exit(0);
	} else {
		mouse_body = cpBodyNewKinematic();
		
		RunDemo(demo_index);
		SetupGLFW();
		
		while(1){
			Display();
		}
	}

	return 0;
}
