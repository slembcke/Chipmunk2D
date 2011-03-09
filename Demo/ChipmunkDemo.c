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
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

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
#include "drawSpace.h"
#include "ChipmunkDemo.h"

#define SLEEP_TICKS 16

static chipmunkDemo *demos;
static int demoCount = 0;
static chipmunkDemo *currDemo = NULL;
static const int firstDemoIndex = 'a' - 'a';

static int paused = 0;
static int step = 0;
static int ticks = 0;
static cpSpace *space;

cpVect mousePoint;
cpVect mousePoint_last;
cpBody *mouseBody = NULL;
cpConstraint *mouseJoint = NULL;

char messageString[1024] = {};

int key_up = 0;
int key_down = 0;
int key_left = 0;
int key_right = 0;

cpVect arrowDirection = {};

drawSpaceOptions options = {
	0,
	0,
	1,
	4.0f,
	0.0f,
	1.5f,
};

static void
drawString(int x, int y, const char *str)
{
	glColor3f(0.0f, 0.0f, 0.0f);
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
		"Arrow keys control some demos.\n"
		"\\ disables anti-aliasing.\n"
		"- toggles spatial hash visualization.\n"
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
		"KE:% 5.2e";
	
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
		constraints, maxConstraints, (ke < 1e-10f ? 0.0f : ke)
	);
	
	drawString(0, 220, buffer);
}

static void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	
	double scale = 0.5/cpfmin(width/640.0, height/480.0);
	double hw = width*scale;
	double hh = height*scale;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-hw, hw, -hh, hh, -1.0, 1.0);
	glTranslated(0.5, 0.5, 0.0);
}

static void
display(void)
{
	cpVect newPoint = cpvlerp(mousePoint_last, mousePoint, 0.25f);
	mouseBody->p = newPoint;
	mouseBody->v = cpvmult(cpvsub(newPoint, mousePoint_last), 60.0f);
	mousePoint_last = newPoint;
  if(!paused || step > 0){
    currDemo->updateFunc(ticks);
    step = (step > 1 ? step - 1 : 0);
  }
  
	glClear(GL_COLOR_BUFFER_BIT);
	
	drawSpace(space, currDemo->drawOptions ? currDemo->drawOptions : &options);
	drawInstructions();
	drawInfo();
	drawString(-300, -210, messageString);
		
	glutSwapBuffers();
	ticks++;
}

static char *
demoTitle(chipmunkDemo *demo)
{
	static char title[1024];
	sprintf(title, "Demo: %s", demo->name);
	
	return title;
}

static void
runDemo(chipmunkDemo *demo)
{
	srand(45073);
	
	if(currDemo)
		currDemo->destroyFunc();
		
	currDemo = demo;
	ticks = 0;
	mouseJoint = NULL;
	messageString[0] = '\0';
	maxArbiters = 0;
	maxPoints = 0;
	maxConstraints = 0;
	space = currDemo->initFunc();

	glutSetWindowTitle(demoTitle(currDemo));
}

static void
keyboard(unsigned char key, int x, int y)
{
	int index = key - 'a';
	
	if(0 <= index && index < demoCount){
		runDemo(&demos[index]);
	} else if(key == '\r'){
		runDemo(currDemo);
  } else if(key == '`'){
		paused = !paused;
  } else if(key == '1'){
		step += 1;
	} else if(key == '-'){
		options.drawHash = !options.drawHash;
	} else if(key == '='){
		options.drawBBs = !options.drawBBs;
	} else if(key == '\\'){
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
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
	mousePoint = mouseToSpace(x, y);
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
				mouseJoint->biasCoef = 0.15f;
				cpSpaceAddConstraint(space, mouseJoint);
			}
		} else if(mouseJoint){
			cpSpaceRemoveConstraint(space, mouseJoint);
			cpConstraintFree(mouseJoint);
			mouseJoint = NULL;
		}
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
	
	arrowDirection = cpv(x, y);
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
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);
}

static void
glutStuff(int argc, const char *argv[])
{
	glutInit(&argc, (char**)argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	
	glutInitWindowSize(640, 480);
	glutCreateWindow(demoTitle(&demos[firstDemoIndex]));
	
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
	currDemo = &demos[index];
	space = currDemo->initFunc();
	
	double start_time = GetMilliseconds();
	
	for(int i=0; i<count; i++)
		currDemo->updateFunc(i);
	
	double end_time = GetMilliseconds();
	
	currDemo->destroyFunc();
	
	printf("Time(%c) = %8.2f ms (%s)\n", index + 'a', end_time - start_time, currDemo->name);
}

extern chipmunkDemo LogoSmash;
extern chipmunkDemo Simple;
extern chipmunkDemo PyramidStack;
extern chipmunkDemo Plink;
extern chipmunkDemo Tumble;
extern chipmunkDemo PyramidTopple;
extern chipmunkDemo Bounce;
extern chipmunkDemo Planet;
extern chipmunkDemo Springies;
extern chipmunkDemo Pump;
extern chipmunkDemo TheoJansen;
extern chipmunkDemo MagnetsElectric;
extern chipmunkDemo UnsafeOps;
extern chipmunkDemo Query;
extern chipmunkDemo OneWay;
extern chipmunkDemo Player;
extern chipmunkDemo Sensors;
extern chipmunkDemo Joints;
extern chipmunkDemo Tank;

extern chipmunkDemo bench_list[];
extern int bench_count;

int
main(int argc, const char **argv)
{
	chipmunkDemo demo_list[] = {
		LogoSmash,
		Simple,
		PyramidStack,
		Plink,
		Tumble,
		PyramidTopple,
		Bounce,
		Planet,
		Springies,
		Pump,
		TheoJansen,
		MagnetsElectric,
		UnsafeOps,
		Query,
		OneWay,
		Player,
		Sensors,
		Joints,
		Tank,
	};
	
	demos = demo_list;
	demoCount = sizeof(demo_list)/sizeof(chipmunkDemo);
	int trial = 0;
	
	for(int i=0; i<argc; i++){
		if(strcmp(argv[i], "-bench") == 0){
			demos = bench_list;
			demoCount = bench_count;
		} else if(strcmp(argv[i], "-trial") == 0){
			trial = 1;
		}
	}
	
	cpInitChipmunk();
	
	if(trial){
		sleep(1);
		for(int i=0; i<demoCount; i++) time_trial(i, 1000);
		exit(0);
	} else {
		mouseBody = cpBodyNew(INFINITY, INFINITY);
		
		glutStuff(argc, argv);
		
		runDemo(&demos[firstDemoIndex]);
		glutMainLoop();
	}

	return 0;
}
