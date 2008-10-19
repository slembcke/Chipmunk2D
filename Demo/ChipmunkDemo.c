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

extern chipmunkDemo Test;
extern chipmunkDemo LogoSmash;
extern chipmunkDemo PyramidStack;
extern chipmunkDemo Plink;
extern chipmunkDemo Tumble;
extern chipmunkDemo PyramidTopple;
extern chipmunkDemo Bounce;
extern chipmunkDemo Planet;
extern chipmunkDemo Springies;
extern chipmunkDemo Pump;

static chipmunkDemo *demos[] = {
	&Test,
	&LogoSmash,
	&PyramidStack,
	&Plink,
	&Tumble,
	&PyramidTopple,
	&Bounce,
	&Planet,
	&Springies,
	&Pump,
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

drawSpaceOptions options = {
	0,
	1,
	0.0f,
	0.0f,
	1.5f,
};

static void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	drawSpace(space, currDemo->drawOptions ? currDemo->drawOptions : &options);
	
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
	sprintf(title, "Demo: %s (a thru %c switch demos)", currDemo->name, 'a' + demoCount - 1);
	
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
idle(void)
{
	glutPostRedisplay();
}

static void
initGL(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);

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

	glutMotionFunc(mouse);
	glutPassiveMotionFunc(mouse);
	glutMouseFunc(click);
	
	glutMainLoop();
}

int
main(int argc, const char **argv)
{
	cpInitChipmunk();
		
	mouseBody = cpBodyNew(INFINITY, INFINITY);
	runDemo(demos[firstDemoIndex]);
	
	glutStuff(argc, argv);
	return 0;
}
