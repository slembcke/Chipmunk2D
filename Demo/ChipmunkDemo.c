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
//#include <sys/time.h>

#ifdef __APPLE__
	#include "OpenGL/gl.h"
	#include "OpenGL/glu.h"
	#include <GLUT/glut.h>
#else
#ifdef WIN32
	#include <windows.h>
#endif
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#endif

#include "chipmunk.h"
#include "drawSpace.h"
#include "ChipmunkDemo.h"

#define SLEEP_TICKS 16

//extern chipmunkDemo Test;
extern chipmunkDemo LogoSmash;
extern chipmunkDemo PyramidStack;
extern chipmunkDemo Plink;
extern chipmunkDemo Tumble;
extern chipmunkDemo PyramidTopple;
extern chipmunkDemo Bounce;
extern chipmunkDemo Planet;
extern chipmunkDemo Springies;
extern chipmunkDemo Pump;
extern chipmunkDemo WalkBot;
extern chipmunkDemo TheoJansen;

//extern chipmunkDemo Test;

static chipmunkDemo *demos[] = {
//	&Test,
	&LogoSmash,
	&PyramidStack,
	&Plink,
	&Tumble,
	&PyramidTopple,
	&Bounce,
	&Planet,
	&Springies,
	&Pump,
	&WalkBot,
	&TheoJansen,
};
static const int demoCount = sizeof(demos)/sizeof(chipmunkDemo *);
static chipmunkDemo *currDemo = NULL;
static const int firstDemoIndex = 'a' - 'a';

static int ticks = 0;
static cpSpace *space;

cpVect mousePoint;
cpVect mousePoint_last;
cpBody *mouseBody = NULL;
cpConstraint *mouseJoint = NULL;

int key_up = 0;
int key_down = 0;
int key_left = 0;
int key_right = 0;

cpVect arrowDirection = {};

drawSpaceOptions options = {
	0,
	1,
	0.0f,
	0.0f,
	1.5f,
};

static void
drawInstructions()
{
	int x = -300;
	int y =  220;
	
	char *str = (
		"Controls:\n"
		"A - * Switch demos.\n"
		"Use the mouse to grab objects.\n"
		"Arrow Keys control some demos.\n"
		"\\ enables anti-aliasing."
	);
	
	glColor3f(0.0f, 0.0f, 0.0f);
	glRasterPos2i(x, y);
	
	for(int i=0, len=strlen(str); i<len; i++){
		if(str[i] == '\n'){
			y -= 16;
			glRasterPos2i(x, y);
		} else if(str[i] == '*'){
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'A' + demoCount - 1);
		} else {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, str[i]);
		}
	}
}

static void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	drawSpace(space, currDemo->drawOptions ? currDemo->drawOptions : &options);
	drawInstructions();
		
	glutSwapBuffers();
	ticks++;
	
	cpVect newPoint = cpvadd(mousePoint_last, cpvmult(cpvsub(mousePoint, mousePoint_last), 0.25f));
	mouseBody->p = newPoint;
	mouseBody->v = cpvmult(cpvsub(newPoint, mousePoint_last), 60.0);
	mousePoint_last = newPoint;
	currDemo->updateFunc(ticks);
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
	if(currDemo)
		currDemo->destroyFunc();
		
	currDemo = demo;
	ticks = 0;
	mouseJoint = NULL;
	space = currDemo->initFunc();

	glutSetWindowTitle(demoTitle(currDemo));
}

static void
keyboard(unsigned char key, int x, int y)
{
	int index = key - 'a';
	
	if(0 <= index && index < demoCount){
		runDemo(demos[index]);
	} else if(key == '\r'){
		runDemo(currDemo);
	} else if(key == '\\'){
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
		glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);
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
findBody(cpShape *shape, void *data)
{
	cpBody **body_ptr = (cpBody **)data;
	*body_ptr = shape->body;
}

static void
click(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON){
		if(state == GLUT_DOWN){
			cpVect point = mouseToSpace(x, y);
		
			cpBody *body = NULL;
			cpSpaceShapePointQuery(space, point, findBody, &body);
			if(!body) return;
			
			mouseJoint = cpPivotJointNew(mouseBody, body, cpvzero, cpBodyWorld2Local(body, point));
			mouseJoint->maxForce = 50000.0f;
			mouseJoint->biasCoef = 0.15f;
			cpSpaceAddConstraint(space, mouseJoint);
		} else {
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

static void
idle(void)
{
	glutPostRedisplay();
}

static void
initGL(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-320.0, 320.0, -240.0, 240.0, -1.0, 1.0);
	glTranslatef(0.5, 0.5, 0.0);
	
	glEnableClientState(GL_VERTEX_ARRAY);
}

static void
glutStuff(int argc, const char *argv[])
{
	glutInit(&argc, (char**)argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	
	glutInitWindowSize(640, 480);
	glutCreateWindow(demoTitle(demos[firstDemoIndex]));
	
	initGL();
	
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

int
main(int argc, const char **argv)
{
	cpInitChipmunk();
		
	mouseBody = cpBodyNew(INFINITY, INFINITY);
	
	glutStuff(argc, argv);
	
	runDemo(demos[firstDemoIndex]);
	glutMainLoop();

	return 0;
}
