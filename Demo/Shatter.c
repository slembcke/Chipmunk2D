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
#include "constraints/util.h"

#include "ChipmunkDemo.h"

#define DENSITY (1.0/10000.0)

//static void
//ClipPoly(cpSpace *space, cpShape *shape, cpVect n, cpFloat dist)
//{
//	cpBody *body = cpShapeGetBody(shape);
//	
//	int count = cpPolyShapeGetNumVerts(shape);
//	int clippedCount = 0;
//	
//	cpVect *clipped = (cpVect *)alloca((count + 1)*sizeof(cpVect));
//	
//	for(int i=0, j=count-1; i<count; j=i, i++){
//		cpVect a = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, j));
//		cpFloat a_dist = cpvdot(a, n) - dist;
//		
//		if(a_dist < 0.0){
//			clipped[clippedCount] = a;
//			clippedCount++;
//		}
//		
//		cpVect b = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, i));
//		cpFloat b_dist = cpvdot(b, n) - dist;
//		
//		if(a_dist*b_dist < 0.0f){
//			cpFloat t = cpfabs(a_dist)/(cpfabs(a_dist) + cpfabs(b_dist));
//			
//			clipped[clippedCount] = cpvlerp(a, b, t);
//			clippedCount++;
//		}
//	}
//}

static cpVect
WorleyPoint(int i, int j, cpBB bb, int width, int height)
{
	return cpv(
		cpflerp(bb.l, bb.r, ((cpFloat)i + 0.5f)/(cpFloat)width),
		cpflerp(bb.b, bb.t, ((cpFloat)j + 0.5f)/(cpFloat)height)
	);
}

static int
ClipCell(cpShape *shape, cpVect center, int i, int j, cpBB bb, int width, int height, cpVect *verts, cpVect *clipped, int count)
{
	cpVect other = WorleyPoint(i, j, bb, width, height);
	if(cpShapeNearestPointQuery(shape, other, NULL) > 0.0f){
		memcpy(clipped, verts, count*sizeof(cpVect));
		return count;
	}
	
	cpVect n = cpvsub(other, center);
	cpFloat dist = cpvdot(n, cpvlerp(center, other, 0.5f));
	
	int clipped_count = 0;
	for(int j=0, i=count-1; j<count; i=j, j++){
		cpVect a = verts[i];
		cpFloat a_dist = cpvdot(a, n) - dist;
		
		if(a_dist <= 0.0){
			clipped[clipped_count] = a;
			clipped_count++;
		}
		
		cpVect b = verts[j];
		cpFloat b_dist = cpvdot(b, n) - dist;
		
		if(a_dist*b_dist < 0.0f){
			cpFloat t = cpfabs(a_dist)/(cpfabs(a_dist) + cpfabs(b_dist));
			
			clipped[clipped_count] = cpvlerp(a, b, t);
			clipped_count++;
		}
	}
	
	return clipped_count;
}

static void
ShatterCell(cpSpace *space, cpShape *shape, cpVect cell, int i, int j, cpBB bb, int width, int height)
{
	cpBody *body = cpShapeGetBody(shape);
	
	int count = cpPolyShapeGetNumVerts(shape);
	cpVect *ping = (cpVect *)alloca((count + 8)*sizeof(cpVect));
	cpVect *pong = (cpVect *)alloca((count + 8)*sizeof(cpVect));
	
	for(int i=0; i<count; i++){
		ping[i] = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, i));
	}
	
	count = ClipCell(shape, cell, i - 1, j - 1, bb, width, height, ping, pong, count);
	count = ClipCell(shape, cell, i    , j - 1, bb, width, height, pong, ping, count);
	count = ClipCell(shape, cell, i + 1, j - 1, bb, width, height, ping, pong, count);
	count = ClipCell(shape, cell, i - 1, j    , bb, width, height, pong, ping, count);
	count = ClipCell(shape, cell, i + 1, j    , bb, width, height, ping, pong, count);
	count = ClipCell(shape, cell, i - 1, j + 1, bb, width, height, pong, ping, count);
	count = ClipCell(shape, cell, i    , j + 1, bb, width, height, ping, pong, count);
	count = ClipCell(shape, cell, i + 1, j + 1, bb, width, height, pong, ping, count);
	
//	if(count != 4){
//		printf("weird %d\n", count);
//		return;
//	}
	
	cpVect centroid = cpCentroidForPoly(count, ping);
	cpFloat mass = cpAreaForPoly(count, ping)*DENSITY;
	cpFloat moment = cpMomentForPoly(mass, count, ping, cpvneg(centroid));
	
	cpBody *new_body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	cpBodySetPos(new_body, centroid);
	cpBodySetVel(new_body, cpBodyGetVelAtWorldPoint(body, centroid));
	cpBodySetAngVel(new_body, cpBodyGetAngVel(body));
	
	cpShape *new_shape = cpSpaceAddShape(space, cpPolyShapeNew(new_body, count, ping, cpvneg(centroid)));
	// Copy whatever properties you have set on the original shape that are important
	cpShapeSetFriction(new_shape, cpShapeGetFriction(shape));
}

static void
ShatterShape(cpSpace *space, cpShape *shape)
{
	cpBB bb = cpShapeGetBB(shape);
	int width = 4;
	int height = 2;
	
	for(int i=0; i<width; i++){
		for(int j=0; j<height; j++){
			cpVect cell = WorleyPoint(i, j, bb, width, height);
			if(cpShapeNearestPointQuery(shape, cell, NULL) < 0.0f){
				ShatterCell(space, shape, cell, i, j, bb, width, height);
			}
		}
	}
}

static void
update(cpSpace *space)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static cpSpace *
init(void)
{
	ChipmunkDemoMessageString = "Right click something to shatter it.";
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetIterations(space, 30);
	cpSpaceSetGravity(space, cpv(0, -500));
	cpSpaceSetSleepTimeThreshold(space, 0.5f);
	cpSpaceSetCollisionSlop(space, 0.5f);
	
	cpBody *body, *staticBody = cpSpaceGetStaticBody(space);
	cpShape *shape;
	
	// Create segments around the edge of the screen.
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-320,-240), cpv(320,-240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	cpFloat width = 200.0f;
	cpFloat height = 300.0f;
	cpFloat mass = width*height*DENSITY;
	cpFloat moment = cpMomentForBox(mass, width, height);
	
//	body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
//	
//	shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
//	cpShapeSetFriction(shape, 0.6f);
		
	body = cpBodyNew(mass, moment);
	shape = cpBoxShapeNew(body, width, height);
	cpShapeSetFriction(shape, 0.6f);
	
	cpShapeCacheBB(shape);
	ShatterShape(space, shape);
		
	return space;
}

static void
destroy(cpSpace *space)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Shatter = {
	"Shatter.",
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};
