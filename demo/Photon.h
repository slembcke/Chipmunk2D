#ifndef PHOTON_H
#define PHOTON_H

/* Copyright (c) 2015 Scott Lembcke and Howling Moon Software
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

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <string.h>

#include "PhotonMath.h"


#ifdef __cplusplus
extern "C" {
#endif


//MARK: Utils

#define PHOTON_GLSL(x) "#version 330\n"#x

#if DEBUG
	void _PhotonPrintErrors(const char *file, int line);
	#define PhotonPrintErrors() _PhotonPrintErrors(__FILE__, __LINE__)
#else
	#define PhotonPrintErrors()
#endif


//MARK: Vertexes

enum {
	PhotonAttributePosition,
	PhotonAttributeUV1,
	PhotonAttributeUV2,
	PhotonAttributeColor,
	PhotonAttributeMax,
};

typedef struct PhotonVertex {
	pvec4 position;
	pvec2 uv1, uv2;
	pvec4 color;
} PhotonVertex;


//MARK: BlendModes

typedef enum PhotonBlendFactor {
	PhotonBlendFactorZero,
	PhotonBlendFactorOne,
	PhotonBlendFactorSrcColor,
	PhotonBlendFactorOneMinusSrcColor,
	PhotonBlendFactorDstColor,
	PhotonBlendFactorOneMinusDstColor,
	PhotonBlendFactorSrcAlpha,
	PhotonBlendFactorOneMinusSrcAlpha,
	PhotonBlendFactorDstAlpha,
	PhotonBlendFactorOneMinusDstAlpha,
} PhotonBlendFactor;

typedef enum PhotonBlendOp {
	PhotonBlendOpAdd,
	PhotonBlendOpSubtract,
	PhotonBlendOpReverseSubtract,
	PhotonBlendOpMin,
	PhotonBlendOpMax,
}PhotonBlendOp;

typedef struct PhotonBlendMode {
	PhotonBlendOp colorOp;
	PhotonBlendFactor colorSrcFactor, colorDstFactor;
	
	PhotonBlendOp alphaOp;
	PhotonBlendFactor alphaSrcFactor, alphaDstFactor;
} PhotonBlendMode;

extern PhotonBlendMode PhotonBlendModePremultipliedAlpha;
extern PhotonBlendMode PhotonBlendModeAdd;
extern PhotonBlendMode PhotonBlendModeSubtract;
extern PhotonBlendMode PhotonBlendModeMultiply;


//MARK: Textures

typedef enum PhotonTextureFormat {
	PhotonTextureFormatR8,
	PhotonTextureFormatRG8,
	PhotonTextureFormatRGBA8,
	PhotonTextureFormatSRGBA8,
	PhotonTextureFormatRGBA16F,
} PhotonTextureFormat;

typedef enum PhotonTextureFilter {
	PhotonTextureFilterMipmapNone,
	PhotonTextureFilterNearest,
	PhotonTextureFilterLinear,
} PhotonTextureFilter;

typedef enum PhotonTextureAddressMode {
	PhotonTextureAddressModeClampToEdge,
	PhotonTextureAddressModeClampToBorder,
	PhotonTextureAddressModeRepeat,
	PhotonTextureAddressModeMirroredRepeat,
} PhotonTextureAddressMode;

typedef struct PhotonTextureOptions {
	PhotonTextureFormat format;
	bool generateMipmaps;
	
	PhotonTextureFilter minFilter, magFilter, mipFilter;
	PhotonTextureAddressMode addressX, addressY;
} PhotonTextureOptions;

extern const PhotonTextureOptions PhotonTextureOptionsDefault;
extern const PhotonTextureOptions PhotonTextureOptionsTrilinearRepeat;

bool PhotonTextureOptionsEqual(const PhotonTextureOptions *a, const PhotonTextureOptions *b);

typedef struct PhotonTexture PhotonTexture;
PhotonTexture *PhotonTextureNew(int width, int height, const void *pixelData, const PhotonTextureOptions *options);
void PhotonTextureFree(PhotonTexture *texture);

void PhotonTextureGetInfo(PhotonTexture *texture, int *width, int *height, const PhotonTextureOptions **options);

void PhotonTextureSubImage(PhotonTexture *texture, int x, int y, int w, int h, size_t stride, const void *pixelData);


//MARK: RenderTexture

typedef struct PhotonRenderTexture PhotonRenderTexture;
PhotonRenderTexture *PhotonRenderTextureNew(PhotonTexture *texture);
void PhotonRenderTextureFree(PhotonRenderTexture *renderTexture);

PhotonTexture *PhotonRenderTextureGetTexture(PhotonRenderTexture *renderTexture);


//MARK: Shaders

typedef struct PhotonShader PhotonShader;
PhotonShader *PhotonShaderNew(const char *vshader, const char *fshader);
void PhotonShaderFree(PhotonShader *shader);


//MARK: Uniforms

typedef struct PhotonUniforms PhotonUniforms;
PhotonUniforms *PhotonUniformsNew(const PhotonShader *shader);
void PhotonUniformsFree(PhotonUniforms *uniforms);

// Set the PhotonLocals uniform block value.
void PhotonUniformsSetLocals(PhotonUniforms *uniforms, const void *ptr);

// Set a texture binding.
void PhotonUniformsSetTexture(PhotonUniforms *uniforms, const char *name, const PhotonTexture *texture);


//MARK: RenderStates

typedef struct PhotonRenderState PhotonRenderState;

PhotonRenderState *PhotonRenderStateNew(const PhotonBlendMode *blendMode, const PhotonUniforms *uniforms);
void PhotonRenderStateFree(PhotonRenderState *state);


//MARK: RenderBuffers

typedef uint16_t PhotonIndex;

typedef struct PhotonRenderBuffers {
	volatile PhotonVertex *vertexes;
	volatile PhotonIndex *indexes;
	PhotonIndex batchOffset;
} PhotonRenderBuffers;

static inline volatile PhotonVertex *
PhotonVertexPush(volatile PhotonVertex *v, const pvec4 p, const pvec2 uv1, const pvec2 uv2, const pvec4 c)
{
	v->position.x = p.x; v->position.y = p.y; v->position.z = p.z, v->position.w = p.w;
	v->uv1.x = uv1.x; v->uv1.y = uv1.y;
	v->uv2.x = uv2.x; v->uv2.y = uv2.y;
	v->color.r = c.r; v->color.g = c.g; v->color.b = c.b; v->color.a = c.a;
	
	return v + 1;
}

static inline volatile PhotonVertex *
PhotonVertexCopy(volatile PhotonVertex *dst, const PhotonVertex src)
{
	return PhotonVertexPush(dst, src.position, src.uv1, src.uv2, src.color);
}

static inline volatile PhotonVertex *
PhotonVertexesCopy(volatile PhotonVertex *buffer, const PhotonVertex *vertexes, const size_t first, const size_t count)
{
	// Cast to non-volatile is undefined behavior, but very unlikely to be a perf problem on modern WC memory?
	memcpy((void *)buffer, vertexes + first, count*sizeof(PhotonVertex));
	return buffer + count;
}

static inline void
PhotonRenderBuffersCopyVertexes(const PhotonRenderBuffers *buffers, const PhotonVertex *vertexes, const size_t first, const size_t count)
{
	PhotonVertexesCopy(buffers->vertexes, vertexes, first, count);
}

static inline volatile PhotonIndex *
PhotonIndexesCopy(volatile PhotonIndex *buffer, const PhotonIndex *indexes, const size_t first, const size_t count, const PhotonIndex offset)
{
	for(size_t i = 0; i < count; i++) buffer[i] = indexes[i + first] + offset;
	return buffer + count;
}

static inline void
PhotonRenderBuffersCopyIndexes(const PhotonRenderBuffers *buffers, const PhotonIndex *indexes, const size_t first, const size_t count)
{
	PhotonIndexesCopy(buffers->indexes, indexes, first, count, buffers->batchOffset);
}


//MARK: Renderer

typedef struct PhotonRenderer PhotonRenderer;

PhotonRenderer *PhotonRendererNew(void);
void PhotonRendererFree(PhotonRenderer *renderer);

// Get a temporary buffer that will be freed the next time PhotonRendererPrepare is called.
void *PhotonRendererTemporaryMemory(PhotonRenderer *renderer, size_t size);

// Get temporary rendering objects.
PhotonUniforms *PhotonRendererTemporaryUniforms(PhotonRenderer *renderer, const PhotonShader *shader);
PhotonRenderState *PhotonRendererTemporaryRenderState(PhotonRenderer *renderer, const PhotonBlendMode *blendMode, const PhotonUniforms *uniforms);

// Reset the current batch. (Usually for debugging)
void PhotonRendererResetBatch(PhotonRenderer *renderer);

// Prepare a renderer to queue new rendering commands.
// Must pass the size of the default framebuffer 
void PhotonRendererPrepare(PhotonRenderer *renderer, pvec2 defaultFramebufferSize);

// Set the PhotonGlobals uniform block value.
void PhotonRendererSetGlobals(PhotonRenderer *renderer, void *ptr, size_t size);

// Execute the rendering commands in the queue.
void PhotonRendererFlush(PhotonRenderer *renderer);

// Check if a renderer's resources are ready.
bool PhotonRendererIsReady(PhotonRenderer *renderer);

// Wait until a renderer is ready again.
bool PhotonRendererWait(PhotonRenderer *renderer, double seconds);


//MARK: Render Commands

// Enqueue a rendering command that draws triangles with the given rendering state.
PhotonRenderBuffers PhotonRendererEnqueueTriangles(PhotonRenderer *renderer, size_t triangles, size_t vertexes, const PhotonRenderState *state);

// Pop triangles (and vertexes) from the most recent PhotonRendererEnqueueTriangles() call.
void PhotonRendererPopTriangles(PhotonRenderer *renderer, size_t triangles, size_t vertexes);

// Custom rendering commands.
typedef void (*PhotonCustomCommandFunc)(void *context);
void PhotonRendererEnqueueCustomCommand(PhotonRenderer *renderer, PhotonRenderState *state, PhotonCustomCommandFunc func, void *context);

// Bind a render texture.
typedef enum PhotonLoadAction {
	PhotonLoadActionLoad,
	PhotonLoadActionClear,
	PhotonLoadActionDontCare,
} PhotonLoadAction;

typedef enum PhotonStoreAction {
	PhotonStoreActionStore,
	PhotonStoreActionDontCare,
} PhotonStoreAction;

void PhotonRendererBindRenderTexture(
	PhotonRenderer *renderer, PhotonRenderTexture *renderTexture,
	PhotonLoadAction loadAction, PhotonStoreAction storeAction, pvec4 clearColor
);

// Scissoring
typedef enum {
	PhotonScissorStateDisable,
//	PhotonScissorStateNormalized,
	PhotonScissorStateClipSpace,
//	PhotonScissorStatePixel,
} PhotonScissorState; 

void PhotonRendererScissor(PhotonRenderer *renderer, PhotonScissorState enable, paabb2 rect);


#ifdef __cplusplus
}
#endif

#endif
