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
 
#include "chipmunk/chipmunk_private.h"
#include "chipmunk_unsafe.h"

cpPolyShape *
cpPolyShapeAlloc(void)
{
	return (cpPolyShape *)cpcalloc(1, sizeof(cpPolyShape));
}

static cpBB
cpPolyShapeTransform(cpPolyShape *poly, cpTransform transform)
{
	cpSplittingPlane *src = poly->planes;
	cpSplittingPlane *dst = poly->tPlanes;
	
	cpFloat l = (cpFloat)INFINITY, r = -(cpFloat)INFINITY;
	cpFloat b = (cpFloat)INFINITY, t = -(cpFloat)INFINITY;
	
	for(int i=0; i<poly->count; i++){
		cpVect v = cpTransformPoint(transform, src[i].v0);
		cpVect n = cpTransformVect(transform, src[i].n);
		
		dst[i].v0 = v;
		dst[i].n = n;
		dst[i].d = transform.tx*n.x + transform.ty*n.y + src[i].d;
		
		l = cpfmin(l, v.x);
		r = cpfmax(r, v.x);
		b = cpfmin(b, v.y);
		t = cpfmax(t, v.y);
	}
	
	cpFloat radius = poly->r;
	return cpBBNew(l - radius, b - radius, r + radius, t + radius);
}

static cpBB
cpPolyShapeCacheData(cpPolyShape *poly, cpTransform transform)
{
	cpBB bb = poly->shape.bb = cpPolyShapeTransform(poly, transform);
	
	return bb;
}

static void
cpPolyShapeDestroy(cpPolyShape *poly)
{
	cpfree(poly->planes);
}

static void
cpPolyShapePointQuery(cpPolyShape *poly, cpVect p, cpPointQueryInfo *info){
	int count = poly->count;
	cpSplittingPlane *planes = poly->tPlanes;
	cpFloat r = poly->r;
	
	cpVect v0 = planes[count - 1].v0;
	cpFloat minDist = INFINITY;
	cpVect closestPoint = cpvzero;
	cpVect closestNormal = cpvzero;
	cpBool outside = cpFalse;
	
	for(int i=0; i<count; i++){
		if(cpvdot(planes[i].n, p) - planes[i].d > 0.0f) outside = cpTrue;
		
		cpVect v1 = planes[i].v0;
		cpVect closest = cpClosetPointOnSegment(p, v0, v1);
		
		cpFloat dist = cpvdist(p, closest);
		if(dist < minDist){
			minDist = dist;
			closestPoint = closest;
			closestNormal = planes[i].n;
		}
		
		v0 = v1;
	}
	
	cpFloat dist = (outside ? minDist : -minDist);
	cpVect g = cpvmult(cpvsub(p, closestPoint), 1.0f/dist);
	
	info->shape = (cpShape *)poly;
	info->point = cpvadd(closestPoint, cpvmult(g, r));
	info->distance = dist - r;
	
	// Use the normal of the closest segment if the distance is small.
	info->gradient = (minDist > MAGIC_EPSILON ? g : closestNormal);
}

static void
cpPolyShapeSegmentQuery(cpPolyShape *poly, cpVect a, cpVect b, cpFloat r2, cpSegmentQueryInfo *info)
{
	cpSplittingPlane *planes = poly->tPlanes;
	int count = poly->count;
	cpFloat r = poly->r;
	cpFloat rsum = r + r2;
	
	for(int i=0; i<count; i++){
		cpVect n = planes[i].n;
		cpFloat an = cpvdot(a, n);
		cpFloat d =  an - planes[i].d - rsum;
		if(d < 0.0f) continue;
		
		cpFloat bn = cpvdot(b, n);
		cpFloat t = d/(an - bn);
		if(t < 0.0f || 1.0f < t) continue;
		
		cpVect point = cpvlerp(a, b, t);
		cpFloat dt = cpvcross(n, point);
		cpFloat dtMin = cpvcross(n, planes[(i - 1 + count)%count].v0);
		cpFloat dtMax = cpvcross(n, planes[i].v0);
		
		if(dtMin <= dt && dt <= dtMax){
			info->shape = (cpShape *)poly;
			info->point = cpvsub(cpvlerp(a, b, t), cpvmult(n, r2));
			info->normal = n;
			info->alpha = t;
		}
	}
	
	// Also check against the beveled vertexes.
	if(rsum > 0.0f){
		for(int i=0; i<count; i++){
			cpSegmentQueryInfo circle_info = {NULL, b, cpvzero, 1.0f};
			CircleSegmentQuery(&poly->shape, planes[i].v0, r, a, b, r2, &circle_info);
			if(circle_info.alpha < info->alpha) (*info) = circle_info;
		}
	}
}

cpBool
cpPolyValidate(const cpVect *verts, const int count)
{
	for(int i=0; i<count; i++){
		cpVect a = verts[i];
		cpVect b = verts[(i+1)%count];
		cpVect c = verts[(i+2)%count];
		
		if(cpvcross(cpvsub(b, a), cpvsub(c, a)) > 0.0f){
			return cpFalse;
		}
	}
	
	return cpTrue;
}

static void
SetVerts(cpPolyShape *poly, int count, const cpVect *verts)
{
	poly->count = count;
	poly->planes = (cpSplittingPlane *)cpcalloc(2*count, sizeof(cpSplittingPlane));
	poly->tPlanes = poly->planes + count;
	
	for(int i=0; i<count; i++){
		cpVect a = verts[(i - 1 + count)%count];
		cpVect b = verts[i];
		cpVect n = cpvnormalize(cpvrperp(cpvsub(b, a)));
		
		poly->planes[i].v0 = b;
		poly->planes[i].n = n;
		poly->planes[i].d = cpvdot(n, b);
	}
}

static struct cpShapeMassInfo
cpPolyShapeMassInfo(cpFloat mass, int count, const cpVect *verts, cpFloat radius)
{
	// TODO moment is approximate due to radius.
	
	cpVect centroid = cpCentroidForPoly(count, verts);
	struct cpShapeMassInfo info = {
		mass, cpMomentForPoly(1.0f, count, verts, cpvneg(centroid), radius),
		centroid,
		cpAreaForPoly(count, verts, radius),
	};
	
	return info;
}

static const cpShapeClass polyClass = {
	CP_POLY_SHAPE,
	(cpShapeCacheDataImpl)cpPolyShapeCacheData,
	(cpShapeDestroyImpl)cpPolyShapeDestroy,
	(cpShapePointQueryImpl)cpPolyShapePointQuery,
	(cpShapeSegmentQueryImpl)cpPolyShapeSegmentQuery,
};

cpPolyShape *
cpPolyShapeInit(cpPolyShape *poly, cpBody *body, int count, const cpVect *verts, cpTransform transform, cpFloat radius)
{
	cpVect *hullVerts = (cpVect *)alloca(count*sizeof(cpVect));
	
	// Transform the verts before building the hull in case of a negative scale.
	for(int i=0; i<count; i++) hullVerts[i] = cpTransformPoint(transform, verts[i]);
	
	unsigned int hullCount = cpConvexHull(count, hullVerts, hullVerts, NULL, 0.0);
	return cpPolyShapeInitRaw(poly, body, hullCount, hullVerts, radius);
}

cpPolyShape *
cpPolyShapeInitRaw(cpPolyShape *poly, cpBody *body, int count, const cpVect *verts, cpFloat radius)
{
	cpShapeInit((cpShape *)poly, &polyClass, body, cpPolyShapeMassInfo(0.0f, count, verts, radius));
	
	SetVerts(poly, count, verts);
	poly->r = radius;

	return poly;
}



cpShape *
cpPolyShapeNew(cpBody *body, int count, const cpVect *verts, cpTransform transform, cpFloat radius)
{
	return (cpShape *)cpPolyShapeInit(cpPolyShapeAlloc(), body, count, verts, transform, radius);
}

cpPolyShape *
cpBoxShapeInit(cpPolyShape *poly, cpBody *body, cpFloat width, cpFloat height, cpFloat radius)
{
	cpFloat hw = width/2.0f;
	cpFloat hh = height/2.0f;
	
	return cpBoxShapeInit2(poly, body, cpBBNew(-hw, -hh, hw, hh), radius);
}

cpPolyShape *
cpBoxShapeInit2(cpPolyShape *poly, cpBody *body, cpBB box, cpFloat radius)
{
	cpVect verts[] = {
		cpv(box.r, box.b),
		cpv(box.r, box.t),
		cpv(box.l, box.t),
		cpv(box.l, box.b),
	};
	
	return cpPolyShapeInitRaw(poly, body, 4, verts, radius);
}

cpShape *
cpBoxShapeNew(cpBody *body, cpFloat width, cpFloat height, cpFloat radius)
{
	return (cpShape *)cpBoxShapeInit(cpPolyShapeAlloc(), body, width, height, radius);
}

cpShape *
cpBoxShapeNew2(cpBody *body, cpBB box, cpFloat radius)
{
	return (cpShape *)cpBoxShapeInit2(cpPolyShapeAlloc(), body, box, radius);
}

int
cpPolyShapeGetNumVerts(const cpShape *shape)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	return ((cpPolyShape *)shape)->count;
}

cpVect
cpPolyShapeGetVert(const cpShape *shape, int idx)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	cpAssertHard(0 <= idx && idx < cpPolyShapeGetNumVerts(shape), "Index out of range.");
	
	return ((cpPolyShape *)shape)->planes[idx].v0;
}

cpFloat
cpPolyShapeGetRadius(const cpShape *shape)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	return ((cpPolyShape *)shape)->r;
}

// Unsafe API (chipmunk_unsafe.h)

void
cpPolyShapeSetVerts(cpShape *shape, int count, cpVect *verts, cpTransform transform)
{
	cpVect *hullVerts = (cpVect *)alloca(count*sizeof(cpVect));
	
	// Transform the verts before building the hull in case of a negative scale.
	for(int i=0; i<count; i++) hullVerts[i] = cpTransformPoint(transform, verts[i]);
	
	unsigned int hullCount = cpConvexHull(count, hullVerts, hullVerts, NULL, 0.0);
	cpPolyShapeSetVertsRaw(shape, hullCount, hullVerts);
}

void
cpPolyShapeSetVertsRaw(cpShape *shape, int count, cpVect *verts)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	cpPolyShape *poly = (cpPolyShape *)shape;
	cpPolyShapeDestroy(poly);
	
	SetVerts(poly, count, verts);
	
//	cpFloat mass = shape->massInfo.m;
//	shape->massInfo = cpPolyShapeMassInfo(shape->massInfo.m, poly->count, poly->verts, poly->r);
//	if(mass > 0.0f) cpBodyAccumulateMass(shape->body);
}

void
cpPolyShapeSetRadius(cpShape *shape, cpFloat radius)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	cpPolyShape *poly = (cpPolyShape *)shape;
	poly->r = radius;

//	cpFloat mass = shape->massInfo.m;
//	shape->massInfo = cpPolyShapeMassInfo(shape->massInfo.m, poly->count, poly->verts, poly->r);
//	if(mass > 0.0f) cpBodyAccumulateMass(shape->body);
}
