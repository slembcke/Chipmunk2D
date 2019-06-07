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

#define GLSL33(x) "#version 330\n" #x

static sg_bindings bindings;
static sg_pipeline pipeline;

typedef struct {float x, y;} float2;
typedef struct {uint8_t r, g, b, a;} RGBA8;
typedef struct {float2 pos; float2 uv; float r; RGBA8 fill, outline;} Vertex;
typedef uint16_t Index;

static RGBA8 cp_to_rgba(cpSpaceDebugColor c){return (RGBA8){(uint8_t)(0xFF*c.r), (uint8_t)(0xFF*c.g), (uint8_t)(0xFF*c.b), (uint8_t)(0xFF*c.a)};}

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
			.uniforms[0] = {.name = "U_vp_matrix", .type = SG_UNIFORMTYPE_MAT4},
		},
		.vs.source = GLSL33(
			layout(location = 0) in vec2 IN_pos;
			layout(location = 1) in vec2 IN_uv;
			layout(location = 2) in float IN_radius;
			layout(location = 3) in vec4 IN_fill;
			layout(location = 4) in vec4 IN_outline;
			
			uniform mat4 U_vp_matrix;
			
			out struct {
				vec2 uv;
				vec4 fill;
				vec4 outline;
			} FRAG;
			
			void main(){
				gl_Position = U_vp_matrix*vec4(IN_pos + IN_radius*IN_uv, 0, 1);
				FRAG.uv = IN_uv;
				FRAG.fill = IN_fill;
				FRAG.fill.rgb *= IN_fill.a;
				FRAG.outline = IN_outline;
				FRAG.outline.a *= IN_outline.a;
			}
		),
		.fs.source = GLSL33(
			in struct {
				vec2 uv;
				vec4 fill;
				vec4 outline;
			} FRAG;
			
			out vec4 OUT_color;
			
			void main(){
				float len = length(FRAG.uv);
				float fw = length(fwidth(FRAG.uv));
				float mask = smoothstep(-1, fw - 1, -len);
				
				float outline = 1 - fw;
				float outline_mask = smoothstep(outline - fw, outline, len);
				vec4 color = FRAG.fill + (FRAG.outline - FRAG.fill*FRAG.outline.a)*outline_mask;
				OUT_color = color*mask;
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
				[0] = {.offset = offsetof(Vertex, pos), .format = SG_VERTEXFORMAT_FLOAT2},
				[1] = {.offset = offsetof(Vertex, uv), .format = SG_VERTEXFORMAT_FLOAT2},
				[2] = {.offset = offsetof(Vertex, r), .format = SG_VERTEXFORMAT_FLOAT},
				[3] = {.offset = offsetof(Vertex, fill), .format = SG_VERTEXFORMAT_UBYTE4N},
				[4] = {.offset = offsetof(Vertex, outline), .format = SG_VERTEXFORMAT_UBYTE4N},
			}
		}
	});
}

static Vertex *push_vertexes(size_t vcount, const Index *index_src, size_t icount){
	cpAssertHard(VertexCount + vcount <= VERTEX_MAX || IndexCount + icount <= INDEX_MAX, "Geometry buffer full.");
	
	Vertex *vertex_dst = Vertexes + VertexCount;
	size_t base = VertexCount;
	VertexCount += vcount;
	
	Index *index_dst = Indexes + IndexCount;
	for(size_t i = 0; i < icount; i++) index_dst[i] = index_src[i] + (Index)base;
	IndexCount += icount;
	
	return vertex_dst;
}

void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fillColor)
{
	float r = (float)(size*0.5f*ChipmunkDebugDrawPointLineScale);
	RGBA8 fill = cp_to_rgba(fillColor);
	Vertex *vertexes = push_vertexes(4, (Index[]){0, 1, 2, 0, 2, 3}, 6);
	vertexes[0] = (Vertex){{(float)pos.x, (float)pos.y}, {-1, -1}, r, fill, fill};
	vertexes[1] = (Vertex){{(float)pos.x, (float)pos.y}, {-1,  1}, r, fill, fill};
	vertexes[2] = (Vertex){{(float)pos.x, (float)pos.y}, { 1,  1}, r, fill, fill};
	vertexes[3] = (Vertex){{(float)pos.x, (float)pos.y}, { 1, -1}, r, fill, fill};
}

void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
	float r = (float)radius + ChipmunkDebugDrawPointLineScale;
	RGBA8 fill = cp_to_rgba(fillColor), outline = cp_to_rgba(outlineColor);
	Vertex *vertexes = push_vertexes(4, (Index[]){0, 1, 2, 0, 2, 3}, 6);
	vertexes[0] = (Vertex){{(float)pos.x, (float)pos.y}, {-1, -1}, r, fill, outline};
	vertexes[1] = (Vertex){{(float)pos.x, (float)pos.y}, {-1,  1}, r, fill, outline};
	vertexes[2] = (Vertex){{(float)pos.x, (float)pos.y}, { 1,  1}, r, fill, outline};
	vertexes[3] = (Vertex){{(float)pos.x, (float)pos.y}, { 1, -1}, r, fill, outline};
	
	ChipmunkDebugDrawSegment(pos, cpvadd(pos, cpvmult(cpvforangle(angle), 0.75f*radius)), outlineColor);
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
	
	float r = (float)radius + ChipmunkDebugDrawPointLineScale;
	RGBA8 fill = cp_to_rgba(fillColor), outline = cp_to_rgba(outlineColor);
	
	vertexes[0] = (Vertex){{(float)a.x, (float)a.y}, {(float)(-t.x + t.y), (float)(-t.x - t.y)}, r, fill, outline};
	vertexes[1] = (Vertex){{(float)a.x, (float)a.y}, {(float)(-t.x - t.y), (float)(+t.x - t.y)}, r, fill, outline};
	vertexes[2] = (Vertex){{(float)a.x, (float)a.y}, {(float)(-0.0 + t.y), (float)(-t.x + 0.0)}, r, fill, outline};
	vertexes[3] = (Vertex){{(float)a.x, (float)a.y}, {(float)(-0.0 - t.y), (float)(+t.x + 0.0)}, r, fill, outline};
	vertexes[4] = (Vertex){{(float)b.x, (float)b.y}, {(float)(+0.0 + t.y), (float)(-t.x - 0.0)}, r, fill, outline};
	vertexes[5] = (Vertex){{(float)b.x, (float)b.y}, {(float)(+0.0 - t.y), (float)(+t.x - 0.0)}, r, fill, outline};
	vertexes[6] = (Vertex){{(float)b.x, (float)b.y}, {(float)(+t.x + t.y), (float)(-t.x + t.y)}, r, fill, outline};
	vertexes[7] = (Vertex){{(float)b.x, (float)b.y}, {(float)(+t.x - t.y), (float)(+t.x + t.y)}, r, fill, outline};
}

#define MAX_POLY_VERTEXES 64
// Fill needs (count - 2) triangles.
// Outline needs 4*count triangles.
#define MAX_POLY_INDEXES (3*(5*MAX_POLY_VERTEXES - 2))

void ChipmunkDebugDrawPolygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor)
{
	RGBA8 fill = cp_to_rgba(fillColor), outline = cp_to_rgba(outlineColor);
	
	Index indexes[MAX_POLY_INDEXES];
	
	// Polygon fill triangles.
	for(int i = 0; i < count - 2; i++){
		indexes[3*i + 0] = 0;
		indexes[3*i + 1] = 4*(i + 1);
		indexes[3*i + 2] = 4*(i + 2);
	}
	
	// Polygon outline triangles.
	Index *cursor = indexes + 3*(count - 2);
	for(int i0 = 0; i0 < count; i0++){
		int i1 = (i0 + 1)%count;
		cursor[12*i0 +  0] = 4*i0 + 0;
		cursor[12*i0 +  1] = 4*i0 + 1;
		cursor[12*i0 +  2] = 4*i0 + 2;
		cursor[12*i0 +  3] = 4*i0 + 0;
		cursor[12*i0 +  4] = 4*i0 + 2;
		cursor[12*i0 +  5] = 4*i0 + 3;
		cursor[12*i0 +  6] = 4*i0 + 0;
		cursor[12*i0 +  7] = 4*i0 + 3;
		cursor[12*i0 +  8] = 4*i1 + 0;
		cursor[12*i0 +  9] = 4*i0 + 3;
		cursor[12*i0 + 10] = 4*i1 + 0;
		cursor[12*i0 + 11] = 4*i1 + 1;
	}
	
	float inset = (float)-cpfmax(0, 2*ChipmunkDebugDrawPointLineScale - radius);
	float outset = (float)radius + ChipmunkDebugDrawPointLineScale;
	float r = outset - inset;
	
	Vertex *vertexes = push_vertexes(4*count, indexes, 3*(5*count - 2));
	for(int i=0; i<count; i++){
		cpVect v0 = verts[i];
		cpVect v_prev = verts[(i+(count - 1))%count];
		cpVect v_next = verts[(i+(count + 1))%count];
		
		cpVect n1 = cpvnormalize(cpvrperp(cpvsub(v0, v_prev)));
		cpVect n2 = cpvnormalize(cpvrperp(cpvsub(v_next, v0)));
		cpVect of = cpvmult(cpvadd(n1, n2), 1.0/(cpvdot(n1, n2) + 1.0f));
		cpVect v = cpvadd(v0, cpvmult(of, inset));
		
		vertexes[4*i + 0] = (Vertex){{(float)v.x, (float)v.y}, {0.0f, 0.0f}, 0.0f, fill, outline};
		vertexes[4*i + 1] = (Vertex){{(float)v.x, (float)v.y}, {(float)n1.x, (float)n1.y}, r, fill, outline};
		vertexes[4*i + 2] = (Vertex){{(float)v.x, (float)v.y}, {(float)of.x, (float)of.y}, r, fill, outline};
		vertexes[4*i + 3] = (Vertex){{(float)v.x, (float)v.y}, {(float)n2.x, (float)n2.y}, r, fill, outline};
	}
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
ChipmunkDebugDrawFlushRenderer(void)
{
	cpTransform t = ChipmunkDebugDrawVPMatrix;
	Uniforms uniforms = {
		.U_vp_matrix = {(float)t.a , (float)t.b , 0.0f, 0.0f, (float)t.c , (float)t.d , 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (float)t.tx, (float)t.ty, 0.0f, 1.0f},
	};
	
	sg_update_buffer(VertexBuffer, Vertexes, VertexCount*sizeof(Vertex));
	sg_update_buffer(IndexBuffer, Indexes, IndexCount*sizeof(Index));
	
	sg_apply_pipeline(pipeline);
	sg_apply_bindings(&bindings);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &uniforms, sizeof(Uniforms));
	sg_draw(0, IndexCount, 1);
}

void
ChipmunkDebugDrawClearRenderer(void)
{
	VertexCount = IndexCount = 0;
}


static size_t PushedVertexCount, PushedIndexCount;

void
ChipmunkDebugDrawPushRenderer(void)
{
	PushedVertexCount = VertexCount;
	PushedIndexCount = IndexCount;
}

void
ChipmunkDebugDrawPopRenderer(void)
{
	VertexCount = PushedVertexCount;
	IndexCount = PushedIndexCount;
}
