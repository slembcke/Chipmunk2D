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
#include <stddef.h>
#include <string.h>

#include "sokol/sokol_gfx.h"

#include "chipmunk/chipmunk_private.h"
#include "ChipmunkDebugDraw.h"

cpTransform ChipmunkDebugDrawVPMatrix;
float ChipmunkDebugDrawPointLineScale = 1.0f;
float ChipmunkDebugDrawOutlineWidth = 1.0f;

#define GLSL33(x) "#version 330\n" #x

static sg_pass_action pass_action;
static sg_bindings bindings;
static sg_pipeline pipeline;

typedef struct {float x, y;} float2;
typedef struct {uint8_t r, g, b, a;} RGBA8;
typedef struct {float radius; float2 position; float2 uv; RGBA8 color;} Vertex;
typedef uint16_t Index;

static RGBA8 cp_to_rgba(cpSpaceDebugColor c){return (RGBA8){0xFF*c.r, 0xFF*c.g, 0xFF*c.b, 0xFF*c.a};}

typedef struct {
		float U_vp_matrix[16];
} Uniforms;


// Meh, just max out 16 bit index size.
#define VERTEX_MAX (64*1024)
#define INDEX_MAX (128*1024)

static sg_buffer VertexBuffer, IndexBuffer;
static size_t VertexCount, IndexCount;

static Vertex Vertexes[VERTEX_MAX];
static uint16_t Indexes[INDEX_MAX];
	
void
ChipmunkDebugDrawInit(void)
{
	sg_desc desc = {};
	sg_setup(&desc);
	cpAssertHard(sg_isvalid(), "Could not init Sokol GFX.");
	
	pass_action = (sg_pass_action){
		.colors = {
			[0] = {.action = SG_ACTION_CLEAR, .val = {0x00/255.0, 0x2B/255.0, 0x36/255.0, 0.0}},
		},
	};
	
	VertexBuffer = sg_make_buffer(&(sg_buffer_desc){
		.label = "ChipmunkDebugDraw Vertex Buffer",
		.size = VERTEX_MAX*sizeof(Vertex),
		.type = SG_BUFFERTYPE_VERTEXBUFFER,
		.usage = SG_USAGE_STREAM,
	});
	
	IndexBuffer = sg_make_buffer(&(sg_buffer_desc){
		.label = "ChipmunkDebugDraw Index Buffer",
		.size = INDEX_MAX*sizeof(Index),
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.usage = SG_USAGE_STREAM,
	});
	
	bindings = (sg_bindings){
		.vertex_buffers[0] = VertexBuffer,
		.index_buffer = IndexBuffer,
	};
	
	sg_shader shd = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
				.size = sizeof(Uniforms),
				.uniforms = {
						[0] = {.name = "U_vp_matrix", .type = SG_UNIFORMTYPE_MAT4},
				}
		},
		.vs.source = GLSL33(
			layout(location = 0) in float IN_radius;
			layout(location = 1) in vec2 IN_position;
			layout(location = 2) in vec2 IN_uv;
			layout(location = 3) in vec4 IN_color;
			
			uniform mat4 U_vp_matrix;
			
			out struct {
				vec2 uv;
				vec4 color;
			} FRAG;
			
			void main(){
				gl_Position = U_vp_matrix*vec4(IN_position + IN_radius*IN_uv, 0, 1);
				FRAG.uv = IN_uv;
				FRAG.color = IN_color;
			}
		),
		.fs.source = GLSL33(
			in struct {
				vec2 uv;
				vec4 color;
			} FRAG;
			
			out vec4 OUT_color;
			
			void main(){
				float len = length(FRAG.uv);
				float fw = length(fwidth(FRAG.uv));
				float mask = smoothstep(1, 1 - fw, len);
				OUT_color = FRAG.color*mask;
			}
		),
	});
	
	pipeline = sg_make_pipeline(&(sg_pipeline_desc){
		.shader = shd,
		.blend = {
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_ONE,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
		},
		.index_type = SG_INDEXTYPE_UINT16,
		.layout = {
			.attrs = {
				[0] = {.offset = offsetof(Vertex, radius), .format = SG_VERTEXFORMAT_FLOAT},
				[1] = {.offset = offsetof(Vertex, position), .format = SG_VERTEXFORMAT_FLOAT2},
				[2] = {.offset = offsetof(Vertex, uv), .format = SG_VERTEXFORMAT_FLOAT2},
				[3] = {.offset = offsetof(Vertex, color), .format = SG_VERTEXFORMAT_UBYTE4N},
			}
		}
	});
	
	/*
	// Setup the AA shader.
	GLint vshader = CompileShader(GL_VERTEX_SHADER, GLSL(
		attribute vec2 vertex;
		attribute vec2 aa_coord;
		attribute vec4 fill_color;
		attribute vec4 outline_color;
		
		varying vec2 v_aa_coord;
		varying vec4 v_fill_color;
		varying vec4 v_outline_color;
		
		void main(void){
			// TODO: get rid of the GL 2.x matrix bit eventually?
			gl_Position = gl_ModelViewProjectionMatrix*vec4(vertex, 0.0, 1.0);
			
			v_fill_color = fill_color;
			v_outline_color = outline_color;
			v_aa_coord = aa_coord;
		}
	));
	
	GLint fshader = CompileShader(GL_FRAGMENT_SHADER, GLSL(
		uniform float u_outline_coef;
		
		varying vec2 v_aa_coord;
		varying vec4 v_fill_color;
		//const vec4 v_fill_color = vec4(0.0, 0.0, 0.0, 1.0);
		varying vec4 v_outline_color;
		
		float aa_step(float t1, float t2, float f)
		{
			//return step(t2, f);
			return smoothstep(t1, t2, f);
		}
		
		void main(void)
		{
			float l = length(v_aa_coord);
			
			// Different pixel size estimations are handy.
			//float fw = fwidth(l);
			//float fw = length(vec2(dFdx(l), dFdy(l)));
			float fw = length(fwidth(v_aa_coord));
			
			// Outline width threshold.
			float ow = 1.0 - fw;// *u_outline_coef;
			
			// Fill/outline color.
			float fo_step = aa_step(max(ow - fw, 0.0), ow, l);
			vec4 fo_color = mix(v_fill_color, v_outline_color, fo_step);
			
			// Use pre-multiplied alpha.
			float alpha = 1.0 - aa_step(1.0 - fw, 1.0, l);
			gl_FragColor = fo_color*(fo_color.a*alpha);
			//gl_FragColor = vec4(vec3(l), 1);
		}
	));
	*/
}

static Vertex *push_vertexes(size_t vcount, const Index *index_src, size_t icount){
	cpAssertHard(VertexCount + vcount <= VERTEX_MAX || IndexCount + icount <= INDEX_MAX, "Geometry buffer full.");
	
	Vertex *vertex_dst = Vertexes + VertexCount;
	size_t base = VertexCount;
	VertexCount += vcount;
	
	Index *index_dst = Indexes + IndexCount;
	for(size_t i = 0; i < icount; i++) index_dst[i] = index_src[i] + base;
	IndexCount += icount;
	
	return vertex_dst;
}

void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fillColor)
{
	float r = (float)(size*0.5f/ChipmunkDebugDrawPointLineScale);
	RGBA8 fill = cp_to_rgba(fillColor);
	Vertex *vertexes = push_vertexes(4, (Index[]){0, 1, 2, 0, 2, 3}, 6);
	vertexes[0] = (Vertex){r, {pos.x, pos.y}, {-1, -1}, fill};
	vertexes[1] = (Vertex){r, {pos.x, pos.y}, {-1,  1}, fill};
	vertexes[2] = (Vertex){r, {pos.x, pos.y}, { 1,  1}, fill};
	vertexes[3] = (Vertex){r, {pos.x, pos.y}, { 1, -1}, fill};
}

void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
	cpFloat r = radius + 1.0f/ChipmunkDebugDrawPointLineScale;
	RGBA8 fill = cp_to_rgba(fillColor), outline = cp_to_rgba(outlineColor);
	Vertex *vertexes = push_vertexes(4, (Index[]){0, 1, 2, 0, 2, 3}, 6);
	vertexes[0] = (Vertex){r, {pos.x, pos.y}, {-1, -1}, fill};
	vertexes[1] = (Vertex){r, {pos.x, pos.y}, {-1,  1}, fill};
	vertexes[2] = (Vertex){r, {pos.x, pos.y}, { 1,  1}, fill};
	vertexes[3] = (Vertex){r, {pos.x, pos.y}, { 1, -1}, fill};
	
	ChipmunkDebugDrawSegment(pos, cpvadd(pos, cpvmult(cpvforangle(angle), radius - ChipmunkDebugDrawPointLineScale*0.5f)), outlineColor);
}

void ChipmunkDebugDrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color)
{
	ChipmunkDebugDrawFatSegment(a, b, 0.0f, color, color);
}

void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
	static const Index indexes[] = {0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 6, 5, 6, 7};
	Vertex *vertexes = push_vertexes(8, indexes, 18);
	
	cpVect t = cpvnormalize(cpvsub(b, a));
	
	cpFloat half = 1.0f/ChipmunkDebugDrawPointLineScale;
	cpFloat r = radius + half;
	if(r <= half){
		r = half;
		fillColor = outlineColor;
	}
	
	RGBA8 fill = cp_to_rgba(fillColor), outline = cp_to_rgba(outlineColor);
	
	vertexes[0] = (Vertex){r, {a.x, a.y}, {-t.x + t.y, -t.x - t.y}, fill};
	vertexes[1] = (Vertex){r, {a.x, a.y}, {-t.x - t.y, +t.x - t.y}, fill};
	vertexes[2] = (Vertex){r, {a.x, a.y}, {-0.0 + t.y, -t.x + 0.0}, fill};
	vertexes[3] = (Vertex){r, {a.x, a.y}, {-0.0 - t.y, +t.x + 0.0}, fill};
	vertexes[4] = (Vertex){r, {b.x, b.y}, {+0.0 + t.y, -t.x - 0.0}, fill};
	vertexes[5] = (Vertex){r, {b.x, b.y}, {+0.0 - t.y, +t.x - 0.0}, fill};
	vertexes[6] = (Vertex){r, {b.x, b.y}, {+t.x + t.y, -t.x + t.y}, fill};
	vertexes[7] = (Vertex){r, {b.x, b.y}, {+t.x - t.y, +t.x + t.y}, fill};
}

extern cpVect ChipmunkDemoMouse;

void ChipmunkDebugDrawPolygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
	/*
	struct ExtrudeVerts {cpVect offset, n;};
	size_t bytes = sizeof(struct ExtrudeVerts)*count;
	struct ExtrudeVerts *extrude = (struct ExtrudeVerts *)alloca(bytes);
	memset(extrude, 0, bytes);
	
	for(int i=0; i<count; i++){
		cpVect v0 = verts[(i-1+count)%count];
		cpVect v1 = verts[i];
		cpVect v2 = verts[(i+1)%count];
		
		cpVect n1 = cpvnormalize(cpvrperp(cpvsub(v1, v0)));
		cpVect n2 = cpvnormalize(cpvrperp(cpvsub(v2, v1)));
		
		cpVect offset = cpvmult(cpvadd(n1, n2), 1.0/(cpvdot(n1, n2) + 1.0f));
		struct ExtrudeVerts v = {offset, n2}; extrude[i] = v;
	}
	
//	Triangle *triangles = PushTriangles(6*count);
	Triangle *triangles = PushTriangles(5*count - 2);
	Triangle *cursor = triangles;
	
	cpFloat inset = -cpfmax(0.0f, 1.0f/ChipmunkDebugDrawPointLineScale - radius);
	for(int i=0; i<count-2; i++){
		struct v2f v0 = v2f(cpvadd(verts[  0], cpvmult(extrude[  0].offset, inset)));
		struct v2f v1 = v2f(cpvadd(verts[i+1], cpvmult(extrude[i+1].offset, inset)));
		struct v2f v2 = v2f(cpvadd(verts[i+2], cpvmult(extrude[i+2].offset, inset)));
		
		Triangle t = {{v0, v2f0, fillColor, fillColor}, {v1, v2f0, fillColor, fillColor}, {v2, v2f0, fillColor, fillColor}}; *cursor++ = t;
	}
	
	cpFloat outset = 1.0f/ChipmunkDebugDrawPointLineScale + radius - inset;
	for(int i=0, j=count-1; i<count; j=i, i++){
		cpVect vA = verts[i];
		cpVect vB = verts[j];
		
		cpVect nA = extrude[i].n;
		cpVect nB = extrude[j].n;
		
		cpVect offsetA = extrude[i].offset;
		cpVect offsetB = extrude[j].offset;
		
		cpVect innerA = cpvadd(vA, cpvmult(offsetA, inset));
		cpVect innerB = cpvadd(vB, cpvmult(offsetB, inset));
		
		// Admittedly my variable naming sucks here...
		struct v2f inner0 = v2f(innerA);
		struct v2f inner1 = v2f(innerB);
		struct v2f outer0 = v2f(cpvadd(innerA, cpvmult(nB, outset)));
		struct v2f outer1 = v2f(cpvadd(innerB, cpvmult(nB, outset)));
		struct v2f outer2 = v2f(cpvadd(innerA, cpvmult(offsetA, outset)));
		struct v2f outer3 = v2f(cpvadd(innerA, cpvmult(nA, outset)));
		
		struct v2f n0 = v2f(nA);
		struct v2f n1 = v2f(nB);
		struct v2f offset0 = v2f(offsetA);
		
		Triangle t0 = {{inner0, v2f0, fillColor, outlineColor}, {inner1,    v2f0, fillColor, outlineColor}, {outer1,      n1, fillColor, outlineColor}}; *cursor++ = t0;
		Triangle t1 = {{inner0, v2f0, fillColor, outlineColor}, {outer0,      n1, fillColor, outlineColor}, {outer1,      n1, fillColor, outlineColor}}; *cursor++ = t1;
		Triangle t2 = {{inner0, v2f0, fillColor, outlineColor}, {outer0,      n1, fillColor, outlineColor}, {outer2, offset0, fillColor, outlineColor}}; *cursor++ = t2;
		Triangle t3 = {{inner0, v2f0, fillColor, outlineColor}, {outer2, offset0, fillColor, outlineColor}, {outer3,      n0, fillColor, outlineColor}}; *cursor++ = t3;
	}
	*/
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

void
ChipmunkDebugDrawFlushRenderer(int pass_width, int pass_height)
{
	cpTransform t = ChipmunkDebugDrawVPMatrix;
	Uniforms uniforms = {
		.U_vp_matrix = {t.a , t.b , 0, 0, t.c , t.d , 0, 0, 0, 0, 1, 0, t.tx, t.ty, 0, 1},
	};
	
	sg_begin_default_pass(&pass_action, pass_width, pass_height);
	
	sg_update_buffer(VertexBuffer, Vertexes, VertexCount*sizeof(Vertex));
	sg_update_buffer(IndexBuffer, Indexes, IndexCount*sizeof(Index));
	
	sg_apply_pipeline(pipeline);
	sg_apply_bindings(&bindings);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &uniforms, sizeof(Uniforms));
	sg_draw(0, IndexCount, 1);
	
	sg_end_pass();
	sg_commit();
	
	VertexCount = 0;
	IndexCount = 0;
}

void
ChipmunkDebugDrawClearRenderer(void)
{
	// triangle_count = 0;
}

// static int pushed_triangle_count = 0;
void
ChipmunkDebugDrawPushRenderer(void)
{
	// pushed_triangle_count = triangle_count;
}

void
ChipmunkDebugDrawPopRenderer(void)
{
	// triangle_count = pushed_triangle_count;
}
