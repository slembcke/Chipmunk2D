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

struct WorleyContex {
	int width, height;
	cpBB bb;
};

static inline cpVect
HashVect(uint32_t x, uint32_t y)
{
	cpFloat border = 0.21f;
	uint32_t h = (x*1640531513 ^ y*2654435789);
	
	return cpv(
		cpflerp(border, 1.0f - border, (cpFloat)(      h & 0xFFFF)/(cpFloat)0xFFFF),
		cpflerp(border, 1.0f - border, (cpFloat)((h>>16) & 0xFFFF)/(cpFloat)0xFFFF)
	);
}

static cpVect
WorleyPoint(int i, int j, struct WorleyContex *context)
{
	int width = context->width;
	int height = context->height;
	cpBB bb = context->bb;
	
//	cpVect fv = cpv(0.5, 0.5);
	cpVect fv = HashVect(i, j);
	
	return cpv(
		cpflerp(bb.l, bb.r, ((cpFloat)i + fv.x)/(cpFloat)width),
		cpflerp(bb.b, bb.t, ((cpFloat)j + fv.y)/(cpFloat)height)
	);
}

static int
ClipCell(cpShape *shape, cpVect center, int i, int j, struct WorleyContex *context, cpVect *verts, cpVect *clipped, int count)
{
	cpVect other = WorleyPoint(i, j, context);
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
ShatterCell(cpSpace *space, cpShape *shape, cpVect cell, int i, int j, struct WorleyContex *context)
{
	cpBody *body = cpShapeGetBody(shape);
	
	int count = cpPolyShapeGetNumVerts(shape);
	cpVect *ping = (cpVect *)alloca((count + 8)*sizeof(cpVect));
	cpVect *pong = (cpVect *)alloca((count + 8)*sizeof(cpVect));
	
	for(int i=0; i<count; i++){
		ping[i] = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, i));
	}
	
	count = ClipCell(shape, cell, i - 1, j - 1, context, ping, pong, count);
	count = ClipCell(shape, cell, i    , j - 1, context, pong, ping, count);
	count = ClipCell(shape, cell, i + 1, j - 1, context, ping, pong, count);
	count = ClipCell(shape, cell, i - 1, j    , context, pong, ping, count);
	count = ClipCell(shape, cell, i + 1, j    , context, ping, pong, count);
	count = ClipCell(shape, cell, i - 1, j + 1, context, pong, ping, count);
	count = ClipCell(shape, cell, i    , j + 1, context, ping, pong, count);
	count = ClipCell(shape, cell, i + 1, j + 1, context, pong, ping, count);
                                                                                                                                                                                                                                                                                 	
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
	cpSpaceRemoveShape(space, shape);
	cpSpaceRemoveBody(space, shape->body);
	
	struct WorleyContex context = {3, 3, cpShapeGetBB(shape)};
	
	for(int i=0; i<context.width; i++){
		for(int j=0; j<context.height; j++){
			cpVect cell = WorleyPoint(i, j, &context);
			if(cpShapeNearestPointQuery(shape, cell, NULL) < 0.0f){
				ShatterCell(space, shape, cell, i, j, &context);
			}
		}
	}
	
	cpBodyFree(shape->body);
	cpShapeFree(shape);
}

static void
update(cpSpace *space)
{
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
	
	if(ChipmunkDemoRightDown){
		cpNearestPointQueryInfo info;
		if(cpSpaceNearestPointQueryNearest(space, ChipmunkDemoMouse, 0, GRABABLE_MASK_BIT, CP_NO_GROUP, &info)){
			ShatterShape(space, info.shape);
		}
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
	shape = cpSpaceAddShape(space, cpSegmentShapeNew(staticBody, cpv(-1000, -240), cpv( 1000, -240), 0.0f));
	cpShapeSetElasticity(shape, 1.0f);
	cpShapeSetFriction(shape, 1.0f);
	cpShapeSetLayers(shape, NOT_GRABABLE_MASK);

	cpFloat width = 300.0f;
	cpFloat height = 300.0f;
	cpFloat mass = width*height*DENSITY;
	cpFloat moment = cpMomentForBox(mass, width, height);
	
	body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
	
	shape = cpSpaceAddShape(space, cpBoxShapeNew(body, width, height));
	cpShapeSetFriction(shape, 0.6f);
		
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
