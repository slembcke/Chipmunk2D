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

#define SLEEP_TICKS 16

#define CLEAR_COLOR 1.0f, 1.0f, 1.0f
#define LINE_COLOR 0.0f, 0.0f, 0.0f
#define COLLISION_COLOR 1.0f, 0.0f, 0.0f
#define BODY_COLOR 0.0f, 0.0f, 1.0f

extern void demo1_init(void);
extern void demo1_update(int);

extern void demo2_init(void);
extern void demo2_update(int);

extern void demo3_init(void);
extern void demo3_update(int);

extern void demo4_init(void);
extern void demo4_update(int);

extern void demo5_init(void);
extern void demo5_update(int);

extern void demo6_init(void);
extern void demo6_update(int);

extern void demo7_init(void);
extern void demo7_update(int);


typedef void (*demo_init_func)(void);
typedef void (*demo_update_func)(int);
typedef void (*demo_destroy_func)(void);

demo_init_func init_funcs[] = {
	demo1_init,
	demo2_init,
	demo3_init,
	demo4_init,
	demo5_init,
	demo6_init,
	demo7_init,
};

demo_update_func update_funcs[] = {
	demo1_update,
	demo2_update,
	demo3_update,
	demo4_update,
	demo5_update,
	demo6_update,
	demo7_update,
};

void demo_destroy(void);

demo_destroy_func destroy_funcs[] = {
	demo_destroy,
	demo_destroy,
	demo_destroy,
	demo_destroy,
	demo_destroy,
	demo_destroy,
	demo_destroy,
};

int demo_index = 6;

int ticks = 0;
cpSpace *space;
cpBody *staticBody;

cpVect mousePoint;
cpVect mousePoint_last;
cpBody *mouseBody = NULL;
cpConstraint *mouseJoint = NULL;

void demo_destroy(void)
{
	cpSpaceFreeChildren(space);
	cpSpaceFree(space);
	
	cpBodyFree(staticBody);
}


static void
glColor_from_pointer(void *ptr)
{
	unsigned long val = (long)ptr;
	
	// hash the pointer up nicely
	val = (val+0x7ed55d16) + (val<<12);
	val = (val^0xc761c23c) ^ (val>>19);
	val = (val+0x165667b1) + (val<<5);
	val = (val+0xd3a2646c) ^ (val<<9);
	val = (val+0xfd7046c5) + (val<<3);
	val = (val^0xb55a4f09) ^ (val>>16);
	
	GLfloat v = (GLfloat)val/(GLfloat)ULONG_MAX;
	v = 0.95f - v*0.15f;
	
	glColor3f(v, v, v);

//	GLubyte r = (val>>0) & 0xFF;
//	GLubyte g = (val>>8) & 0xFF;
//	GLubyte b = (val>>16) & 0xFF;
//	
//	GLubyte max = r>g ? (r>b ? r : b) : (g>b ? g : b);
//	
//	const int mult = 127;
//	const int add = 127;
//	r = (r*mult)/max + add;
//	g = (g*mult)/max + add;
//	b = (b*mult)/max + add;
//	
//	glColor3ub(r, g, b);
}

GLfloat circleVAR[] = {
	 1.0000f,  0.0000f,
	 0.9135f,  0.4067f,
	 0.6691f,  0.7431f,
	 0.3090f,  0.9511f,
	-0.1045f,  0.9945f,
	-0.5000f,  0.8660f,
	-0.8090f,  0.5878f,
	-0.9781f,  0.2079f,
	-0.9781f, -0.2079f,
	-0.8090f, -0.5878f,
	-0.5000f, -0.8660f,
	-0.1045f, -0.9945f,
	 0.3090f, -0.9511f,
	 0.6691f, -0.7431f,
	 0.9135f, -0.4067f,
	 1.0000f,  0.0000f,
	 0.0f, 0.0f, // For an extra line to see the rotation.
};
int circleVAR_count = sizeof(circleVAR)/sizeof(GLfloat)/2;

static void
drawCircleShape(cpBody *body, cpCircleShape *circle)
{
	glVertexPointer(2, GL_FLOAT, 0, circleVAR);

	glPushMatrix(); {
		cpVect center = cpvadd(body->p, cpvrotate(circle->c, body->rot));
		glTranslatef(center.x, center.y, 0.0f);
		glRotatef(body->a*180.0/M_PI, 0.0f, 0.0f, 1.0f);
		glScalef(circle->r, circle->r, 1.0f);
		
		glColor_from_pointer(circle);
		glDrawArrays(GL_TRIANGLE_FAN, 0, circleVAR_count - 1);
		
		glColor3f(LINE_COLOR);
		glDrawArrays(GL_LINE_STRIP, 0, circleVAR_count);
	} glPopMatrix();
}

static void
drawSegmentShape(cpBody *body, cpSegmentShape *seg)
{
	cpVect a = cpvadd(body->p, cpvrotate(seg->a, body->rot));
	cpVect b = cpvadd(body->p, cpvrotate(seg->b, body->rot));
	
	glColor3f(LINE_COLOR);
	glBegin(GL_LINES); {
		glVertex2f(a.x, a.y);
		glVertex2f(b.x, b.y);
	} glEnd();
}

static void
drawPolyShape(cpBody *body, cpPolyShape *poly)
{
	int count = count=poly->numVerts;
	GLfloat VAR[count*2];
	glVertexPointer(2, GL_FLOAT, 0, VAR);

	cpVect *verts = poly->verts;
	for(int i=0; i<count; i++){
		cpVect v = cpvadd(body->p, cpvrotate(verts[i], body->rot));
		VAR[2*i    ] = v.x;
		VAR[2*i + 1] = v.y;
	}

	glColor_from_pointer(poly);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);
	
	glColor3f(LINE_COLOR);
	glDrawArrays(GL_LINE_LOOP, 0, count);
}

static void
drawObject(void *ptr, void *unused)
{
	cpShape *shape = (cpShape *)ptr;
	cpBody *body = shape->body;
	
	switch(shape->klass->type){
		case CP_CIRCLE_SHAPE:
			drawCircleShape(body, (cpCircleShape *)shape);
			break;
		case CP_SEGMENT_SHAPE:
			drawSegmentShape(body, (cpSegmentShape *)shape);
			break;
		case CP_POLY_SHAPE:
			drawPolyShape(body, (cpPolyShape *)shape);
			break;
		default:
			printf("Bad enumeration in drawObject().\n");
	}
}

//static void
//drawBB(void *ptr, void *unused)
//{
//	cpShape *shape = (cpShape *)ptr;
//
//	glBegin(GL_LINE_LOOP); {
//		glVertex2f(shape->bb.l, shape->bb.b);
//		glVertex2f(shape->bb.l, shape->bb.t);
//		glVertex2f(shape->bb.r, shape->bb.t);
//		glVertex2f(shape->bb.r, shape->bb.b);
//	} glEnd();
//}

static void
drawCollisions(void *ptr, void *data)
{
	cpArbiter *arb = (cpArbiter *)ptr;
	for(int i=0; i<arb->numContacts; i++){
		cpVect v = arb->contacts[i].p;
		glVertex2f(v.x, v.y);
	}
}

//static void 
//pickingFunc(cpShape *shape, void *data)
//{
//	drawObject(shape, NULL);
//}

static void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
//	glColor3f(0.6, 1.0, 0.6);
//	cpSpaceHashEach(space->activeShapes, &drawBB, NULL);
//	cpSpaceHashEach(space->staticShapes, &drawBB, NULL);
	
	cpSpaceHashEach(space->activeShapes, &drawObject, NULL);
	cpSpaceHashEach(space->staticShapes, &drawObject, NULL);
	
//	glColor3f(1.0, 0.0, 0.0);
//	cpSpaceShapePointQuery(space, mousePoint, pickingFunc, NULL);
//	cpSpaceStaticShapePointQuery(space, mousePoint, pickingFunc, NULL);
	
	cpArray *bodies = space->bodies;
	int num = bodies->num;
	
	glBegin(GL_POINTS); {
		glColor3f(BODY_COLOR);
		for(int i=0; i<num; i++){
			cpBody *body = (cpBody *)bodies->arr[i];
			glVertex2f(body->p.x, body->p.y);
		}
		
		glColor3f(COLLISION_COLOR);
		cpArrayEach(space->arbiters, &drawCollisions, NULL);
	} glEnd();
	
	glutSwapBuffers();
	ticks++;
	
	cpVect newPoint = cpvadd(mousePoint_last, cpvmult(cpvsub(mousePoint, mousePoint_last), 0.25f));
	mouseBody->p = newPoint;
	mouseBody->v = cpvmult(cpvsub(newPoint, mousePoint_last), 60.0);
	mousePoint_last = newPoint;
	update_funcs[demo_index](ticks);
}

static void
keyboard(unsigned char key, int x, int y)
{
	int new_index = key - '1';
	
	if(0 <= new_index && new_index < 7){
		destroy_funcs[demo_index]();
		
		demo_index = new_index;
		ticks = 0;
		mouseJoint = NULL;
		init_funcs[demo_index]();
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
	glClearColor(CLEAR_COLOR, 0.0);

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
	glutCreateWindow("Press 1-7 to switch demos");
	
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
	init_funcs[demo_index]();
	
	glutStuff(argc, argv);
	return 0;
}
