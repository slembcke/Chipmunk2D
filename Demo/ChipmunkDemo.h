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

#include "ChipmunkDebugDraw.h"

typedef struct ChipmunkDemo ChipmunkDemo;

typedef cpSpace *(*ChipmunkDemoInitFunc)(void);
typedef void (*ChipmunkDemoUpdateFunc)(cpSpace *space, double dt);
typedef void (*ChipmunkDemoDrawFunc)(cpSpace *space);
typedef void (*ChipmunkDemoDestroyFunc)(cpSpace *space);

struct ChipmunkDemo {
	const char *name;
	double timestep;
 
	ChipmunkDemoInitFunc initFunc;
	ChipmunkDemoUpdateFunc updateFunc;
	ChipmunkDemoDrawFunc drawFunc;
	
	ChipmunkDemoDestroyFunc destroyFunc;
};

static inline cpFloat
frand(void)
{
	return (cpFloat)rand()/(cpFloat)RAND_MAX;
}

static inline cpVect
frand_unit_circle(){
	cpVect v = cpv(frand()*2.0f - 1.0f, frand()*2.0f - 1.0f);
	return (cpvlengthsq(v) < 1.0f ? v : frand_unit_circle());
}

extern int ChipmunkDemoTicks;
extern double ChipmunkDemoTime;
extern cpVect ChipmunkDemoKeyboard;
extern cpVect ChipmunkDemoMouse;
extern cpBool ChipmunkDemoRightClick;
extern cpBool ChipmunkDemoRightDown;

extern char *ChipmunkDemoMessageString;
void ChipmunkDemoPrintString(char *fmt, ...);

extern cpShapeFilter GRAB_FILTER;
extern cpShapeFilter NOT_GRABBABLE_FILTER;

void ChipmunkDemoDefaultDrawImpl(cpSpace *space);
void ChipmunkDemoFreeSpaceChildren(cpSpace *space);
