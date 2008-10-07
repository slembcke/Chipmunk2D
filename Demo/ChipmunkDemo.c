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

extern chipmunkDemo Demo1;

static chipmunkDemo *demos[] = {
	&Demo1,
};
static const int demoCount = sizeof(demos)/sizeof(chipmunkDemo *);
static chipmunkDemo *currDemo = NULL;

static int ticks = 0;
static cpSpace *space;

cpVect mousePoint;
cpVect mousePoint_last;
cpBody *mouseBody = NULL;
cpConstraint *mouseJoint = NULL;

drawSpaceOptions options = {
	0,
	1,
	0,
};

static void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	drawSpace(space, &options);
	
	glutSwapBuffers();
	ticks++;
	
	cpVect newPoint = cpvadd(mousePoint_last, cpvmult(cpvsub(mousePoint, mousePoint_last), 0.25f));
	mouseBody->p = newPoint;
	mouseBody->v = cpvmult(cpvsub(newPoint, mousePoint_last), 60.0);
	mousePoint_last = newPoint;
	currDemo->updateFunc(ticks);
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
	
	static char title[1024];
	sprintf(title, "Demo: %s (press a - %c to switch demos)", currDemo->name, 'a' + demoCount - 1);
	
	glutSetWindowTitle(title);
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

//static void
//idle(void)
//{
//	glutPostRedisplay();
//}

static void
initGL(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0);

	glPointSize(3.0);
	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_POINT_SMOOTH_HINT, GL_DONT_CARE);
	glLineWidth(1.5f);

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
	glutCreateWindow("REPLACE ME!");
	
	initGL();
	
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
//	glutIdleFunc(idle);
	glutTimerFunc(SLEEP_TICKS, timercall, 0);
//	glutMouseFunc(buttons);
	glutMotionFunc(mouse);
	glutPassiveMotionFunc(mouse);
	glutMouseFunc(click);
	
	glutMainLoop();
}

//void time_trial(int index, int count)
//{
//	demo_index = index;
//	init_funcs[demo_index]();
//	
//	struct timeval start_time, end_time;
//	gettimeofday(&start_time, NULL);
//	
//	for(int i=0; i<count; i++)
//		update_funcs[demo_index](i);
//	
//	gettimeofday(&end_time, NULL);
//	long millisecs = (end_time.tv_sec - start_time.tv_sec)*1000;
//	millisecs += (end_time.tv_usec - start_time.tv_usec)/1000;
//	
//	printf("Time(%d) = %ldms\n", index + 1, millisecs);
//}

int
main(int argc, const char **argv)
{
	cpInitChipmunk();
	
//	time_trial(1, 5000);
//	time_trial(2, 5000);
//	time_trial(3, 5000);
//	time_trial(4, 1000);
//	time_trial(6, 25000);
//	exit(0);
	
	mouseBody = cpBodyNew(INFINITY, INFINITY);
	runDemo(demos[0]);
	
	glutStuff(argc, argv);
	return 0;
}
