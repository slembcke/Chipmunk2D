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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

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

#define LINE_COLOR 0.0f, 0.0f, 0.0f
#define COLLISION_COLOR 1.0f, 0.0f, 0.0f
#define BODY_COLOR 0.0f, 0.0f, 1.0f

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
	
//	GLfloat v = (GLfloat)val/(GLfloat)ULONG_MAX;
//	v = 0.95f - v*0.15f;
//	
//	glColor3f(v, v, v);

	GLubyte r = (val>>0) & 0xFF;
	GLubyte g = (val>>8) & 0xFF;
	GLubyte b = (val>>16) & 0xFF;
	
	GLubyte max = r>g ? (r>b ? r : b) : (g>b ? g : b);
	
	const int mult = 127;
	const int add = 63;
	r = (r*mult)/max + add;
	g = (g*mult)/max + add;
	b = (b*mult)/max + add;
	
	glColor3ub(r, g, b);
}

static const GLfloat circleVAR[] = {
	 0.0000,  1.0000,
	 0.2588,  0.9659,
	 0.5000,  0.8660,
	 0.7071,  0.7071,
	 0.8660,  0.5000,
	 0.9659,  0.2588,
	 1.0000,  0.0000,
	 0.9659, -0.2588,
	 0.8660, -0.5000,
	 0.7071, -0.7071,
	 0.5000, -0.8660,
	 0.2588, -0.9659,
	 0.0000, -1.0000,
	-0.2588, -0.9659,
	-0.5000, -0.8660,
	-0.7071, -0.7071,
	-0.8660, -0.5000,
	-0.9659, -0.2588,
	-1.0000, -0.0000,
	-0.9659,  0.2588,
	-0.8660,  0.5000,
	-0.7071,  0.7071,
	-0.5000,  0.8660,
	-0.2588,  0.9659,
	 0.0000,  1.0000,
	 0.0f, 0.0f, // For an extra line to see the rotation.
};
static const int circleVAR_count = sizeof(circleVAR)/sizeof(GLfloat)/2;

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

static const GLfloat pillVAR[] = {
	 0.0000,  1.0000,
	 0.2588,  0.9659,
	 0.5000,  0.8660,
	 0.7071,  0.7071,
	 0.8660,  0.5000,
	 0.9659,  0.2588,
	 1.0000,  0.0000,
	 0.9659, -0.2588,
	 0.8660, -0.5000,
	 0.7071, -0.7071,
	 0.5000, -0.8660,
	 0.2588, -0.9659,
	 0.0000, -1.0000,

	 0.0000, -1.0000,
	-0.2588, -0.9659,
	-0.5000, -0.8660,
	-0.7071, -0.7071,
	-0.8660, -0.5000,
	-0.9659, -0.2588,
	-1.0000, -0.0000,
	-0.9659,  0.2588,
	-0.8660,  0.5000,
	-0.7071,  0.7071,
	-0.5000,  0.8660,
	-0.2588,  0.9659,
	 0.0000,  1.0000,
};
static const int pillVAR_count = sizeof(pillVAR)/sizeof(GLfloat)/2;

static void
drawSegmentShape(cpBody *body, cpSegmentShape *seg)
{
	cpVect a = cpvadd(body->p, cpvrotate(seg->a, body->rot));
	cpVect b = cpvadd(body->p, cpvrotate(seg->b, body->rot));
	
	if(seg->r){
		cpVect delta = cpvsub(b, a);
		cpFloat len = cpvlength(delta)/seg->r;
		
		GLfloat VAR[pillVAR_count*2];
		memcpy(VAR, pillVAR, sizeof(pillVAR));
		
		for(int i=0, half=pillVAR_count; i<half; i+=2)
			VAR[i] += len;
			
		glVertexPointer(2, GL_FLOAT, 0, VAR);
		glPushMatrix(); {
			GLfloat x = a.x;
			GLfloat y = a.y;
			GLfloat cos = delta.x/len;
			GLfloat sin = delta.y/len;

			const GLfloat matrix[] = {
				 cos,  sin, 0.0f, 0.0f,
				-sin,  cos, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f,
					 x,    y, 0.0f, 1.0f,
			};
			
			glMultMatrixf(matrix);
				
			glColor_from_pointer(seg);
			glDrawArrays(GL_TRIANGLE_FAN, 0, pillVAR_count);
			
			glColor3f(LINE_COLOR);
			glDrawArrays(GL_LINE_LOOP, 0, pillVAR_count);
		} glPopMatrix();
	} else {
		glBegin(GL_LINES); {
			glVertex2f(a.x, a.y);
			glVertex2f(b.x, b.y);
		} glEnd();
	}
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

static void
drawBB(void *ptr, void *unused)
{
	cpShape *shape = (cpShape *)ptr;

	glBegin(GL_LINE_LOOP); {
		glVertex2f(shape->bb.l, shape->bb.b);
		glVertex2f(shape->bb.l, shape->bb.t);
		glVertex2f(shape->bb.r, shape->bb.t);
		glVertex2f(shape->bb.r, shape->bb.b);
	} glEnd();
}

static void
drawCollisions(void *ptr, void *data)
{
	cpArbiter *arb = (cpArbiter *)ptr;
	for(int i=0; i<arb->numContacts; i++){
		cpVect v = arb->contacts[i].p;
		glVertex2f(v.x, v.y);
	}
}

void
drawSpace(cpSpace *space, drawSpaceOptions *options)
{
	glLineWidth(1.0f);
	if(options->drawBBs){
		glColor3f(0.6, 1.0, 0.6);
		cpSpaceHashEach(space->activeShapes, &drawBB, NULL);
		cpSpaceHashEach(space->staticShapes, &drawBB, NULL);
	}

	glLineWidth(options->lineThickness);
	if(options->drawShapes){
		cpSpaceHashEach(space->activeShapes, &drawObject, NULL);
		cpSpaceHashEach(space->staticShapes, &drawObject, NULL);
	}
	
	cpArray *bodies = space->bodies;
	int num = bodies->num;
	
	if(options->bodyPointSize){
		glPointSize(options->bodyPointSize);
		glBegin(GL_POINTS); {
			glColor3f(LINE_COLOR);
			for(int i=0; i<num; i++){
				cpBody *body = (cpBody *)bodies->arr[i];
				glVertex2f(body->p.x, body->p.y);
			}
		} glEnd();
	}

	if(options->collisionPointSize){
		glPointSize(options->collisionPointSize);
		glBegin(GL_POINTS); {
			glColor3f(COLLISION_COLOR);
			cpArrayEach(space->arbiters, &drawCollisions, NULL);
		} glEnd();
	}
}
