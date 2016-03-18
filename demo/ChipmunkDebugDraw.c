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

#include <limits.h>
#include <string.h>

#include "Photon.h"

#include "chipmunk/chipmunk_private.h"
#include "ChipmunkDebugDraw.h"

float ChipmunkDebugDrawPointLineScale = 1.0f;
float ChipmunkDebugDrawOutlineWidth = 1.0f;

//struct v2f {float x, y;};
//static struct v2f v2f0 = {0.0f, 0.0f};
//
//static inline struct v2f
//v2f(cpVect v)
//{
//	struct v2f v2 = {(float)v.x, (float)v.y};
//	return v2;
//}
//
//typedef struct Vertex {struct v2f vertex, aa_coord; cpSpaceDebugColor fill_color, outline_color;} Vertex;
//typedef struct Triangle {Vertex a, b, c;} Triangle;

PhotonRenderState *DEBUG_RENDER_STATE = NULL;

void
ChipmunkDebugDrawInit(void)
{
	PhotonShader *debugShader = PhotonShaderNew(PHOTON_GLSL(
		in vec4 PhotonAttributePosition;
		in vec4 PhotonAttributeColor;
		
		out vec4 PhotonFragColor;
		
		void main(){
			gl_Position = PhotonAttributePosition;
			PhotonFragColor = PhotonAttributeColor;
		}
	), PHOTON_GLSL(
		in vec4 PhotonFragColor;
		
		out vec4 PhotonFragOut;
		
		void main(){
			PhotonFragOut = PhotonFragColor;
		}
	));
	
	PhotonUniforms *debugUniforms = PhotonUniformsNew(debugShader);
	DEBUG_RENDER_STATE = PhotonRenderStateNew(NULL, debugShader, debugUniforms);
	
	// Setup the AA shader.
//	GLint vshader = CompileShader(GL_VERTEX_SHADER, "#version 150\n"GLSL(
//		in vec2 vertex;
//		in vec2 aa_coord;
//		in vec4 fill_color;
//		in vec4 outline_color;
//		
//		out vec2 v_aa_coord;
//		out vec4 v_fill_color;
//		out vec4 v_outline_color;
//		
//		void main(void){
//			// TODO: get rid of the GL 2.x matrix bit eventually?
//			gl_Position = gl_ModelViewProjectionMatrix*vec4(vertex, 0.0, 1.0);
//			
//			v_fill_color = fill_color;
//			v_outline_color = outline_color;
//			v_aa_coord = aa_coord;
//		}
//	));
//	
//	GLint fshader = CompileShader(GL_FRAGMENT_SHADER, GLSL(
//		uniform float u_outline_coef;
//		
//		in vec2 v_aa_coord;
//		in vec4 v_fill_color;
//		in vec4 v_outline_color;
//		
//		float aa_step(float t1, float t2, float f)
//		{
//			//return step(t2, f);
//			return smoothstep(t1, t2, f);
//		}
//		
//		void main(void)
//		{
//			float l = length(v_aa_coord);
//			
//			// Different pixel size estimations are handy.
//			//float fw = fwidth(l);
//			//float fw = length(vec2(dFdx(l), dFdy(l)));
//			float fw = length(fwidth(v_aa_coord));
//			
//			// Outline width threshold.
//			float ow = 1.0 - fw;//*u_outline_coef;
//			
//			// Fill/outline color.
//			float fo_step = aa_step(max(ow - fw, 0.0), ow, l);
//			vec4 fo_color = mix(v_fill_color, v_outline_color, fo_step);
//			
//			// Use pre-multiplied alpha.
//			float alpha = 1.0 - aa_step(1.0 - fw, 1.0, l);
//			gl_FragColor = fo_color*(fo_color.a*alpha);
//			//gl_FragColor = vec4(vec3(l), 1);
//		}
//	));
//	
//	program = LinkProgram(vshader, fshader);
//	CHECK_GL_ERRORS();
//	
//	// Setu VBO and VAO.
//	glGenVertexArrays(1, &vao);
//	glBindVertexArray(vao);
//	
//	glGenBuffers(1, &vbo);
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
//	
//	SET_ATTRIBUTE(program, struct Vertex, vertex, GL_FLOAT);
//	SET_ATTRIBUTE(program, struct Vertex, aa_coord, GL_FLOAT);
//	SET_ATTRIBUTE(program, struct Vertex, fill_color, GL_FLOAT);
//	SET_ATTRIBUTE(program, struct Vertex, outline_color, GL_FLOAT);
//	
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//
//	CHECK_GL_ERRORS();
}

static PhotonRenderer *CHIPMUNK_DEBUG_DRAW_RENDERER = NULL;
static cpTransform VP_TRANSFORM = {};

void ChipmunkDebugDrawSetRenderer(PhotonRenderer *renderer, cpTransform vpTransform){
	CHIPMUNK_DEBUG_DRAW_RENDERER = renderer;
	VP_TRANSFORM = vpTransform;
}

#undef MAX // Defined on some systems
#define MAX(__a__, __b__) (__a__ > __b__ ? __a__ : __b__)

void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
	assert(CHIPMUNK_DEBUG_DRAW_RENDERER);
	
	cpVect p = cpTransformPoint(VP_TRANSFORM, pos);
	cpVect r = cpTransformVect(VP_TRANSFORM, cpv(radius, radius));
	pVec4 color = *(pVec4 *)&fillColor;
	
	PhotonRenderBuffers buffers = PhotonRendererEnqueueTriangles(CHIPMUNK_DEBUG_DRAW_RENDERER, 2, 4, DEBUG_RENDER_STATE);
	PhotonRenderBuffersCopyIndexes(&buffers, (PhotonIndex[6]){0, 1, 2, 0, 2, 3}, 0, 6);
	PhotonRenderBuffersCopyVertexes(&buffers, (PhotonVertex[4]){
		{{{p.x - r.x, p.y - r.y, 0, 1}}, {-1, -1}, {0, 0}, color},
		{{{p.x - r.x, p.y + r.y, 0, 1}}, {-1,  1}, {0, 0}, color},
		{{{p.x + r.x, p.y + r.y, 0, 1}}, { 1,  1}, {0, 0}, color},
		{{{p.x + r.x, p.y - r.y, 0, 1}}, { 1, -1}, {0, 0}, color},
	}, 0, 4);
	
//	ChipmunkDebugDrawSegment(pos, cpvadd(pos, cpvmult(cpvforangle(angle), radius - ChipmunkDebugDrawPointLineScale*0.5f)), outlineColor);
}

void ChipmunkDebugDrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color)
{
	ChipmunkDebugDrawFatSegment(a, b, 0.0f, color, color);
}

void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
	assert(CHIPMUNK_DEBUG_DRAW_RENDERER);
	
	cpVect n = cpvnormalize(cpvrperp(cpvsub(b, a)));
	cpVect t = cpvrperp(n);
	
	cpVect pa = cpTransformPoint(VP_TRANSFORM, a);
	cpVect pb = cpTransformPoint(VP_TRANSFORM, b);
	cpVect r = cpTransformVect(VP_TRANSFORM, cpv(radius, radius));
	
//	cpFloat half = 1.0f/ChipmunkDebugDrawPointLineScale;
//	cpFloat r = radius + half;
//	if(r <= half){
//		r = half;
//		fillColor = outlineColor;
//	}
	
	cpVect nw = {n.x*r.x, n.y*r.y};
	cpVect tw = {t.x*r.x, t.y*r.y};
	cpVect v0 = cpvsub(pb, cpvadd(nw, tw)); // { 1.0, -1.0}
	cpVect v1 = cpvadd(pb, cpvsub(nw, tw)); // { 1.0,  1.0}
	cpVect v2 = cpvsub(pb, nw); // { 0.0, -1.0}
	cpVect v3 = cpvadd(pb, nw); // { 0.0,  1.0}
	cpVect v4 = cpvsub(pa, nw); // { 0.0, -1.0}
	cpVect v5 = cpvadd(pa, nw); // { 0.0,  1.0}
	cpVect v6 = cpvsub(pa, cpvsub(nw, tw)); // {-1.0, -1.0}
	cpVect v7 = cpvadd(pa, cpvadd(nw, tw)); // {-1.0,  1.0}

	pVec4 color = *(pVec4 *)&fillColor;
	
	PhotonRenderBuffers buffers = PhotonRendererEnqueueTriangles(CHIPMUNK_DEBUG_DRAW_RENDERER, 6, 8, DEBUG_RENDER_STATE);
	PhotonRenderBuffersCopyIndexes(&buffers, (PhotonIndex[18]){0, 1, 2, 3, 1, 2, 3, 4, 2, 3, 4, 5, 6, 4, 5, 6, 7, 5}, 0, 18);
	PhotonRenderBuffersCopyVertexes(&buffers, (PhotonVertex[8]){
		{{{v0.x, v0.y, 0, 1}}, { 1, -1}, {0, 0}, color},
		{{{v1.x, v1.y, 0, 1}}, { 1,  1}, {0, 0}, color},
		{{{v2.x, v2.y, 0, 1}}, { 0, -1}, {0, 0}, color},
		{{{v3.x, v3.y, 0, 1}}, { 0,  1}, {0, 0}, color},
		{{{v4.x, v4.y, 0, 1}}, { 0, -1}, {0, 0}, color},
		{{{v5.x, v5.y, 0, 1}}, { 0,  1}, {0, 0}, color},
		{{{v6.x, v6.y, 0, 1}}, {-1, -1}, {0, 0}, color},
		{{{v7.x, v7.y, 0, 1}}, {-1,  1}, {0, 0}, color},
	}, 0, 8);
//	Triangle *triangles = PushTriangles(6);
//	
//	cpVect n = cpvnormalize(cpvrperp(cpvsub(b, a)));
//	cpVect t = cpvrperp(n);
//	
//	cpFloat half = 1.0f/ChipmunkDebugDrawPointLineScale;
//	cpFloat r = radius + half;
//	if(r <= half){
//		r = half;
//		fillColor = outlineColor;
//	}
//	
//	cpVect nw = (cpvmult(n, r));
//	cpVect tw = (cpvmult(t, r));
//	struct v2f v0 = v2f(cpvsub(b, cpvadd(nw, tw))); // { 1.0, -1.0}
//	struct v2f v1 = v2f(cpvadd(b, cpvsub(nw, tw))); // { 1.0,  1.0}
//	struct v2f v2 = v2f(cpvsub(b, nw)); // { 0.0, -1.0}
//	struct v2f v3 = v2f(cpvadd(b, nw)); // { 0.0,  1.0}
//	struct v2f v4 = v2f(cpvsub(a, nw)); // { 0.0, -1.0}
//	struct v2f v5 = v2f(cpvadd(a, nw)); // { 0.0,  1.0}
//	struct v2f v6 = v2f(cpvsub(a, cpvsub(nw, tw))); // {-1.0, -1.0}
//	struct v2f v7 = v2f(cpvadd(a, cpvadd(nw, tw))); // {-1.0,  1.0}
//	
//	Triangle t0 = {{v0, { 1.0f, -1.0f}, fillColor, outlineColor}, {v1, { 1.0f,  1.0f}, fillColor, outlineColor}, {v2, { 0.0f, -1.0f}, fillColor, outlineColor}}; triangles[0] = t0;
//	Triangle t1 = {{v3, { 0.0f,  1.0f}, fillColor, outlineColor}, {v1, { 1.0f,  1.0f}, fillColor, outlineColor}, {v2, { 0.0f, -1.0f}, fillColor, outlineColor}}; triangles[1] = t1;
//	Triangle t2 = {{v3, { 0.0f,  1.0f}, fillColor, outlineColor}, {v4, { 0.0f, -1.0f}, fillColor, outlineColor}, {v2, { 0.0f, -1.0f}, fillColor, outlineColor}}; triangles[2] = t2;
//	Triangle t3 = {{v3, { 0.0f,  1.0f}, fillColor, outlineColor}, {v4, { 0.0f, -1.0f}, fillColor, outlineColor}, {v5, { 0.0f,  1.0f}, fillColor, outlineColor}}; triangles[3] = t3;
//	Triangle t4 = {{v6, {-1.0f, -1.0f}, fillColor, outlineColor}, {v4, { 0.0f, -1.0f}, fillColor, outlineColor}, {v5, { 0.0f,  1.0f}, fillColor, outlineColor}}; triangles[4] = t4;
//	Triangle t5 = {{v6, {-1.0f, -1.0f}, fillColor, outlineColor}, {v7, {-1.0f,  1.0f}, fillColor, outlineColor}, {v5, { 0.0f,  1.0f}, fillColor, outlineColor}}; triangles[5] = t5;
}

void ChipmunkDebugDrawPolygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
//	struct ExtrudeVerts {cpVect offset, n;};
//	size_t bytes = sizeof(struct ExtrudeVerts)*count;
//	struct ExtrudeVerts *extrude = (struct ExtrudeVerts *)alloca(bytes);
//	memset(extrude, 0, bytes);
//	
//	for(int i=0; i<count; i++){
//		cpVect v0 = verts[(i-1+count)%count];
//		cpVect v1 = verts[i];
//		cpVect v2 = verts[(i+1)%count];
//		
//		cpVect n1 = cpvnormalize(cpvrperp(cpvsub(v1, v0)));
//		cpVect n2 = cpvnormalize(cpvrperp(cpvsub(v2, v1)));
//		
//		cpVect offset = cpvmult(cpvadd(n1, n2), 1.0/(cpvdot(n1, n2) + 1.0f));
//		struct ExtrudeVerts v = {offset, n2}; extrude[i] = v;
//	}
//	
////	Triangle *triangles = PushTriangles(6*count);
//	Triangle *triangles = PushTriangles(5*count - 2);
//	Triangle *cursor = triangles;
//	
//	cpFloat inset = -cpfmax(0.0f, 1.0f/ChipmunkDebugDrawPointLineScale - radius);
//	for(int i=0; i<count-2; i++){
//		struct v2f v0 = v2f(cpvadd(verts[  0], cpvmult(extrude[  0].offset, inset)));
//		struct v2f v1 = v2f(cpvadd(verts[i+1], cpvmult(extrude[i+1].offset, inset)));
//		struct v2f v2 = v2f(cpvadd(verts[i+2], cpvmult(extrude[i+2].offset, inset)));
//		
//		Triangle t = {{v0, v2f0, fillColor, fillColor}, {v1, v2f0, fillColor, fillColor}, {v2, v2f0, fillColor, fillColor}}; *cursor++ = t;
//	}
//	
//	cpFloat outset = 1.0f/ChipmunkDebugDrawPointLineScale + radius - inset;
//	for(int i=0, j=count-1; i<count; j=i, i++){
//		cpVect vA = verts[i];
//		cpVect vB = verts[j];
//		
//		cpVect nA = extrude[i].n;
//		cpVect nB = extrude[j].n;
//		
//		cpVect offsetA = extrude[i].offset;
//		cpVect offsetB = extrude[j].offset;
//		
//		cpVect innerA = cpvadd(vA, cpvmult(offsetA, inset));
//		cpVect innerB = cpvadd(vB, cpvmult(offsetB, inset));
//		
//		// Admittedly my variable naming sucks here...
//		struct v2f inner0 = v2f(innerA);
//		struct v2f inner1 = v2f(innerB);
//		struct v2f outer0 = v2f(cpvadd(innerA, cpvmult(nB, outset)));
//		struct v2f outer1 = v2f(cpvadd(innerB, cpvmult(nB, outset)));
//		struct v2f outer2 = v2f(cpvadd(innerA, cpvmult(offsetA, outset)));
//		struct v2f outer3 = v2f(cpvadd(innerA, cpvmult(nA, outset)));
//		
//		struct v2f n0 = v2f(nA);
//		struct v2f n1 = v2f(nB);
//		struct v2f offset0 = v2f(offsetA);
//		
//		Triangle t0 = {{inner0, v2f0, fillColor, outlineColor}, {inner1,    v2f0, fillColor, outlineColor}, {outer1,      n1, fillColor, outlineColor}}; *cursor++ = t0;
//		Triangle t1 = {{inner0, v2f0, fillColor, outlineColor}, {outer0,      n1, fillColor, outlineColor}, {outer1,      n1, fillColor, outlineColor}}; *cursor++ = t1;
//		Triangle t2 = {{inner0, v2f0, fillColor, outlineColor}, {outer0,      n1, fillColor, outlineColor}, {outer2, offset0, fillColor, outlineColor}}; *cursor++ = t2;
//		Triangle t3 = {{inner0, v2f0, fillColor, outlineColor}, {outer2, offset0, fillColor, outlineColor}, {outer3,      n0, fillColor, outlineColor}}; *cursor++ = t3;
//	}
}

void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fillColor)
{
	assert(CHIPMUNK_DEBUG_DRAW_RENDERER);
	
	cpVect p = cpTransformPoint(VP_TRANSFORM, pos);
	cpFloat radius = size*0.5f/ChipmunkDebugDrawPointLineScale;
	cpVect r = cpTransformVect(VP_TRANSFORM, cpv(radius, radius));
	pVec4 color = *(pVec4 *)&fillColor;
	
	PhotonRenderBuffers buffers = PhotonRendererEnqueueTriangles(CHIPMUNK_DEBUG_DRAW_RENDERER, 2, 4, DEBUG_RENDER_STATE);
	PhotonRenderBuffersCopyIndexes(&buffers, (PhotonIndex[6]){0, 1, 2, 0, 2, 3}, 0, 6);
	PhotonRenderBuffersCopyVertexes(&buffers, (PhotonVertex[4]){
		{{{p.x - r.x, p.y - r.y, 0, 1}}, {-1, -1}, {0, 0}, color},
		{{{p.x - r.x, p.y + r.y, 0, 1}}, {-1,  1}, {0, 0}, color},
		{{{p.x + r.x, p.y + r.y, 0, 1}}, { 1,  1}, {0, 0}, color},
		{{{p.x + r.x, p.y - r.y, 0, 1}}, { 1, -1}, {0, 0}, color},
	}, 0, 4);
//	Triangle *triangles = PushTriangles(2);
//	
//	float r = (float)(size*0.5f/ChipmunkDebugDrawPointLineScale);
//	Vertex a = {{(float)pos.x - r, (float)pos.y - r}, {-1.0f, -1.0f}, fillColor, fillColor};
//	Vertex b = {{(float)pos.x - r, (float)pos.y + r}, {-1.0f,  1.0f}, fillColor, fillColor};
//	Vertex c = {{(float)pos.x + r, (float)pos.y + r}, { 1.0f,  1.0f}, fillColor, fillColor};
//	Vertex d = {{(float)pos.x + r, (float)pos.y - r}, { 1.0f, -1.0f}, fillColor, fillColor};
//	
//	Triangle t0 = {a, b, c}; triangles[0] = t0;
//	Triangle t1 = {a, c, d}; triangles[1] = t1;
}

void ChipmunkDebugDrawBB(cpBB bb, cpSpaceDebugColor color)
{
	cpVect verts[] = {
		cpv(bb.r, bb.b),
		cpv(bb.r, bb.t),
		cpv(bb.l, bb.t),
		cpv(bb.l, bb.b),
	};
	ChipmunkDebugDrawPolygon(4, verts, 0.0f, color, LAColor(0, 0));
}
