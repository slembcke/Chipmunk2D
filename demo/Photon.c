#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <assert.h>

#include "Photon.h"

#if __APPLE__

#include <OpenGL/gl3.h>

#elif __linux__

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#else

#error Unsupported Platform.

#endif


//MARK: Types

struct PhotonTexture {
	PhotonTextureOptions options;
	int width, height;
	GLuint textureID;
};

struct PhotonRenderTexture {
	PhotonTexture *texture;
	GLuint framebufferID;
};

#define PHOTON_TEXTURE_BINDING_MAX 8
#define PHOTON_TEXTURE_NAME_SIZE 32
#define PHOTON_NAME_BUFFER_SIZE ((size_t)(PHOTON_TEXTURE_BINDING_MAX*PHOTON_TEXTURE_NAME_SIZE))

struct PhotonShader {
	GLuint programID;
	
	size_t localUniformBlockSize;
	const char *uniformTextureNames[PHOTON_TEXTURE_BINDING_MAX];
	char nameBuffer[PHOTON_NAME_BUFFER_SIZE];
};

struct PhotonUniforms {
	const PhotonShader *shader;
	
	const PhotonTexture *textures[PHOTON_TEXTURE_BINDING_MAX];
	uint8_t block[];
};

struct PhotonRenderState {
	const PhotonBlendMode *blendMode;
	const PhotonShader *shader;
	const PhotonUniforms *uniforms;
};

enum PhotonBufferType {
	PhotonBufferTypeVertex,
	PhotonBufferTypeIndex,
	PhotonBufferTypeUniform,
};

struct PhotonBuffer;
typedef void (*PhotonBufferDestroyFunc)(struct PhotonBuffer *buffer);
typedef void (*PhotonBufferResizeFunc)(struct PhotonBuffer *buffer, size_t newCapacity);
typedef void (*PhotonBufferPrepareFunc)(struct PhotonBuffer *buffer);
typedef void (*PhotonBufferCommitFunc)(struct PhotonBuffer *buffer);

typedef struct PhotonBuffer {
	PhotonBufferDestroyFunc destroy;
	PhotonBufferResizeFunc resize;
	PhotonBufferPrepareFunc prepare;
	PhotonBufferCommitFunc commit;
	
	size_t count;
	size_t capacity;
	size_t elementSize;
	enum PhotonBufferType type;
	
	void *ptr;
} PhotonBuffer;

typedef struct PhotonBufferGL3 {
	struct PhotonBuffer super;
	GLuint bufferID;
} PhotonBufferGL3;

static size_t NO_PAGE = -1;

typedef struct PhotonBufferSet {
	GLuint vertexArrayID;
	
	PhotonBufferGL3 *vertexes;
	PhotonBufferGL3 *indexes;
	PhotonBufferGL3 *uniforms;
} PhotonBufferSet;

typedef struct PhotonUniformBindings {
	GLuint bufferID;
	size_t localUniformBlockOffset, localUniformBlockSize;
	GLuint textureIDs[PHOTON_TEXTURE_BINDING_MAX];
} PhotonUniformBindings;

typedef struct PhotonScratchBlock {
	struct PhotonScratchBlock *next;
	size_t count, capacity;
	uint8_t block[];
} PhotonScratchBlock;

struct PhotonCommand;
typedef const PhotonRenderState *(*PhotonCommandFunc)(const struct PhotonCommand *command, PhotonRenderer *renderer, const PhotonRenderState *state);

typedef struct PhotonCommand {
	PhotonCommandFunc invoke;
	struct PhotonCommand *next;
} PhotonCommand;

typedef struct PhotonDrawCommand {
	PhotonCommand super;
	
	const PhotonRenderState *state;
	PhotonUniformBindings uniformBindings;
	
	size_t vertexPage, count, firstIndex;
} PhotonDrawCommand;

typedef struct PhotonScissorCommand {
	PhotonCommand command;
	
	PhotonScissorState state;
	paabb2 rect;
	
	struct PhotonScissorCommand *next;
} PhotonScissorCommand;

typedef struct PhotonRenderTextureBindCommand {
	PhotonCommand command;
	
	PhotonRenderTexture *renderTexture;
	PhotonLoadAction loadAction;
	PhotonStoreAction storeAction;
	pvec4 clearColor;
} PhotonRenderTextureBindCommand;

struct PhotonRenderer {
	size_t vertexPageBound;
	PhotonBufferSet *buffers;
	
	size_t globalUniformBlockOffset, globalUniformBlockSize;
	// TODO global texture bindings.
	
	PhotonScratchBlock *scratchBlock;
	
	PhotonCommand *firstCommand, **nextCommand;
	
	// Current draw command being batched, or NULL if none.
	PhotonDrawCommand *batchCommand;
	
	// Most recent framebuffer bind command.
	PhotonRenderTextureBindCommand *framebufferBindCommand;
	
	// Size of the default framebuffer.
	pvec2 defaultFramebufferSize;
	
	// Fence object to track when the renderer finishes.
	GLsync fence;
};


//MARK: Utils

void
_PhotonPrintErrors(const char *file, int line)
{
	for(GLenum err; (err = glGetError());){
		const char *error = "Unknown Error";
		
		switch(err){
			case 0x0500: error = "Invalid Enumeration"; break;
			case 0x0501: error = "Invalid Value"; break;
			case 0x0502: error = "Invalid Operation"; break;
		}
		
		fprintf(stderr, "GL Error(%s:%d): %s\n", file, line, error);
		assert(false);
	}
}


//MARK: BlendModes

static const GLenum BlendFactors[] = {
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA,
};

static const GLenum BlendOps[] = {
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX,	
};

PhotonBlendMode PhotonBlendModePremultipliedAlpha = {
	PhotonBlendOpAdd, PhotonBlendFactorOne, PhotonBlendFactorOneMinusSrcAlpha,
	PhotonBlendOpAdd, PhotonBlendFactorOne, PhotonBlendFactorOneMinusSrcAlpha,
};

PhotonBlendMode PhotonBlendModeAdd = {
	PhotonBlendOpAdd, PhotonBlendFactorOne, PhotonBlendFactorOne,
	PhotonBlendOpAdd, PhotonBlendFactorOne, PhotonBlendFactorOne,
};

PhotonBlendMode PhotonBlendModeSubtract = {
	PhotonBlendOpReverseSubtract, PhotonBlendFactorOne, PhotonBlendFactorOne,
	PhotonBlendOpReverseSubtract, PhotonBlendFactorOne, PhotonBlendFactorOne,
};

PhotonBlendMode PhotonBlendModeMultiply = {
	PhotonBlendOpAdd, PhotonBlendFactorDstColor, PhotonBlendFactorZero,
	PhotonBlendOpAdd, PhotonBlendFactorDstColor, PhotonBlendFactorZero,
};


//MARK: Textures

bool
PhotonTextureOptionsEqual(const PhotonTextureOptions *a, const PhotonTextureOptions *b)
{
	return (
		a->format == b->format &&
		a->generateMipmaps == b->generateMipmaps &&
		a->minFilter == b->minFilter &&
		a->magFilter == b->magFilter &&
		a->addressX == b->addressX &&
		a->addressY == b->addressY
	);
}

const PhotonTextureOptions PhotonTextureOptionsDefault = {
	.format = PhotonTextureFormatRGBA8,
	.generateMipmaps = false,
	.minFilter = PhotonTextureFilterLinear,
	.magFilter = PhotonTextureFilterLinear,
	.mipFilter = PhotonTextureFilterMipmapNone,
	.addressX = PhotonTextureAddressModeClampToEdge,
	.addressY = PhotonTextureAddressModeClampToEdge,
};

const PhotonTextureOptions PhotonTextureOptionsTrilinearRepeat = {
	.format = PhotonTextureFormatRGBA8,
	.generateMipmaps = true,
	.minFilter = PhotonTextureFilterLinear,
	.magFilter = PhotonTextureFilterLinear,
	.mipFilter = PhotonTextureFilterLinear,
	.addressX = PhotonTextureAddressModeRepeat,
	.addressY = PhotonTextureAddressModeRepeat,
};

static GLenum TextureInternalFormat[] = {GL_R8, GL_RG8, GL_RGBA8, GL_SRGB8_ALPHA8, GL_RGBA16F};
static GLenum TextureFormat[] = {GL_RED, GL_RG, GL_RGBA, GL_RGBA, GL_RGBA};
static GLenum TextureType[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_HALF_FLOAT};
static GLenum TextureAddressMode[] = {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_REPEAT, GL_MIRRORED_REPEAT};

static GLenum TextureFilters[3][3] = {
	{GL_LINEAR, GL_LINEAR, GL_LINEAR}, // Invalid enum, fall back to linear.
	{GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR}, // nearest
	{GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR}, // linear
};

PhotonTexture *
PhotonTextureNew(int width, int height, const void *pixelData, const PhotonTextureOptions *options)
{
	options = options ?: &PhotonTextureOptionsDefault;
	
	PhotonTexture *texture = calloc(1, sizeof(*texture));
	texture->options = *options;
	texture->width = width;
	texture->height = height;
	
	glGenTextures(1, &texture->textureID);
	glBindTexture(GL_TEXTURE_2D, texture->textureID);
	
	GLenum internalFormat = TextureInternalFormat[options->format];
	GLenum format = TextureFormat[options->format];
	GLenum type = TextureType[options->format];
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (GLint)width, (GLint)height, 0, format, type, pixelData);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureAddressMode[options->addressX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureAddressMode[options->addressY]);
	
	GLenum minFilter = TextureFilters[options->minFilter][PhotonTextureFilterMipmapNone];
	GLenum magFilter = TextureFilters[options->magFilter][options->mipFilter];
	assert(minFilter);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	
	if(options->generateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
		
	glBindTexture(GL_TEXTURE_2D, 0);
	PhotonPrintErrors();
	
	return texture;
}

void
PhotonTextureFree(PhotonTexture *texture)
{
	if(texture){
		glDeleteTextures(1, &texture->textureID);
		free(texture);
	}
}

void
PhotonTextureGetInfo(PhotonTexture *texture, int *width, int *height, const PhotonTextureOptions **options)
{
	(*width) = texture->width;
	(*height) = texture->height;
	(*options) = &texture->options;
}

void
PhotonTextureSubImage(PhotonTexture *texture, int x, int y, int w, int h, size_t stride, const void *pixelData)
{
	glBindTexture(GL_TEXTURE_2D, texture->textureID);
	
	GLenum format = TextureFormat[texture->options.format];
	GLenum type = TextureType[texture->options.format];
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)stride);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, (GLint)w, (GLint)h, format, type, pixelData);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	PhotonPrintErrors();
}


//MARK: RenderTexture

PhotonRenderTexture *
PhotonRenderTextureNew(PhotonTexture *texture)
{
	PhotonRenderTexture *renderTexture = calloc(1, sizeof(*renderTexture));
	renderTexture->texture = texture;
	
	glGenFramebuffers(1, &renderTexture->framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, renderTexture->framebufferID);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->textureID, 0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	PhotonPrintErrors();
	
	return renderTexture;
}

void
PhotonRenderTextureFree(PhotonRenderTexture *renderTexture)
{
	if(renderTexture){
		glDeleteFramebuffers(1, &renderTexture->framebufferID);
		free(renderTexture);
	}
}

PhotonTexture *
PhotonRenderTextureGetTexture(PhotonRenderTexture *renderTexture)
{
	return renderTexture->texture;
}


//MARK: Shaders

typedef void (* GetShaderivFunc) (GLuint shaderID, GLenum pname, GLint* param);
typedef void (* GetShaderInfoLogFunc) (GLuint shaderID, GLsizei bufSize, GLsizei* length, GLchar* infoLog);

static bool
PhotonCheckShaderError(GLuint objID, GLenum status, GetShaderivFunc getiv, GetShaderInfoLogFunc getInfoLog)
{
	GLint success;
	getiv(objID, status, &success);
	
	if(!success){
		GLint length;
		getiv(objID, GL_INFO_LOG_LENGTH, &length);
		
		char log[length];
		getInfoLog(objID, length, NULL, log);
		PhotonPrintErrors();
		
		fprintf(stderr, "Shader compile error for 0x%04X: %s\n", status, log);
		return false;
	} else {
		return true;
	}
}

static void
PhotonLogShader(const char *label, const char *source)
{
	fprintf(stderr, "%s\n", label);
	
	const char *cursor = source;
	for(int line = 1; true; line++){
		fprintf(stderr, "% 4d: ", line);
		
		while(true){
			if(cursor[0] == '\0') return;
			
			fputc(cursor[0], stderr);
			cursor++;
			
			if(cursor[-1] == '\n') break;
		}
	}
}

static GLuint
CompileShaderSource(GLenum type, const char *source)
{
	GLuint shaderID = glCreateShader(type);
	glShaderSource(shaderID, 1, (const GLchar *[]){source}, NULL);
	glCompileShader(shaderID);
	PhotonPrintErrors();
	
	if(PhotonCheckShaderError(shaderID, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog)){
		return shaderID;
	} else {
		PhotonLogShader((type == GL_VERTEX_SHADER) ? "Vertex Shader:" : "Fragment Shader:", source);
		glDeleteShader(shaderID);
		PhotonPrintErrors();
		
		return 0;
	}
}

static GLuint
CompileShader(const char *vsource, const char *fsource)
{
	GLuint shaderID = glCreateProgram();
	PhotonPrintErrors();
	
	glBindAttribLocation(shaderID, PhotonAttributePosition, "PhotonAttributePosition");
	glBindAttribLocation(shaderID, PhotonAttributeUV1     , "PhotonAttributeUV1");
	glBindAttribLocation(shaderID, PhotonAttributeUV2     , "PhotonAttributeUV2");
	glBindAttribLocation(shaderID, PhotonAttributeColor   , "PhotonAttributeColor");
	PhotonPrintErrors();
	
	glBindFragDataLocation(shaderID, 0, "PhotonFragOut");
	PhotonPrintErrors();
	
	GLuint vshaderID = CompileShaderSource(GL_VERTEX_SHADER, vsource);
	if(!vshaderID) goto cleanup;
	glAttachShader(shaderID, vshaderID);
	PhotonPrintErrors();
	
	GLuint fshaderID = CompileShaderSource(GL_FRAGMENT_SHADER, fsource);
	if(!fshaderID) goto cleanup;
	glAttachShader(shaderID, fshaderID);
	PhotonPrintErrors();
	
	glLinkProgram(shaderID);
	glDeleteShader(vshaderID);
	glDeleteShader(fshaderID);
	PhotonPrintErrors();
	
	if(PhotonCheckShaderError(shaderID, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog)){
		return shaderID;
	} else {
		if(vshaderID && fshaderID){
			PhotonLogShader("Vertex Shader", vsource);
			PhotonLogShader("Fragment Shader", fsource);
		}
		
		cleanup:
		glDeleteProgram(shaderID);
		glDeleteShader(vshaderID);
		glDeleteShader(fshaderID);
		PhotonPrintErrors();
		
		return 0;
	}
}

enum {
	PhotonUniformBlockGlobals,
	PhotonUniformBlockLocals,
};

static bool CompareNames(const char *staticName, const char *uniformName, const GLsizei nameLength){
	return nameLength == strlen(staticName) && strncmp(uniformName, staticName, nameLength) == 0;
}

PhotonShader *
PhotonShaderNew(const char *vshader, const char *fshader)
{
	PhotonShader *shader = calloc(1, sizeof(*shader));
	GLuint programID = CompileShader(vshader, fshader);
	
	GLint bufferAlignment = 1;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &bufferAlignment);
	
	if(programID){
		shader->programID = programID;
		glUseProgram(programID);
		
		size_t nameCursor = 0;
		
		int blocks = 0;
		glGetProgramiv(programID, GL_ACTIVE_UNIFORM_BLOCKS, &blocks);
		
		for(int i = 0; i < blocks; i++){
			GLchar *name = shader->nameBuffer + nameCursor;
			GLsizei nameLength = 0;
			GLsizei size = 0;
			
			glGetActiveUniformBlockName(programID, i, (GLsizei)(PHOTON_NAME_BUFFER_SIZE - nameCursor - 1), &nameLength, name);
			glGetActiveUniformBlockiv(programID, i, GL_UNIFORM_BLOCK_DATA_SIZE, (GLint *)&size);
			
			if(CompareNames("PhotonGlobals", name, nameLength)){
				glUniformBlockBinding(programID, glGetUniformBlockIndex(programID, name), PhotonUniformBlockGlobals);
			} else if(CompareNames("PhotonLocals", name, nameLength)){
				glUniformBlockBinding(programID, glGetUniformBlockIndex(programID, name), PhotonUniformBlockLocals);
				shader->localUniformBlockSize = size;
			} else {
				fprintf(stderr, "Photon: Igonored uniform block '%s'\n", name);
			}
			
			PhotonPrintErrors();
		}
		
		int uniforms = 0;
		glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &uniforms);
		
		GLuint textureBinding = 0;
		for(int i = 0; i < uniforms; i++){
			GLchar *name = shader->nameBuffer + nameCursor;
			GLsizei nameLength = 0;
			GLsizei size = 0;
			GLenum type = 0;
			
			glGetActiveUniform(programID, i, (GLsizei)(PHOTON_NAME_BUFFER_SIZE - nameCursor - 1), &nameLength, &size, &type, name);
			
			GLint location = glGetUniformLocation(programID, name);
			if(location >= 0){
				switch(type){
					case GL_SAMPLER_2D: {
						shader->uniformTextureNames[textureBinding] = name;
						nameCursor += nameLength + 1;
						
						glUniform1i(location, textureBinding);
						PhotonPrintErrors();
						textureBinding++;
					} break;
					default: {
						fprintf(stderr, "Photon: Igonored uniform '%s' (type: 0x%04X)\n", name, type);
					} break;
				}
			}
			
			PhotonPrintErrors();
		}
		
		glUseProgram(0);
		PhotonPrintErrors();
		
		return shader;
	} else {
		PhotonShaderFree(shader);
		return NULL;
	}
}

void
PhotonShaderFree(PhotonShader *shader)
{
	if(shader){
		glDeleteShader(shader->programID);
		PhotonPrintErrors();
		
		free(shader);
	}
}


//MARK: Uniforms

static size_t
PhotonUniformsAllocSize(const PhotonShader *shader)
{
	assert(shader);
	return sizeof(PhotonUniforms) + shader->localUniformBlockSize;
}

static PhotonUniforms *
PhotonUniformsAlloc(const PhotonShader *shader)
{
	return (PhotonUniforms *)calloc(1, PhotonUniformsAllocSize(shader));
}

static PhotonUniforms *
PhotonUniformsInit(PhotonUniforms *uniforms, const PhotonShader *shader)
{
	assert(shader);
	uniforms->shader = shader;
	
	return uniforms;
}

PhotonUniforms *
PhotonUniformsNew(const PhotonShader *shader)
{
	return PhotonUniformsInit(PhotonUniformsAlloc(shader), shader);
}

void
PhotonUniformsFree(PhotonUniforms *uniforms)
{
	if(uniforms){
		free(uniforms);
	}
}

static int
IndexForName(const char *name, const char * const names[], int max)
{
	for(int i = 0; i < max; i++){
		if(names[i] == NULL){
			break;
		} else if(strcmp(name, names[i]) == 0){
			return i;
		}
	}
	
	return -1;
}

void
PhotonUniformsSetLocals(PhotonUniforms *uniforms, const void *ptr)
{
	const PhotonShader *shader = uniforms->shader;
	memcpy(uniforms->block, ptr, shader->localUniformBlockSize);
}

void
PhotonUniformsSetTexture(PhotonUniforms *uniforms, const char *name, const PhotonTexture *texture)
{
	const PhotonShader *shader = uniforms->shader;
	int i = IndexForName(name, shader->uniformTextureNames, PHOTON_TEXTURE_BINDING_MAX);
	
	if(i >= 0) uniforms->textures[i] = texture;
}


//MARK: RenderStates

static PhotonRenderState *
PhotonRenderStateAlloc()
{
	return (PhotonRenderState *)calloc(1, sizeof(PhotonRenderState));
}

static PhotonRenderState *
PhotonRenderStateInit(PhotonRenderState *state, const PhotonBlendMode *blendMode, const PhotonUniforms *uniforms)
{
	assert(uniforms);
	
	state->blendMode = blendMode;
	state->shader = uniforms->shader;
	state->uniforms = uniforms;
	
	return state;
}

PhotonRenderState *
PhotonRenderStateNew(const PhotonBlendMode *blendMode, const PhotonUniforms *uniforms)
{
	return PhotonRenderStateInit(PhotonRenderStateAlloc(), blendMode, uniforms);
}

void
PhotonRenderStateFree(PhotonRenderState *state)
{
	if(state){
		free(state);
	}
}

static void
PhotonRenderStateTransition(const PhotonRenderState *currentState, const PhotonRenderState *destinationState, const PhotonUniformBindings *uniformBindings)
{
	currentState = currentState ?: (PhotonRenderState[]){{NULL, NULL, NULL}};
	if(currentState == destinationState) return;
	
	const PhotonBlendMode *blend = destinationState->blendMode;
	if(currentState->blendMode != blend){
		if(blend){
			glEnable(GL_BLEND);
			glBlendEquationSeparate(BlendOps[blend->colorOp], BlendOps[blend->alphaOp]);
			glBlendFuncSeparate(
				BlendFactors[blend->colorSrcFactor], BlendFactors[blend->colorDstFactor],
				BlendFactors[blend->alphaSrcFactor], BlendFactors[blend->alphaDstFactor]
			);
		} else {
			glDisable(GL_BLEND);
		}
		
		PhotonPrintErrors();
	}
	
	const PhotonUniforms *uniforms = destinationState->uniforms;
	if(uniforms && currentState->uniforms != uniforms){
		const PhotonShader *shader = destinationState->shader;
		if(currentState->shader != shader){
			glUseProgram(shader->programID);
			
	//		glValidateProgram(shader);
	//		GLint validate = 0; glGetProgramiv(shader, GL_VALIDATE_STATUS, &validate);
	//		if(!validate){
	//			fprintf(stderr, "GL Error: Shader failed to validate.");
	//		}
			
			PhotonPrintErrors();
		}
		
		size_t size = uniformBindings->localUniformBlockSize;
		if(size > 0){
			glBindBufferRange(GL_UNIFORM_BUFFER, PhotonUniformBlockLocals, uniformBindings->bufferID, uniformBindings->localUniformBlockOffset, size);
			PhotonPrintErrors();
		}
		
		for(int i = 0; i < PHOTON_TEXTURE_BINDING_MAX; i++){
			GLuint textureID = uniformBindings->textureIDs[i];
			if(textureID){
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, textureID);
			} else {
				break;
			}
			
			PhotonPrintErrors();
		}
	}
}


//MARK: Buffers

static const GLenum BufferTypes[] = {
	GL_ARRAY_BUFFER,
	GL_ELEMENT_ARRAY_BUFFER,
	GL_UNIFORM_BUFFER,
};

static void
PhotonBufferInit(PhotonBuffer *buffer, size_t capacity, size_t elementSize, enum PhotonBufferType type)
{
	buffer->destroy = NULL;
	buffer->resize = NULL;
	buffer->prepare = NULL;
	buffer->commit = NULL;
	
	buffer->count = 0;
	buffer->capacity = capacity;
	buffer->elementSize = elementSize;
	buffer->type = type;
}

static void
PhotonBufferFree(PhotonBuffer *buffer)
{
	if(buffer){
		if(buffer->destroy) buffer->destroy(buffer);
		free(buffer);
	}
}

static void *
PhotonBufferPushElements(PhotonBuffer *buffer, size_t requestedCount)
{
	assert(requestedCount >= 0);
	
	size_t required = buffer->count + requestedCount;
	size_t capacity = buffer->capacity;
	if(required > capacity){
		buffer->resize(buffer, 3*required/2);
	}
	
	intptr_t array = (intptr_t)buffer->ptr + buffer->count*buffer->elementSize;
	buffer->count += requestedCount;
	
	return (void *)array;
}

static void
PhotonBufferPopElements(PhotonBuffer *buffer, size_t requestedCount)
{
	assert(requestedCount >= 0);
	buffer->count -= requestedCount;
}

static void
PhotonBufferPrepare(PhotonBuffer *buffer)
{
	buffer->count = 0;
	if(buffer->prepare) buffer->prepare(buffer);
}

static void
PhotonBufferCommit(PhotonBuffer *buffer)
{
	if(buffer->commit) buffer->commit(buffer);
}


//MARK: GL3 Buffer

static void
PhotonBufferGL3Destroy(PhotonBufferGL3 *buffer)
{
	glDeleteBuffers(1, &buffer->bufferID);
	buffer->bufferID = 0;
	PhotonPrintErrors();
}

#define BUFFER_ACCESS_READ (GL_MAP_READ_BIT)
#define BUFFER_ACCESS_WRITE (GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT)

static void
PhotonBufferGL3Resize(PhotonBufferGL3 *buffer, size_t newCapacity)
{
	GLenum target = BufferTypes[buffer->super.type];
	
	// This is a little tricky.
	// Need to resize the existing GL buffer object without creating a new name.
	
	// Make the buffer readable.
	glBindBuffer(target, buffer->bufferID);
	size_t oldLength = (buffer->super.count*buffer->super.elementSize);
	glFlushMappedBufferRange(target, 0, oldLength);
	glUnmapBuffer(target);
	void *oldBufferPtr = glMapBufferRange(target, 0, oldLength, BUFFER_ACCESS_READ);
	
	// Copy the old contents into a temp buffer.
	size_t newLength = (newCapacity*buffer->super.elementSize);
	void *tempBufferPtr = malloc(newLength);
	memcpy(tempBufferPtr, oldBufferPtr, oldLength);
	
	// Copy that into a new GL buffer.
	glUnmapBuffer(target);
	glBufferData(target, newLength, tempBufferPtr, GL_DYNAMIC_DRAW);
	void *newBufferPtr = glMapBufferRange(target, 0, newLength, BUFFER_ACCESS_WRITE);
	
	// Cleanup.
	free(tempBufferPtr);
	glBindBuffer(target, 0);
	PhotonPrintErrors();
	
	// Update values.
	buffer->super.ptr = newBufferPtr;
	buffer->super.capacity = newCapacity;
}

static void
PhotonBufferGL3Prepare(PhotonBufferGL3 *buffer)
{
	GLenum target = BufferTypes[buffer->super.type];
	
	glBindBuffer(target, buffer->bufferID);
	buffer->super.ptr = glMapBufferRange(target, 0, buffer->super.capacity*buffer->super.elementSize, BUFFER_ACCESS_WRITE);
	glBindBuffer(target, 0);
	PhotonPrintErrors();
}

static void
PhotonBufferGL3Commit(PhotonBufferGL3 *buffer)
{
	GLenum target = BufferTypes[buffer->super.type];
	
	glBindBuffer(target, buffer->bufferID);
	glFlushMappedBufferRange(target, 0, buffer->super.count*buffer->super.elementSize);
	glUnmapBuffer(target);
	glBindBuffer(target, 0);
	PhotonPrintErrors();
	
	buffer->super.ptr = NULL;
}

static PhotonBufferGL3 *
PhotonBufferGL3New(size_t capacity, size_t elementSize, enum PhotonBufferType type){
	PhotonBufferGL3 *buffer = calloc(1, sizeof(*buffer));
	PhotonBufferInit((PhotonBuffer *)buffer, capacity, elementSize, type);
	buffer->super.destroy = (PhotonBufferDestroyFunc)PhotonBufferGL3Destroy;
	buffer->super.resize = (PhotonBufferResizeFunc)PhotonBufferGL3Resize;
	buffer->super.prepare = (PhotonBufferCommitFunc)PhotonBufferGL3Prepare;
	buffer->super.commit = (PhotonBufferCommitFunc)PhotonBufferGL3Commit;
	
	GLenum target = BufferTypes[type];
	assert(target);
	
	glGenBuffers(1, &buffer->bufferID);
	glBindBuffer(target, buffer->bufferID);
	glBufferData(target, buffer->super.capacity*buffer->super.elementSize, NULL, GL_DYNAMIC_DRAW);
	
	PhotonPrintErrors();
	return buffer;
}


//MARK: Buffer Set

static void
PhotonBufferSetBind(const PhotonBufferSet *buffers, size_t vertexPage)
{
	if(buffers){
		glBindVertexArray(buffers->vertexArrayID);
	} else {
		return;
	}
	
	if(vertexPage == NO_PAGE){
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, buffers->vertexes->bufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers->indexes->bufferID);
		PhotonPrintErrors();
		
		glEnableVertexAttribArray(PhotonAttributePosition);
		glEnableVertexAttribArray(PhotonAttributeUV1     );
		glEnableVertexAttribArray(PhotonAttributeUV2     );
		glEnableVertexAttribArray(PhotonAttributeColor   );
		PhotonPrintErrors();
		
		GLsizei stride = sizeof(PhotonVertex);
		size_t pageOffset = vertexPage*(1<<16)*sizeof(PhotonVertex);
		glVertexAttribPointer(PhotonAttributePosition, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(pageOffset + offsetof(PhotonVertex, position)));
		glVertexAttribPointer(PhotonAttributeUV1     , 2, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(pageOffset + offsetof(PhotonVertex, uv1     )));
		glVertexAttribPointer(PhotonAttributeUV2     , 2, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(pageOffset + offsetof(PhotonVertex, uv2     )));
		glVertexAttribPointer(PhotonAttributeColor   , 4, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(pageOffset + offsetof(PhotonVertex, color   )));
		PhotonPrintErrors();
		
		// TODO unbind buffers?
	}
}

static PhotonBufferSet *
PhotonBufferSetNew(size_t vertexCapacity, size_t indexCapacity, size_t uniformCapacity)
{
	PhotonBufferSet *buffers = calloc(1, sizeof(*buffers));
	
	GLint uniformAlignment = 1;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformAlignment);
	
	buffers->vertexes = PhotonBufferGL3New(vertexCapacity , sizeof(PhotonVertex), PhotonBufferTypeVertex);
	buffers->indexes  = PhotonBufferGL3New(indexCapacity, sizeof(PhotonIndex), PhotonBufferTypeIndex);
	buffers->uniforms = PhotonBufferGL3New(uniformCapacity, uniformAlignment, PhotonBufferTypeUniform);
	
	glGenVertexArrays(1, &buffers->vertexArrayID);
	PhotonBufferSetBind(buffers, 0);
	
	PhotonBufferSetBind(NULL, 0);
	PhotonPrintErrors();
	
	return buffers;
}

static void
PhotonBufferSetFree(PhotonBufferSet *buffers)
{
	if(buffers){
		glDeleteVertexArrays(1, &buffers->vertexArrayID);
		
		PhotonBufferFree((PhotonBuffer *)buffers->vertexes); buffers->vertexes = NULL;
		PhotonBufferFree((PhotonBuffer *)buffers->indexes ); buffers->indexes  = NULL;
		PhotonBufferFree((PhotonBuffer *)buffers->uniforms); buffers->uniforms = NULL;
		
		free(buffers);
	}
}

static void
PhotonBufferSetPrepareForAccess(PhotonBufferSet *buffers)
{
	PhotonBufferPrepare((PhotonBuffer *)buffers->vertexes);
	PhotonBufferPrepare((PhotonBuffer *)buffers->indexes );
	PhotonBufferPrepare((PhotonBuffer *)buffers->uniforms);
}

static void
PhotonBufferSetPrepareForRendering(PhotonBufferSet *buffers)
{
	PhotonBufferCommit((PhotonBuffer *)buffers->vertexes);
	PhotonBufferCommit((PhotonBuffer *)buffers->indexes );
	PhotonBufferCommit((PhotonBuffer *)buffers->uniforms);
}

//MARK: Scratch Memory 

static PhotonScratchBlock *
AllocScratchBlock(size_t size, PhotonScratchBlock *next)
{
	PhotonScratchBlock *scratch = calloc(1, size + sizeof(PhotonScratchBlock));
	scratch->next = next;
	scratch->count = 0;
	scratch->capacity = size;
	
	return scratch;
}

static void
FreeScratchBlocks(PhotonScratchBlock *scratch)
{
	if(scratch){
		FreeScratchBlocks(scratch->next);
		free(scratch);
	}
}


//MARK: Renderer

PhotonRenderer *
PhotonRendererNew(void)
{
	PhotonRenderer *renderer = calloc(1, sizeof(*renderer));
	
	renderer->buffers = PhotonBufferSetNew(1024, 1024, 32*1024);
	
	return renderer;
}

void
PhotonRendererFree(PhotonRenderer *renderer)
{
	if(renderer){
		PhotonBufferSetFree(renderer->buffers);
		FreeScratchBlocks(renderer->scratchBlock);
		
		if(renderer->fence) glDeleteSync(renderer->fence);

		free(renderer);
	}
}

static void
PhotonRendererBindBuffers(PhotonRenderer *renderer, size_t vertexPage)
{
	if(renderer->vertexPageBound != vertexPage){
		PhotonBufferSetBind(renderer->buffers, vertexPage);
		renderer->vertexPageBound = vertexPage;
	}
}

void *
PhotonRendererTemporaryMemory(PhotonRenderer *renderer, size_t size)
{
	PhotonScratchBlock *scratch = renderer->scratchBlock;
	
	if(!scratch){
		// ALlocate the initial scratch buffer.
		renderer->scratchBlock = AllocScratchBlock(8, NULL);
		return PhotonRendererTemporaryMemory(renderer, size);
	} else if(size > scratch->capacity - scratch->count){
		// Not enough space remaining in the current buffer.
		size_t requested = 3*(scratch->count + size)/2;
		renderer->scratchBlock = AllocScratchBlock(requested, renderer->scratchBlock);
		
		return PhotonRendererTemporaryMemory(renderer, size);
	} else {
		// Allocate from the current buffer.
		void *ptr = scratch->block + scratch->count;
		bzero(ptr, size);
		
		scratch->count += size;
		return ptr;
	}
}

PhotonUniforms *
PhotonRendererTemporaryUniforms(PhotonRenderer *renderer, const PhotonShader *shader)
{
	return PhotonUniformsInit(PhotonRendererTemporaryMemory(renderer, PhotonUniformsAllocSize(shader)), shader);
}

PhotonRenderState *
PhotonRendererTemporaryRenderState(PhotonRenderer *renderer, const PhotonBlendMode *blendMode, const PhotonUniforms *uniforms)
{
	return PhotonRenderStateInit(PhotonRendererTemporaryMemory(renderer, sizeof(PhotonRenderState)), blendMode, uniforms);
}

void
PhotonRendererResetBatch(PhotonRenderer *renderer)
{
	renderer->batchCommand = NULL;
}

void
PhotonRendererPrepare(PhotonRenderer *renderer, pvec2 defaultFramebufferSize)
{
	renderer->defaultFramebufferSize = defaultFramebufferSize;
	
	PhotonScratchBlock *scratch = renderer->scratchBlock;
	if(scratch){
		// Free all but the first (largest) scratch block.
		FreeScratchBlocks(scratch->next);
		
		// Reset the block.
		scratch->next = NULL;
		scratch->count = 0;
	}
	
	PhotonRendererBindBuffers(renderer, NO_PAGE);
	PhotonBufferSetPrepareForAccess(renderer->buffers);
	
	// Reset command list.
	renderer->firstCommand = NULL;
	renderer->nextCommand = &renderer->firstCommand;
	
	PhotonRendererResetBatch(renderer);
}

static size_t
AlignSize(size_t size, size_t alignment)
{
	return (size - 1)/alignment + 1;
}

void
PhotonRendererSetGlobals(PhotonRenderer *renderer, void *ptr, size_t size)
{
	PhotonBufferGL3 *uniforms = renderer->buffers->uniforms;
	renderer->globalUniformBlockOffset = uniforms->super.count;
	
	GLint bufferAlignment = 1;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &bufferAlignment);
	
	renderer->globalUniformBlockSize = size;
	
	void *block = PhotonBufferPushElements((PhotonBuffer *)uniforms, AlignSize(size, uniforms->super.elementSize));
	memcpy(block, ptr, size);
	
	glBindBufferRange(GL_UNIFORM_BUFFER, PhotonUniformBlockGlobals, uniforms->bufferID, renderer->globalUniformBlockOffset, size);
	PhotonPrintErrors();
}


static void
PhotonRendererExecute(PhotonRenderer *renderer)
{
	const PhotonRenderState *currentState = NULL;
	for(PhotonCommand *command = renderer->firstCommand; command; command = command->next){
		currentState = command->invoke(command, renderer, currentState);
	}
}

void
PhotonRendererFlush(PhotonRenderer *renderer)
{
	PhotonBufferSetPrepareForRendering(renderer->buffers);
	PhotonRendererExecute(renderer);
	
	renderer->fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	PhotonPrintErrors();
}

bool
PhotonRendererIsReady(PhotonRenderer *renderer)
{
	PhotonRendererWait(renderer, 0);
	
	// Renderer is not ready as long as the fence still exists.
	return (renderer->fence == NULL);
}

bool
PhotonRendererWait(PhotonRenderer *renderer, double seconds)
{
	if(renderer->fence){
		uint64_t nanos = seconds*1e9;
		switch(glClientWaitSync(renderer->fence, GL_SYNC_FLUSH_COMMANDS_BIT, nanos)){
			case GL_ALREADY_SIGNALED:
			case GL_CONDITION_SATISFIED:
				// Fence has completed. Clean it up.
				glDeleteSync(renderer->fence);
				renderer->fence = NULL;
				return true;
			default:
				PhotonPrintErrors();
			case GL_TIMEOUT_EXPIRED:
				return false;
		}
	} else {
		return true;
	}
}

static void *
PhotonRendererPushCommand(PhotonRenderer *renderer, size_t commandSize, PhotonCommandFunc commandFunc)
{
	PhotonCommand *command = PhotonRendererTemporaryMemory(renderer, commandSize);
	
	 // Setup command. 
	bzero(command, commandSize);
	command->invoke = commandFunc;
	
	// Push command to the list.
	(*renderer->nextCommand) = command;
	renderer->nextCommand = &command->next;
	
	return command;
}

static pvec2
PhotonRendererGetFramebufferSize(PhotonRenderer *renderer)
{
	PhotonRenderTextureBindCommand *binding = renderer->framebufferBindCommand;
	assert(binding);
	
	PhotonRenderTexture *renderTexture = binding->renderTexture; 
	if(renderTexture){
		PhotonTexture *texture = renderTexture->texture; 
		return (pvec2){texture->width, texture->height};
	} else {
		return renderer->defaultFramebufferSize;
	}
}


//MARK: Photon Commands

static void
BufferUniforms(PhotonRenderer *renderer, const PhotonRenderState *state, PhotonUniformBindings *uniformBindings)
{
	const PhotonShader *shader = state->shader;
	size_t size = shader->localUniformBlockSize;
	if(size > 0){
		PhotonBufferGL3 *uniformBuffer = renderer->buffers->uniforms;
		uniformBindings->bufferID = uniformBuffer->bufferID;
		uniformBindings->localUniformBlockOffset = uniformBuffer->super.count*uniformBuffer->super.elementSize;
		uniformBindings->localUniformBlockSize = size;
		
		void *block = PhotonBufferPushElements((PhotonBuffer *)uniformBuffer, AlignSize(size, uniformBuffer->super.elementSize));
		memcpy(block, state->uniforms->block, size);
	}
	
	for(int i = 0; i < PHOTON_TEXTURE_BINDING_MAX; i++){
		if(shader->uniformTextureNames[i] == NULL) break;
		const PhotonTexture *texture = state->uniforms->textures[i];
		
		if(texture == NULL){
			// TODO implement global textures here?
		} else {
			uniformBindings->textureIDs[i] = texture->textureID;
		}
	}
}


//MARK: Draw Command

static const PhotonRenderState *
PhotonDrawCommandInvoke(const PhotonCommand *command, PhotonRenderer *renderer, const PhotonRenderState *currentState)
{
	assert(renderer->framebufferBindCommand);
	
	const PhotonDrawCommand *draw = (PhotonDrawCommand *)command;
	const PhotonRenderState *state = draw->state;
	PhotonRenderStateTransition(currentState, state, &draw->uniformBindings);
	
	PhotonRendererBindBuffers(renderer, draw->vertexPage);
	glDrawElements(GL_TRIANGLES, (GLsizei)draw->count, GL_UNSIGNED_SHORT, (GLvoid *)(draw->firstIndex*sizeof(PhotonIndex)));
	PhotonPrintErrors();
	
	return state;
}

static inline size_t
VertexPageOffset(size_t firstVertex, size_t vertexCount)
{
	assert(vertexCount <= UINT16_MAX + 1);
	
	// Space remaining on the current vertex page.
	size_t remaining = (firstVertex | UINT16_MAX) - firstVertex + 1;
	
	if(remaining >= vertexCount){
		// Allocation will not overlap a page boundary. 
		return 0;
	} else {
		// Offset the allocation to the next page.
		return remaining;
	}
}

PhotonRenderBuffers
PhotonRendererEnqueueTriangles(PhotonRenderer *renderer, size_t triangleCount, size_t vertexCount, const PhotonRenderState *state)
{
	const PhotonBufferSet *buffers = renderer->buffers;
	const size_t firstVertex = buffers->vertexes->super.count;
	const size_t firstIndex  = buffers->indexes->super.count;
	
	// Vertex buffers are split into pages of 64k vertexes since GLES2 requires indexing with 16 bit uints.
	// Normally the value is 0 unless the allocation would overflow the current page.
	const size_t vertexPageOffset = VertexPageOffset(firstVertex, vertexCount);
	
	const size_t indexCount = 3*triangleCount;
	PhotonIndex *indexes   = (PhotonIndex *)PhotonBufferPushElements((PhotonBuffer *)buffers->indexes , indexCount);
	PhotonVertex *vertexes = (PhotonVertex *)PhotonBufferPushElements((PhotonBuffer *)buffers->vertexes, vertexCount + vertexPageOffset) + vertexPageOffset;
	
	assert(indexes);
	assert(vertexes);
	
	const size_t vertexPage = (firstVertex + vertexPageOffset) >> 16;
	PhotonDrawCommand *previous = renderer->batchCommand;
	if(previous && previous->state == state && previous->vertexPage == vertexPage){
		renderer->batchCommand->count += indexCount;
	} else {
		PhotonDrawCommand *draw = PhotonRendererPushCommand(renderer, sizeof(*draw), PhotonDrawCommandInvoke);
		draw->state = state;
		draw->count = indexCount;
		draw->vertexPage = vertexPage;
		draw->firstIndex = firstIndex;
		
		renderer->batchCommand = draw;
		
		if(state->uniforms){
			BufferUniforms(renderer, state, &draw->uniformBindings);
		}
	}
	
	const size_t vertexPageIndex = (firstVertex + vertexPageOffset) & 0xFFFF;
	return (PhotonRenderBuffers){vertexes, indexes, (PhotonIndex)vertexPageIndex};
}

void
PhotonRendererPopTriangles(PhotonRenderer *renderer, size_t triangles, size_t vertexes)
{
	const size_t indexCount = 3*triangles;
	
	PhotonDrawCommand *batch = renderer->batchCommand;
	assert(batch && batch->count >= indexCount);
	
	batch->count -= indexCount;
	
	const PhotonBufferSet *buffers = renderer->buffers;
	PhotonBufferPopElements((PhotonBuffer *)buffers->indexes, 3*triangles);
	PhotonBufferPopElements((PhotonBuffer *)buffers->vertexes, vertexes);
}


//MARK: Custom Draw Command

typedef struct PhotonCustomCommand {
	PhotonCommand command;
	
	const PhotonRenderState *state;
	PhotonUniformBindings uniformBindings;
	
	PhotonCustomCommandFunc func;
	void *context;
} PhotonCustomCommand;

static const PhotonRenderState *
PhotonCustomCommandInvoke(const PhotonCommand *command, PhotonRenderer *renderer, const PhotonRenderState *currentState)
{
	assert(renderer->framebufferBindCommand);
	
	const PhotonCustomCommand *custom = (PhotonCustomCommand *)command;
	const PhotonRenderState *state = custom->state;
	if(state) PhotonRenderStateTransition(currentState, state, &custom->uniformBindings);
	
	custom->func(custom->context);
	PhotonPrintErrors();
	
	renderer->vertexPageBound = NO_PAGE;
	return state ?: currentState;
}

void
PhotonRendererEnqueueCustomCommand(PhotonRenderer *renderer, PhotonRenderState *state, PhotonCustomCommandFunc func, void *context)
{
	PhotonCustomCommand *custom = PhotonRendererPushCommand(renderer, sizeof(*custom), PhotonCustomCommandInvoke);
	custom->state = state;
	custom->func = func;
	custom->context = context;
	
	PhotonRendererResetBatch(renderer);
	
	if(state && state->uniforms){
		BufferUniforms(renderer, state, &custom->uniformBindings);
	}
}


//MARK: Misc. Commands

static const PhotonRenderState *
PhotonRenderTextureBindInvoke(const struct PhotonCommand *command, PhotonRenderer *renderer, const PhotonRenderState *state)
{
	PhotonRenderTextureBindCommand *bind = (PhotonRenderTextureBindCommand *)command;
	
	PhotonRenderTexture *renderTexture = bind->renderTexture;
	if(renderTexture){
		glBindFramebuffer(GL_FRAMEBUFFER, renderTexture->framebufferID);
		
		PhotonTexture *texture = renderTexture->texture;
		glViewport(0, 0, texture->width, texture->height);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		pvec2 size = renderer->defaultFramebufferSize;
		glViewport(0, 0, size.x, size.y);
	}
	
	switch(bind->loadAction){
		case PhotonLoadActionLoad:
		case PhotonLoadActionDontCare:
			break;
		
		case PhotonLoadActionClear:{
			pvec4 color = bind->clearColor;
			glClearColor(color.r, color.g, color.b, color.a);
			glClear(GL_COLOR_BUFFER_BIT);
		} break;
	}
	
	renderer->framebufferBindCommand = bind;
	return state;
}

void
PhotonRendererBindRenderTexture(
	PhotonRenderer *renderer, PhotonRenderTexture *renderTexture,
	PhotonLoadAction loadAction, PhotonStoreAction storeAction, pvec4 clearColor
){
	PhotonRendererResetBatch(renderer);
	
	PhotonRenderTextureBindCommand *bind = PhotonRendererPushCommand(renderer, sizeof(*bind), PhotonRenderTextureBindInvoke);
	bind->renderTexture = renderTexture;
	bind->loadAction = loadAction;
	bind->storeAction = storeAction;
	bind->clearColor = clearColor;
}

static void ScissorPixelRect(paabb2 rect){
	glEnable(GL_SCISSOR_TEST);
	
	rect.min.x = floorf(rect.min.x);
	rect.min.y = floorf(rect.min.y);
	rect.max.x = ceilf(rect.max.x);
	rect.max.y = ceilf(rect.max.y);
	
	pvec2 size = pvec2Sub(rect.max, rect.min);
	glScissor(rect.min.x, rect.min.y, size.x, size.y);
}

static const PhotonRenderState *
PhotonScissorInvoke(const struct PhotonCommand *command, PhotonRenderer *renderer, const PhotonRenderState *state)
{
	PhotonScissorCommand *scissor = (PhotonScissorCommand *)command;
	
	switch(scissor->state){
		case PhotonScissorStateDisable: {
			glDisable(GL_SCISSOR_TEST);
			break;
		}
		
//		case PhotonScissorStateNormalized: {
//			paabb2 rect = scissor->rect;
//			pvec2 viewSize = PhotonRendererGetFramebufferSize(renderer);
//			
//			rect.min.x *= viewSize.x;
//			rect.min.y *= viewSize.y;
//			rect.max.x *= viewSize.x;
//			rect.max.y *= viewSize.y;
//			
//			ScissorPixelRect(rect);
//			break;
//		}
		
		case PhotonScissorStateClipSpace: {
			paabb2 rect = scissor->rect;
			pvec2 viewSize = PhotonRendererGetFramebufferSize(renderer);
			
			rect.min.x = (0.5*rect.min.x + 0.5)*viewSize.x;
			rect.min.y = (0.5*rect.min.y + 0.5)*viewSize.y;
			rect.max.x = (0.5*rect.max.x + 0.5)*viewSize.x;
			rect.max.y = (0.5*rect.max.y + 0.5)*viewSize.y;
			
			ScissorPixelRect(rect);
			break;
		}
		
//		case PhotonScissorStatePixel: {
//			ScissorPixelRect(scissor->rect);
//			break;
//		}
	}
	
	return state;
}

void
PhotonRendererScissor(PhotonRenderer *renderer, PhotonScissorState state, paabb2 rect)
{
	PhotonRendererResetBatch(renderer);
	
	PhotonScissorCommand *cmd = PhotonRendererPushCommand(renderer, sizeof(*cmd), PhotonScissorInvoke);
	cmd->state = state;
	cmd->rect = rect;
}
