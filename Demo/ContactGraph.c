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
 
#include "chipmunk.h"
#include "ChipmunkDemo.h"

// static body that we will be making into a scale
static cpBody *scaleStaticBody;
static cpBody *ballBody;

// If your compiler supports blocks (Clang or some GCC versions),
// You can use the block based iterators instead of the function ones to make your life easier.
#if defined(__has_extension)
#if __has_extension(blocks)

#define USE_BLOCKS 1

#endif
#endif


#if !USE_BLOCKS

static void
ScaleIterator(cpBody *body, cpArbiter *arb, cpVect *sum)
{
	(*sum) = cpvadd(*sum, cpArbiterTotalImpulseWithFriction(arb));
}

static void
BallIterator(cpBody *body, cpArbiter *arb, int *count)
{
	// body is the body we are iterating the arbiters for.
	// CP_ARBITER_GET_*() in an arbiter iterator always returns the body/shape for the iterated body first.
	CP_ARBITER_GET_SHAPES(arb, ball, other);
	ChipmunkDebugDrawBB(cpShapeGetBB(other), RGBAColor(1, 0, 0, 1));
	
	(*count)++;
}

struct CrushingContext {
	cpFloat magnitudeSum;
	cpVect vectorSum;
};

static void
EstimateCrushing(cpBody *body, cpArbiter *arb, struct CrushingContext *context)
{
	cpVect j = cpArbiterTotalImpulseWithFriction(arb);
	context->magnitudeSum += cpvlength(j);
	context->vectorSum = cpvadd(context->vectorSum, j);
}

#endif

static void
update(cpSpace *space)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
	
	ChipmunkDemoPrintString("Place objects on the scale to weigh them. The ball marks the shapes it's sitting on.\n");
	
	// Sum the total impulse applied to the scale from all collision pairs in the contact graph.
	// If your compiler supports blocks, your life is a little easier.
	// You can use the "Block" versions of the functions without needing the callbacks above.
	#if USE_BLOCKS
		__block cpVect impulseSum = cpvzero;
		cpBodyEachArbiter_b(scaleStaticBody, ^(cpArbiter *arb){
			impulseSum = cpvadd(impulseSum, cpArbiterTotalImpulseWithFriction(arb));
		});
	#else
		cpVect impulseSum = cpvzero;
		cpBodyEachArbiter(scaleStaticBody, (cpBodyArbiterIteratorFunc)ScaleIterator, &impulseSum);
	#endif
	
	// Force is the impulse divided by the timestep.
	cpFloat force = cpvlength(impulseSum)/dt;
		
	// Weight can be found similarly from the gravity vector.
	cpVect g = cpSpaceGetGravity(space);
	cpFloat weight = cpvdot(g, impulseSum)/(cpvlengthsq(g)*dt);
	
	ChipmunkDemoPrintString("Total force: %5.2f, Total weight: %5.2f. ", force, weight);
	
	
	// Highlight and count the number of shapes the ball is touching.
	#if USE_BLOCKS
		__block int count = 0;
		cpBodyEachArbiter_b(ballBody, ^(cpArbiter *arb){
			// body is the body we are iterating the arbiters for.
			// CP_ARBITER_GET_*() in an arbiter iterator always returns the body/shape for the iterated body first.
			CP_ARBITER_GET_SHAPES(arb, ball, other);
			ChipmunkDebugDrawBB(cpShapeGetBB(other), RGBAColor(1, 0, 0, 1));
			
			count++;
		});
	#else
		int count = 0;
		cpBodyEachArbiter(ballBody, (cpBodyArbiterIteratorFunc)BallIterator, &count);
	#endif
	
	ChipmunkDemoPrintString("The ball is touching %d shapes.\n", count);
	
	#if USE_BLOCKS
		__block cpFloat magnitudeSum = 0.0f;
		__block cpVect vectorSum = cpvzero;
		cpBodyEachArbiter_b(ballBody, ^(cpArbiter *arb){
			cpVect j = cpArbiterTotalImpulseWithFriction(arb);
			magnitudeSum += cpvlength(j);
			vectorSum = cpvadd(vectorSum, j);
		});
		
		cpFloat crushForce = (magnitudeSum - cpvlength(vectorSum))*dt;
	#else
		struct CrushingContext crush = {0.0f, cpvzero};
		cpBodyEachArbiter(ballBody, (cpBodyArbiterIteratorFunc)EstimateCrushing, &crush);
		
		cpFloat crushForce = (crush.magnitudeSum - cpvlength(crush.vectorSum))*dt;
	#endif
	
	
	if(crushForce > 10.0f){
		ChipmunkDemoPrintString("The ball is being crushed. (f: %.2f)", crushForce);
	} else {
		ChipmunkDemoPrintString("The ball is not being crushed. (f: %.2f)", crushForce);
	}
}

#define WIDTH 4.0f
#define HEIGHT 30.0f

static cpSpace *
init(void)
{
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -300));
	cpSpaceSetCollisionSlop(space, 0.5);
	
	// For cpBodyEachArbiter() to work you must explicitly enable the contact graph or enable sleeping.
	// Generating the contact graph is a small but measurable ~5-10% performance hit so it's not enabled by default.
//	cpSpaceSetEnableContactGraph(space, cpTrue);
	cpSpaceSetSleepTimeThreshold(space, 1.0f);
	
	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(-320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(320,-240), cpv(320,240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	scaleStaticBody = cpBodyNewStatic();
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(scaleStaticBody, cpv(-240,-180), cpv(-140,-180), 4.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);
	
	// add some boxes to stack on the scale
	for(int i=0; i<5; i++){
		body = cpSpaceAddBody(space, cpBodyNew(1.0f, cpMomentForBox(1.0f, 30.0f, 30.0f)));
		cpBodySetPos(body, cpv(0, i*32 - 220));
		
		shape = cpSpaceAddShape(space, cpBoxShapeNew(body, 30.0f, 30.0f));
		cpShapeSetElasticity(shape, 0.0f);
		cpShapeSetFriction(shape, 0.8f);
	}
	
	// Add a ball that we'll track which objects are beneath it.
	cpFloat radius = 15.0f;
	ballBody = cpSpaceAddBody(space, cpBodyNew(10.0f, cpMomentForCircle(10.0f, 0.0f, radius, cpvzero)));
	cpBodySetPos(ballBody, cpv(120, -240 + radius+5));

	shape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
	cpShapeSetElasticity(shape, 0.0f);
	cpShapeSetFriction(shape, 0.9f);
	
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
	
	cpBodyFree(scaleStaticBody);
}

ChipmunkDemo ContactGraph = {
	"Contact Graph",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
