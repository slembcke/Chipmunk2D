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

#ifdef __APPLE__
	#include "OpenGL/gl.h"
	#include "OpenGL/glu.h"
	#include <GLUT/glut.h>
#else
#ifdef WIN32
	#include <windows.h>
#endif
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#endif

#include "chipmunk_private.h"
#include "ChipmunkDemo.h"

#define SLEEP_TICKS 16

static ChipmunkDemo *demos;
static int demoCount = 0;
static int demoIndex = 'a' - 'a';

static cpBool paused = cpFalse;
static cpBool step = cpFalse;

static cpBool drawBBs = cpFalse;

static cpSpace *space;


int ChipmunkDemoTicks = 0;
cpFloat ChipmunkDemoTime;

static cpBody *mouseBody = NULL;
static cpConstraint *mouseJoint = NULL;
cpVect ChipmunkDemoMouse;
cpBool ChipmunkDemoRightClick = cpFalse;
cpBool ChipmunkDemoRightDown = cpFalse;

char *ChipmunkDemoMessageString = NULL;

static int key_up = 0;
static int key_down = 0;
static int key_left = 0;
static int key_right = 0;
cpVect ChipmunkDemoKeyboard = {};

GLfloat translate_x = 0.0;
GLfloat translate_y = 0.0;
GLfloat scale = 1.0;

static void shapeFreeWrap(cpSpace *space, cpShape *shape, void *unused){
	cpSpaceRemoveShape(space, shape);
	cpShapeFree(shape);
}

static void postShapeFree(cpShape *shape, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)shapeFreeWrap, shape, NULL);
}

static void constraintFreeWrap(cpSpace *space, cpConstraint *constraint, void *unused){
	cpSpaceRemoveConstraint(space, constraint);
	cpConstraintFree(constraint);
}

static void postConstraintFree(cpConstraint *constraint, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)constraintFreeWrap, constraint, NULL);
}

static void bodyFreeWrap(cpSpace *space, cpBody *body, void *unused){
	cpSpaceRemoveBody(space, body);
	cpBodyFree(body);
}

static void postBodyFree(cpBody *body, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)bodyFreeWrap, body, NULL);
}

// Safe and future proof way to remove and free all objects that have been added to the space.
void
ChipmunkDemoFreeSpaceChildren(cpSpace *space)
{
	// Must remove these BEFORE freeing the body or you will access dangling pointers.
	cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)postShapeFree, space);
	cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)postConstraintFree, space);
	
	cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)postBodyFree, space);
}

void ChipmunkDemoDefaultDrawImpl(cpSpace *space)
{
	ChipmunkDebugDrawShapes(space);
	ChipmunkDebugDrawConstraints(space);
	
	ChipmunkDebugDrawCollisionPoints(space);
}


static void
drawString(int x, int y, const char *str)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(x, y);
	
	for(int i=0, len=strlen(str); i<len; i++){
		if(str[i] == '\n'){
			y -= 16;
			glRasterPos2i(x, y);
		} else if(str[i] == '*'){ // print out the last demo key
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'A' + demoCount - 1);
		} else {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, str[i]);
		}
	}
}

static void
drawInstructions()
{
	drawString(-300, 220,
		"Controls:\n"
		"A - * Switch demos. (return restarts)\n"
		"Use the mouse to grab objects.\n"
		"= toggles bounding boxes."
	);
}

static int maxArbiters = 0;
static int maxPoints = 0;
static int maxConstraints = 0;

static void
drawInfo()
{
	int arbiters = space->arbiters->num;
	int points = 0;
	
	for(int i=0; i<arbiters; i++)
		points += ((cpArbiter *)(space->arbiters->arr[i]))->numContacts;
	
	int constraints = (space->constraints->num + points)*space->iterations;
	
	maxArbiters = arbiters > maxArbiters ? arbiters : maxArbiters;
	maxPoints = points > maxPoints ? points : maxPoints;
	maxConstraints = constraints > maxConstraints ? constraints : maxConstraints;
	
	char buffer[1024];
	const char *format = 
		"Arbiters: %d (%d) - "
		"Contact Points: %d (%d)\n"
		"Other Constraints: %d, Iterations: %d\n"
		"Constraints x Iterations: %d (%d)\n"
		"Time:% 5.2fs, KE:% 5.2e";
	
	cpArray *bodies = space->bodies;
	cpFloat ke = 0.0f;
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody *)bodies->arr[i];
		if(body->m == INFINITY || body->i == INFINITY) continue;
		
		ke += body->m*cpvdot(body->v, body->v) + body->i*body->w*body->w;
	}
	
	sprintf(buffer, format,
		arbiters, maxArbiters,
		points, maxPoints,
		space->constraints->num, space->iterations,
		constraints, maxConstraints,
		ChipmunkDemoTime, (ke < 1e-10f ? 0.0f : ke)
	);
	
	drawString(0, 220, buffer);
}

static void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	
	double scale = cpfmin(width/640.0, height/480.0);
	double hw = width*(0.5/scale);
	double hh = height*(0.5/scale);
	
	ChipmunkDebugDrawPointLineScale = scale;
	glLineWidth(scale);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-hw, hw, -hh, hh, -1.0, 1.0);
	glTranslated(0.5, 0.5, 0.0);
}

static void
drawShapeBB(cpShape *shape, void *unused)
{
	ChipmunkDebugDrawBB(shape->bb, RGBAColor(0.3f, 0.5f, 0.3f, 1.0f));
}

static char PrintStringBuffer[1024*8];
static char *PrintStringCursor;

void
ChipmunkDemoPrintString(char *fmt, ...)
{
	ChipmunkDemoMessageString = PrintStringBuffer;
	
	va_list args;
	va_start(args, fmt);
	PrintStringCursor += vsprintf(PrintStringCursor, fmt, args);
	va_end(args);
}

static void
display(void)
{
	PrintStringBuffer[0] = 0;
	PrintStringCursor = PrintStringBuffer;
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(scale, scale, 1.0);
	glTranslatef(translate_x, translate_y, 0.0);
	
	demos[demoIndex].drawFunc(space);
	
	if(!paused || step){
		cpVect newPoint = cpvlerp(mouseBody->p, ChipmunkDemoMouse, 0.25f);
		mouseBody->v = cpvmult(cpvsub(newPoint, mouseBody->p), 60.0f);
		mouseBody->p = newPoint;
		
		demos[demoIndex].updateFunc(space);
		
		ChipmunkDemoTicks++;
		ChipmunkDemoTime = ChipmunkDemoTicks/60.0;
		
		step = cpFalse;
		ChipmunkDemoRightDown = cpFalse;
	}
  
	if(drawBBs) cpSpaceEachShape(space, drawShapeBB, NULL);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); {
		// Draw the text at fixed positions,
		// but save the drawing matrix for the mouse picking
		glLoadIdentity();
		
		drawInstructions();
		drawInfo();
		drawString(-300, -200, ChipmunkDemoMessageString);
	} glPopMatrix();
		
	glutSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
}

static char *
demoTitle(int index)
{
	static char title[1024];
	sprintf(title, "Demo(%c): %s", 'a' + index, demos[demoIndex].name);
	
	return title;
}

static void
runDemo(int index)
{
	srand(45073);
	
	demoIndex = index;
	ChipmunkDemoTicks = 0;
	mouseJoint = NULL;
	ChipmunkDemoMessageString = "";
	maxArbiters = 0;
	maxPoints = 0;
	maxConstraints = 0;
	space = demos[demoIndex].initFunc();

	glutSetWindowTitle(demoTitle(index));
}

static void
keyboard(unsigned char key, int x, int y)
{
	int index = key - 'a';
	
	if(0 <= index && index < demoCount){
		demos[demoIndex].destroyFunc(space);
		runDemo(index);
	} else if(key == '\r'){
		demos[demoIndex].destroyFunc(space);
		runDemo(demoIndex);
  } else if(key == '`'){
		paused = !paused;
  } else if(key == '1'){
		step = cpTrue;
	} else if(key == '='){
		drawBBs = !drawBBs;
	} else if(key == '\\'){
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
	
	GLfloat translate_increment = 50.0/scale;
	GLfloat scale_increment = 1.2;
	if(key == '5'){
		translate_x = 0.0;
		translate_y = 0.0;
		scale = 1.0;
	}else if(key == '4'){
		translate_x += translate_increment;
	}else if(key == '6'){
		translate_x -= translate_increment;
	}else if(key == '2'){
		translate_y += translate_increment;
	}else if(key == '8'){
		translate_y -= translate_increment;
	}else if(key == '7'){
		scale /= scale_increment;
	}else if(key == '9'){
		scale *= scale_increment;
	}
}

static cpVect
mouseToSpace(int x, int y)
{
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	
	GLdouble proj[16];
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	
	GLint view[4];
	glGetIntegerv(GL_VIEWPORT, view);
	
	GLdouble mx, my, mz;
	gluUnProject(x, glutGet(GLUT_WINDOW_HEIGHT) - y, 0.0f, model, proj, view, &mx, &my, &mz);
	
	return cpv(mx, my);
}

static void
mouse(int x, int y)
{
	ChipmunkDemoMouse = mouseToSpace(x, y);
}

static void
click(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON){
		if(state == GLUT_DOWN){
			cpVect point = mouseToSpace(x, y);
		
			cpShape *shape = cpSpacePointQueryFirst(space, point, GRABABLE_MASK_BIT, CP_NO_GROUP);
			if(shape){
				cpBody *body = shape->body;
				mouseJoint = cpPivotJointNew2(mouseBody, body, cpvzero, cpBodyWorld2Local(body, point));
				mouseJoint->maxForce = 50000.0f;
				mouseJoint->errorBias = cpfpow(1.0f - 0.15f, 60.0f);
				cpSpaceAddConstraint(space, mouseJoint);
			}
		} else if(mouseJoint){
			cpSpaceRemoveConstraint(space, mouseJoint);
			cpConstraintFree(mouseJoint);
			mouseJoint = NULL;
		}
	} else if(button == GLUT_RIGHT_BUTTON){
		ChipmunkDemoRightDown = ChipmunkDemoRightClick = (state == GLUT_DOWN);
	}
}

static void
timercall(int value)
{
	glutTimerFunc(SLEEP_TICKS, timercall, 0);
		
	glutPostRedisplay();
}

static void
set_arrowDirection()
{
	int x = 0, y = 0;
	
	if(key_up) y += 1;
	if(key_down) y -= 1;
	if(key_right) x += 1;
	if(key_left) x -= 1;
	
	ChipmunkDemoKeyboard = cpv(x, y);
}

static void
arrowKeyDownFunc(int key, int x, int y)
{
	if(key == GLUT_KEY_UP) key_up = 1;
	else if(key == GLUT_KEY_DOWN) key_down = 1;
	else if(key == GLUT_KEY_LEFT) key_left = 1;
	else if(key == GLUT_KEY_RIGHT) key_right = 1;

	set_arrowDirection();
}

static void
arrowKeyUpFunc(int key, int x, int y)
{
	if(key == GLUT_KEY_UP) key_up = 0;
	else if(key == GLUT_KEY_DOWN) key_down = 0;
	else if(key == GLUT_KEY_LEFT) key_left = 0;
	else if(key == GLUT_KEY_RIGHT) key_right = 0;

	set_arrowDirection();
}

//static void
//idle(void)
//{
//	glutPostRedisplay();
//}

static void
initGL(void)
{
	glClearColor(52.0/255.0, 62.0/255.0, 72.0/255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void
glutStuff(int argc, const char *argv[])
{
	glutInit(&argc, (char**)argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	
	glutInitWindowSize(640, 480);
	glutCreateWindow(demoTitle(demoIndex));
	
	initGL();
	
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
//	glutIdleFunc(idle);
	glutTimerFunc(SLEEP_TICKS, timercall, 0);

	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(keyboard);
	
	glutSpecialFunc(arrowKeyDownFunc);
	glutSpecialUpFunc(arrowKeyUpFunc);

	glutMotionFunc(mouse);
	glutPassiveMotionFunc(mouse);
	glutMouseFunc(click);
}

#ifdef WIN32

static double GetMilliseconds(){
	__int64 count, freq;
	QueryPerformanceCounter((LARGE_INTEGER*)&count);
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

	return 1000.0*(double)count/(double)freq;
}

#else

#include <sys/time.h>
#include <unistd.h>

static double GetMilliseconds(){
	struct timeval time;
	gettimeofday(&time, NULL);
	
	return (time.tv_sec*1000.0 + time.tv_usec/1000.0);
}

#endif

void time_trial(int index, int count)
{
	space = demos[index].initFunc();
	
	double start_time = GetMilliseconds();
	
	for(int i=0; i<count; i++)
		demos[index].updateFunc(space);
	
	double end_time = GetMilliseconds();
	
	demos[index].destroyFunc(space);
	
	printf("Time(%c) = %8.2f ms (%s)\n", index + 'a', end_time - start_time, demos[index].name);
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

extern ChipmunkDemo bench_list[];
extern int bench_count;

int
main(int argc, const char **argv)
{
	ChipmunkDemo demo_list[] = {
		LogoSmash,
		PyramidStack,
		Plink,
		BouncyHexagons,
		Tumble,
		PyramidTopple,
		Planet,
		Springies,
		Pump,
		TheoJansen,
		Query,
		OneWay,
		Joints,
		Tank,
		Chains,
		Crane,
		ContactGraph,
		Buoyancy,
		Player,
		Slice,
		Convex,
		Unicycle,
		Sticky,
		Shatter,
	};
	
	demos = demo_list;
	demoCount = sizeof(demo_list)/sizeof(ChipmunkDemo);
	int trial = 0;
	
	for(int i=0; i<argc; i++){
		if(strcmp(argv[i], "-bench") == 0){
			demos = bench_list;
			demoCount = bench_count;
		} else if(strcmp(argv[i], "-trial") == 0){
			trial = 1;
		}
	}
	
	if(trial){
//		sleep(1);
		for(int i=0; i<demoCount; i++) time_trial(i, 1000);
//		time_trial('d' - 'a', 10000);
		exit(0);
	} else {
		mouseBody = cpBodyNew(INFINITY, INFINITY);
		
		glutStuff(argc, argv);
		
		runDemo(demoIndex);
		glutMainLoop();
	}

	return 0;
}
