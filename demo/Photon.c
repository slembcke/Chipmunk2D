#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "Photon.h"

#if ANDROID

#include <GLES2/gl2.h>

#elif __APPLE__

#import <TargetConditionals.h>

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#elif TARGET_OS_MAC

#include <OpenGL/gl3.h>

#endif

#endif


//MARK: Types

struct PhotonTexture {
	size_t width, height;
	GLuint texture;
};

#define PHOTON_UNIFORM_BLOCK_BINDING_MAX 8
#define PHOTON_UNIFORM_TEXTURE_BINDING_MAX 8
#define PHOTON_NAME_BUFFER_SIZE ((size_t)1024)

struct PhotonShader {
	GLuint program;
	
	size_t uniformBlocksSize;
	const char *uniformBlockNames[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	GLenum uniformBlockTypes[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	GLint uniformBlockLocations[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	size_t uniformBlockSizes[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	size_t uniformBlockOffsets[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	
	const char *uniformTextureNames[PHOTON_UNIFORM_TEXTURE_BINDING_MAX];
	
	char nameBuffer[PHOTON_NAME_BUFFER_SIZE];
};

struct PhotonUniforms {
	const PhotonShader *shader;
	
	const PhotonTexture *textures[PHOTON_UNIFORM_TEXTURE_BINDING_MAX];
	uint8_t blocks[];
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
	PhotonBufferTypeCommand,
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

//typedef struct PhotonBufferGLES2 {
//	struct PhotonBuffer super;
//	GLuint glBuffer;
//} PhotonBufferGLES2;

typedef struct PhotonBufferGL3 {
	struct PhotonBuffer super;
	GLuint glBuffer;
} PhotonBufferGL3;

typedef struct PhotonBufferSet {
	GLuint vao;
	
	PhotonBufferGL3 *vertexes;
	PhotonBufferGL3 *indexes;
	PhotonBufferGL3 *uniforms;
} PhotonBufferSet;

typedef struct PhotonUniformBindings {
	PhotonBufferGL3 *uniformBuffer;
	GLenum types[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	GLuint locations[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	struct {size_t offset, size;} ranges[PHOTON_UNIFORM_BLOCK_BINDING_MAX];
	GLuint textures[PHOTON_UNIFORM_TEXTURE_BINDING_MAX];
} PhotonUniformBindings;

struct PhotonCommand;
typedef const PhotonRenderState *(*PhotonCommandFunc)(const struct PhotonCommand *command, PhotonRenderer *renderer, const PhotonRenderState *state);

typedef struct PhotonCommand {
	size_t commandSize;
	PhotonCommandFunc invoke;
} PhotonCommand;

typedef struct PhotonDrawCommand {
	PhotonCommand super;
	
	const PhotonRenderState *state;
	PhotonUniformBindings uniformBindings;
	
	size_t vertexPage, count, firstIndex;
} PhotonDrawCommand;

typedef struct PhotonScratchBlock {
	struct PhotonScratchBlock *next;
	size_t count, capacity;
	uint8_t block[];
} PhotonScratchBlock;

static size_t NO_PAGE = -1;

struct PhotonRenderer {
	size_t vertexPageBound;
	PhotonBufferSet *buffers;
	
	PhotonBuffer *commandBuffer;
	PhotonScratchBlock *scratchBlock;
	
	PhotonDrawCommand *batchCommand;
	
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

static GLenum TextureFormat[] = {GL_R8, GL_RG8, GL_RGBA8, GL_SRGB8_ALPHA8, GL_RGBA16F};
static GLenum TextureAddressMode[] = {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_REPEAT, GL_MIRRORED_REPEAT};

static GLenum TextureFilters[3][3] = {
	{GL_LINEAR, GL_LINEAR, GL_LINEAR}, // Invalid enum, fall back to linear.
	{GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR}, // nearest
	{GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR}, // linear
};


PhotonTexture *
PhotonTextureNew(size_t width, size_t height, const void *pixelData, const PhotonTextureOptions *options)
{
	options = options ?: &PhotonTextureOptionsDefault;
	
	PhotonTexture *texture = (PhotonTexture *)calloc(1, sizeof(PhotonTexture));
	texture->width = width;
	texture->height = height;
	
	glGenTextures(1, &texture->texture);
	glBindTexture(GL_TEXTURE_2D, texture->texture);
	
	GLenum format = TextureFormat[options->format];
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, format, (GLint)width, (GLint)height, 0, format, GL_UNSIGNED_BYTE, pixelData);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureAddressMode[options->addressX]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureAddressMode[options->addressY]);
	
	GLenum minFilter = TextureFilters[options->minFilter][PhotonTextureFilterMipmapNone];
	GLenum magFilter = TextureFilters[options->magFilter][options->mipFilter];
	assert(minFilter);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, magFilter);
	
	if(options->generateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
		
	glBindTexture(GL_TEXTURE_2D, 0);
	PhotonPrintErrors();
	
	return texture;
}

void
PhotonTextureFree(PhotonTexture *texture)
{
	if(texture){
		glDeleteTextures(1, &texture->texture);
		free(texture);
	}
}

void
PhotonTextureSubImage(PhotonTexture *texture, int x, int y, size_t w, size_t h, size_t stride, const void *pixelData)
{
	glBindTexture(GL_TEXTURE_2D, texture->texture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)stride);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, (GLint)w, (GLint)h, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	glBindTexture(GL_TEXTURE_2D, 0);
	PhotonPrintErrors();
}


//MARK: Shaders

typedef void (* GetShaderivFunc) (GLuint shader, GLenum pname, GLint* param);
typedef void (* GetShaderInfoLogFunc) (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);

static bool
PhotonCheckShaderError(GLuint obj, GLenum status, GetShaderivFunc getiv, GetShaderInfoLogFunc getInfoLog)
{
	GLint success;
	getiv(obj, status, &success);
	
	if(!success){
		GLint length;
		getiv(obj, GL_INFO_LOG_LENGTH, &length);
		
		char *log = (char *)alloca(length);
		getInfoLog(obj, length, NULL, log);
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
	fprintf(stderr, "%s\n", source);
}

static GLuint
CompileShaderSource(GLenum type, const char *source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar *[]){source}, NULL);
	glCompileShader(shader);
	PhotonPrintErrors();
	
	if(PhotonCheckShaderError(shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog)){
		return shader;
	} else {
		PhotonLogShader((type == GL_VERTEX_SHADER) ? "Vertex Shader:" : "Fragment Shader:", source);
		glDeleteShader(shader);
		PhotonPrintErrors();
		
		return 0;
	}
}

static GLuint
CompileShader(const char *vsource, const char *fsource)
{
	GLuint shader = glCreateProgram();
	PhotonPrintErrors();
	
	glBindAttribLocation(shader, PhotonAttributePosition, "PhotonAttributePosition");
	glBindAttribLocation(shader, PhotonAttributeUV1     , "PhotonAttributeUV1"     );
	glBindAttribLocation(shader, PhotonAttributeUV2     , "PhotonAttributeUV2"     );
	glBindAttribLocation(shader, PhotonAttributeColor   , "PhotonAttributeColor"   );
	PhotonPrintErrors();
	
	glBindFragDataLocation(shader, 0, "PhotonFragOut");
	PhotonPrintErrors();
	
	GLint vshader = CompileShaderSource(GL_VERTEX_SHADER, vsource);
	GLint fshader = CompileShaderSource(GL_FRAGMENT_SHADER, fsource);
	
	if(!vshader || !fshader) goto cleanup;
	glAttachShader(shader, vshader);
	glAttachShader(shader, fshader);
	PhotonPrintErrors();
	
	glLinkProgram(shader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	PhotonPrintErrors();
	
	if(PhotonCheckShaderError(shader, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog)){
		return shader;
	} else {
		if(vshader && fshader){
			PhotonLogShader("Vertex Shader"  , vsource);
			PhotonLogShader("Fragment Shader", fsource);
		}
		
		cleanup:
		glDeleteProgram(shader);
		glDeleteShader(vshader);
		glDeleteShader(fshader);
		PhotonPrintErrors();
		
		return 0;
	}
}

static void
PhotonShaderPushUniform(PhotonShader *shader, GLuint *uniformBinding, const char *name, GLenum type, GLint location, size_t size, size_t *nameCursor)
{
	int i = *uniformBinding;
	
	shader->uniformBlockNames[i] = name;
	shader->uniformBlockTypes[i] = type;
	shader->uniformBlockLocations[i] = location;
	shader->uniformBlockSizes[i] = size;
	shader->uniformBlockOffsets[i] = shader->uniformBlocksSize;
	shader->uniformBlocksSize += size;
	
	(*nameCursor) += strlen(name) + 1;
	(*uniformBinding)++;
}

PhotonShader *
PhotonShaderNew(const char *vshader, const char *fshader)
{
	PhotonShader *shader = (PhotonShader *)calloc(1, sizeof(PhotonShader));
	GLuint program = CompileShader(vshader, fshader);
	
	if(program){
		shader->program = program;
		glUseProgram(program);
		
		size_t nameCursor = 0;
		GLuint uniformBinding = 0;
		
		int blocks = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blocks);
		
		for(int i = 0; i < blocks; i++){
			GLchar *name = shader->nameBuffer + nameCursor;
			GLsizei nameLength = 0;
			GLsizei size = 0;
			
			glGetActiveUniformBlockName(program, i, (GLsizei)(PHOTON_NAME_BUFFER_SIZE - nameCursor - 1), &nameLength, name);
			glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, (GLint *)&size);
			
			shader->uniformBlockSizes[i] = size;
			shader->uniformBlockOffsets[uniformBinding] = shader->uniformBlocksSize;
			shader->uniformBlocksSize += size;
			
			shader->uniformBlockNames[uniformBinding] = name;
			nameCursor += nameLength + 1;
			
			glUniformBlockBinding(program, glGetUniformBlockIndex(program, name), uniformBinding);
			uniformBinding++;
			
			PhotonPrintErrors();
		}
		
		int uniforms = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniforms);
		
		GLuint textureBinding = 0;
		for(int i = 0; i < uniforms; i++){
			GLchar *name = shader->nameBuffer + nameCursor;
			GLsizei nameLength = 0;
			GLsizei size = 0;
			GLenum type = 0;
			
			glGetActiveUniform(program, i, (GLsizei)(PHOTON_NAME_BUFFER_SIZE - nameCursor - 1), &nameLength, &size, &type, name);
			
			GLint location = glGetUniformLocation(program, name);
			if(location >= 0){
				switch(type){
					case GL_FLOAT: PhotonShaderPushUniform(shader, &uniformBinding, name, type, location, sizeof(GLfloat), &nameCursor); break;
					case GL_FLOAT_VEC2: PhotonShaderPushUniform(shader, &uniformBinding, name, type, location, 2*sizeof(GLfloat), &nameCursor); break;
					case GL_FLOAT_VEC4: PhotonShaderPushUniform(shader, &uniformBinding, name, type, location, 4*sizeof(GLfloat), &nameCursor); break;
					case GL_FLOAT_MAT4: PhotonShaderPushUniform(shader, &uniformBinding, name, type, location, 16*sizeof(GLfloat), &nameCursor); break;
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
		glDeleteShader(shader->program);
		PhotonPrintErrors();
		
		free(shader);
	}
}


//MARK: Uniforms

static size_t
PhotonUniformsAllocSize(const PhotonShader *shader)
{
	assert(shader);
	return sizeof(PhotonUniforms) + shader->uniformBlocksSize;
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
PhotonUniformsSetBlock(PhotonUniforms *uniforms, const char *name, const void *ptr)
{
	const PhotonShader *shader = uniforms->shader;
	int i = IndexForName(name, shader->uniformBlockNames, PHOTON_UNIFORM_BLOCK_BINDING_MAX);
	
	if(i >= 0){
		memcpy(uniforms->blocks + shader->uniformBlockOffsets[i], ptr, shader->uniformBlockSizes[i]);
	}
}

void
PhotonUniformsSetTexture(PhotonUniforms *uniforms, const char *name, const PhotonTexture *texture)
{
	const PhotonShader *shader = uniforms->shader;
	int i = IndexForName(name, shader->uniformTextureNames, PHOTON_UNIFORM_TEXTURE_BINDING_MAX);
	
	if(i >= 0){
		uniforms->textures[i] = texture;
	}
}


//MARK: RenderStates

static PhotonRenderState *
PhotonRenderStateAlloc()
{
	return (PhotonRenderState *)calloc(1, sizeof(PhotonRenderState));
}

static PhotonRenderState *
PhotonRenderStateInit(PhotonRenderState *state, const PhotonBlendMode *blendMode, const PhotonShader *shader, const PhotonUniforms *uniforms)
{
	assert(shader);
	
	state->blendMode = blendMode;
	state->shader = shader;
	state->uniforms = uniforms;
	
	return state;
}

PhotonRenderState *
PhotonRenderStateNew(const PhotonBlendMode *blendMode, const PhotonShader *shader, const PhotonUniforms *uniforms)
{
	return PhotonRenderStateInit(PhotonRenderStateAlloc(), blendMode, shader, uniforms);
}

void
PhotonRenderStateFree(PhotonRenderState *state)
{
	if(state){
		free(state);
	}
}

static void *
PhotonUniformBindingsGetPtr(const PhotonUniformBindings *uniformBindings, int index, size_t size){
	assert(uniformBindings->ranges[index].size == size);
	return (uniformBindings->uniformBuffer->super.ptr + uniformBindings->ranges[index].offset);
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
	
	const PhotonShader *shader = destinationState->shader;
	if(currentState->shader != shader){
		glUseProgram(shader->program);
		
//		glValidateProgram(shader);
//		GLint validate = 0; glGetProgramiv(shader, GL_VALIDATE_STATUS, &validate);
//		if(!validate){
//			fprintf(stderr, "GL Error: Shader failed to validate.");
//		}
		
		PhotonPrintErrors();
	}
	
	if(currentState->uniforms != destinationState->uniforms){
		for(int i = 0; i < PHOTON_UNIFORM_BLOCK_BINDING_MAX; i++){
			size_t size = uniformBindings->ranges[i].size;
			
			// Break for unused uniforms.
			if(size == 0) break;
			
			GLuint location = uniformBindings->locations[i];
			switch(uniformBindings->types[i]){
				case 0: glBindBufferRange(GL_UNIFORM_BUFFER, i, uniformBindings->uniformBuffer->glBuffer, uniformBindings->ranges[i].offset, size); break;
				case GL_FLOAT: glUniform1fv(location, 1, PhotonUniformBindingsGetPtr(uniformBindings, i, sizeof(GLfloat))); break;
				case GL_FLOAT_VEC2: glUniform2fv(location, 1, PhotonUniformBindingsGetPtr(uniformBindings, i, 2*sizeof(GLfloat))); break;
				case GL_FLOAT_VEC4: glUniform4fv(location, 1, PhotonUniformBindingsGetPtr(uniformBindings, i, 4*sizeof(GLfloat))); break;
				case GL_FLOAT_MAT4: glUniformMatrix4fv(location, 1, GL_FALSE, PhotonUniformBindingsGetPtr(uniformBindings, i, 16*sizeof(GLfloat))); break;
				default: break;
			}
			
			PhotonPrintErrors();
		}
		
		for(int i = 0; i < PHOTON_UNIFORM_TEXTURE_BINDING_MAX; i++){
			GLuint texture = uniformBindings->textures[i];
			if(texture){
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, texture);
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
	0, // Command buffer
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

//MARK: Basic Buffer

static void
PhotonBufferBasicDestroy(PhotonBuffer *buffer)
{
	free(buffer->ptr); buffer->ptr = NULL;
}

static void
PhotonBufferBasicResize(PhotonBuffer *buffer, size_t capacity)
{
	buffer->ptr = realloc(buffer->ptr, capacity*buffer->elementSize);
	buffer->capacity = capacity;
}

static PhotonBuffer *
PhotonBufferBasicNew(size_t capacity, size_t elementSize, enum PhotonBufferType type)
{
	PhotonBuffer *buffer = (PhotonBuffer *)calloc(1, sizeof(PhotonBuffer));
	PhotonBufferInit(buffer, capacity, elementSize, type);
	buffer->resize = PhotonBufferBasicResize;
	buffer->destroy = (PhotonBufferDestroyFunc)PhotonBufferBasicDestroy;
	
	buffer->ptr = calloc(buffer->capacity, buffer->elementSize);
	
	return buffer;
}

////MARK: GLES2 Buffer
//
//static void
//PhotonBufferGLES2Destroy(PhotonBufferGLES2 *buffer)
//{
//	glDeleteBuffers(1, &buffer->glBuffer);
//	buffer->glBuffer = 0;
//	PhotonPrintErrors();
//}
//
//static void
//PhotonBufferGLES2Commit(PhotonBufferGLES2 *buffer)
//{
//	GLenum target = BufferTypes[buffer->super.type];
//	
//	glBindBuffer(target, buffer->glBuffer);
//	glBufferData(target, buffer->super.count*buffer->super.elementSize, buffer->super.ptr, GL_STATIC_DRAW);
//	glBindBuffer(target, 0);
//	PhotonPrintErrors();
//}
//
//static PhotonBufferGLES2 *
//PhotonBufferGLES2New(size_t capacity, size_t elementSize, enum PhotonBufferType type){
//	PhotonBufferGLES2 *buffer = (PhotonBufferGLES2 *)calloc(1, sizeof(PhotonBufferGLES2));
//	PhotonBufferInit((PhotonBuffer *)buffer, capacity, elementSize, type);
//	buffer->super.resize = PhotonBufferBasicResize;
//	buffer->super.destroy = (PhotonBufferDestroyFunc)PhotonBufferGLES2Destroy;
//	buffer->super.commit = (PhotonBufferCommitFunc)PhotonBufferGLES2Commit;
//	
//	buffer->super.ptr = calloc(buffer->super.capacity, buffer->super.elementSize);
//	
//	GLenum target = BufferTypes[type];
//	assert(target);
//	
//	glGenBuffers(1, &buffer->glBuffer);
//	
//	PhotonPrintErrors();
//	return buffer;
//}



//MARK: GL3 Buffer

static void
PhotonBufferGL3Destroy(PhotonBufferGL3 *buffer)
{
	glDeleteBuffers(1, &buffer->glBuffer);
	buffer->glBuffer = 0;
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
	glBindBuffer(target, buffer->glBuffer);
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
	
	glBindBuffer(target, buffer->glBuffer);
	buffer->super.ptr = glMapBufferRange(target, 0, buffer->super.capacity*buffer->super.elementSize, BUFFER_ACCESS_WRITE);
	glBindBuffer(target, 0);
	PhotonPrintErrors();
}

static void
PhotonBufferGL3Commit(PhotonBufferGL3 *buffer)
{
	GLenum target = BufferTypes[buffer->super.type];
	
	glBindBuffer(target, buffer->glBuffer);
	glFlushMappedBufferRange(target, 0, buffer->super.count*buffer->super.elementSize);
	glUnmapBuffer(target);
	glBindBuffer(target, 0);
	PhotonPrintErrors();
	
	buffer->super.ptr = NULL;
}

static PhotonBufferGL3 *
PhotonBufferGL3New(size_t capacity, size_t elementSize, enum PhotonBufferType type){
	PhotonBufferGL3 *buffer = (PhotonBufferGL3 *)calloc(1, sizeof(PhotonBufferGL3));
	PhotonBufferInit((PhotonBuffer *)buffer, capacity, elementSize, type);
	buffer->super.destroy = (PhotonBufferDestroyFunc)PhotonBufferGL3Destroy;
	buffer->super.resize = (PhotonBufferResizeFunc)PhotonBufferGL3Resize;
	buffer->super.prepare = (PhotonBufferCommitFunc)PhotonBufferGL3Prepare;
	buffer->super.commit = (PhotonBufferCommitFunc)PhotonBufferGL3Commit;
	
	GLenum target = BufferTypes[type];
	assert(target);
	
	glGenBuffers(1, &buffer->glBuffer);
	glBindBuffer(target, buffer->glBuffer);
	glBufferData(target, buffer->super.capacity*buffer->super.elementSize, NULL, GL_DYNAMIC_DRAW);
	
	PhotonPrintErrors();
	return buffer;
}


//MARK: Buffer Set

static void
PhotonBufferSetBind(const PhotonBufferSet *buffers, size_t vertexPage)
{
	if(buffers){
		glBindVertexArray(buffers->vao);
	} else {
		return;
	}
	
	if(vertexPage == NO_PAGE){
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, buffers->vertexes->glBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers->indexes->glBuffer);
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
	PhotonBufferSet *buffers = (PhotonBufferSet *)calloc(1, sizeof(PhotonBufferSet));
	
	buffers->vertexes = PhotonBufferGL3New(vertexCapacity , sizeof(PhotonVertex), PhotonBufferTypeVertex );
	buffers->indexes  = PhotonBufferGL3New(indexCapacity  , sizeof(PhotonIndex ), PhotonBufferTypeIndex  );
	buffers->uniforms = PhotonBufferGL3New(uniformCapacity, 1                   , PhotonBufferTypeUniform);
	
	glGenVertexArrays(1, &buffers->vao);
	PhotonBufferSetBind(buffers, 0);
	
	PhotonBufferSetBind(NULL, 0);
	PhotonPrintErrors();
	
	return buffers;
}

static void
PhotonBufferSetFree(PhotonBufferSet *buffers)
{
	if(buffers){
		glDeleteVertexArrays(1, &buffers->vao);
		
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


//MARK: Renderer

PhotonRenderer *
PhotonRendererNew(void)
{
	PhotonRenderer *renderer = (PhotonRenderer *)calloc(1, sizeof(PhotonRenderer));
	
	renderer->buffers = PhotonBufferSetNew(1024, 1024, 32*1024);
	renderer->commandBuffer = PhotonBufferBasicNew(1024, 1, PhotonBufferTypeCommand);
	
	return renderer;
}

void
PhotonRendererFree(PhotonRenderer *renderer)
{
	if(renderer){
		PhotonBufferSetFree(renderer->buffers);
		PhotonBufferFree(renderer->commandBuffer);
		
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

static PhotonScratchBlock *
AllocScratchBlock(size_t size, PhotonScratchBlock *next)
{
	PhotonScratchBlock *scratch = calloc(1, size + sizeof(PhotonScratchBlock));
	scratch->next = next;
	scratch->count = 0;
	scratch->capacity = size;
	
	return scratch;
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
PhotonRendererTemporaryRenderState(PhotonRenderer *renderer, const PhotonBlendMode *blendMode, const PhotonShader *shader, const PhotonUniforms *uniforms)
{
	return PhotonRenderStateInit(PhotonRendererTemporaryMemory(renderer, sizeof(PhotonRenderState)), blendMode, shader, uniforms);
}

static void
FreeScratchBlocks(PhotonScratchBlock *scratch)
{
	if(scratch){
		FreeScratchBlocks(scratch->next);
		free(scratch);
	}
}

void
PhotonRendererResetBatch(PhotonRenderer *renderer)
{
	renderer->batchCommand = NULL;
}

void
PhotonRendererPrepare(PhotonRenderer *renderer)
{
	PhotonScratchBlock *scratch = renderer->scratchBlock;
	if(scratch){
		// Free all but the largest scratch block.
		FreeScratchBlocks(scratch->next);
		
		// Reset the largest.
		scratch->next = NULL;
		scratch->count = 0;
	}
	
	PhotonRendererBindBuffers(renderer, NO_PAGE);
	PhotonBufferPrepare(renderer->commandBuffer);
	PhotonBufferSetPrepareForAccess(renderer->buffers);
	
	PhotonRendererResetBatch(renderer);
}

void
PhotonRendererFlush(PhotonRenderer *renderer)
{
	PhotonBufferSetPrepareForRendering(renderer->buffers);
	
	const PhotonRenderState *currentState = NULL;
	PhotonBuffer *commands = renderer->commandBuffer;
	for(size_t cursor = 0; cursor < commands->count;){
		PhotonCommand *command = (PhotonCommand *)((intptr_t)commands->ptr + cursor);
		currentState = command->invoke(command, renderer, currentState);
		
		cursor += command->commandSize;
	}
	
	PhotonBufferCommit(commands); 
	
	renderer->fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	PhotonPrintErrors();
}

bool
PhotonRendererReady(PhotonRenderer *renderer)
{
	if(renderer->fence){
		if(glClientWaitSync(renderer->fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0) == GL_ALREADY_SIGNALED){
			// Fence has completed. Clean it up.
			glDeleteSync(renderer->fence);
			renderer->fence = NULL;
		}
		
		PhotonPrintErrors();
	}
	
	// Renderer is not ready as long as the fence still exists.
	return (renderer->fence == NULL);
}


//MARK: Photon Commands

static void
PhotonCommandInit(PhotonCommand *command, size_t commandSize, PhotonCommandFunc commandFunc)
{
	assert(command);
	bzero(command, commandSize);
	
	command->commandSize = commandSize;
	command->invoke = commandFunc;
}

static void
CopyUniformsAndBindings(const PhotonRenderState *state, PhotonBufferGL3 *uniformBuffer, PhotonUniformBindings *uniformBindings)
{
	uniformBindings->uniformBuffer = uniformBuffer;
	for(int i = 0; i < PHOTON_UNIFORM_BLOCK_BINDING_MAX; i++){
		size_t size = state->shader->uniformBlockSizes[i];
		if(size == 0) break;
		
		uniformBindings->types[i] = state->shader->uniformBlockTypes[i];
		uniformBindings->locations[i] = state->shader->uniformBlockLocations[i];
		uniformBindings->ranges[i].offset = uniformBuffer->super.count;
		uniformBindings->ranges[i].size = size;
		
		void *block = PhotonBufferPushElements((PhotonBuffer *)uniformBuffer, size);
		memcpy(block, state->uniforms->blocks + state->shader->uniformBlockOffsets[i], size);
	}

	for(int i = 0; i < PHOTON_UNIFORM_BLOCK_BINDING_MAX; i++){
		const PhotonTexture *texture = state->uniforms->textures[i];
		if(texture == NULL) break;
		
		uniformBindings->textures[i] = texture->texture;
	}
}


//MARK: Draw Command

static const PhotonRenderState *
PhotonDrawCommandInvoke(const PhotonCommand *command, PhotonRenderer *renderer, const PhotonRenderState *currentState)
{
	const PhotonDrawCommand *draw = (PhotonDrawCommand *)command;
	const PhotonRenderState *state = draw->state;
	PhotonRenderStateTransition(currentState, state, &draw->uniformBindings);
	
	PhotonRendererBindBuffers(renderer, draw->vertexPage);
	glDrawElements(GL_TRIANGLES, (GLsizei)draw->count, GL_UNSIGNED_SHORT, (GLvoid *)(draw->firstIndex*sizeof(PhotonIndex)));
	PhotonPrintErrors();
	
	return state;
}

static void
PhotonDrawCommandInit(PhotonDrawCommand *draw, const PhotonRenderState *state, size_t count, size_t vertexPage, size_t firstIndex)
{
	PhotonCommandInit((PhotonCommand *)draw, sizeof(PhotonDrawCommand), PhotonDrawCommandInvoke);
	draw->state = state;
	draw->count = count;
	draw->vertexPage = vertexPage;
	draw->firstIndex = firstIndex;
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
		PhotonDrawCommand *draw = PhotonBufferPushElements(renderer->commandBuffer, sizeof(PhotonDrawCommand));
		PhotonDrawCommandInit(draw, state, indexCount, vertexPage, firstIndex);
		
		renderer->batchCommand = draw;
		
		if(state->uniforms){
			CopyUniformsAndBindings(state, buffers->uniforms, &draw->uniformBindings);
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
	const PhotonCustomCommand *custom = (PhotonCustomCommand *)command;
	const PhotonRenderState *state = custom->state;
	if(state) PhotonRenderStateTransition(currentState, state, &custom->uniformBindings);
	
	custom->func(custom->context);
	PhotonPrintErrors();
	
	renderer->vertexPageBound = NO_PAGE;
	return state ?: currentState;
}

static void
PhotonCustomCommandInit(PhotonCustomCommand *custom, const PhotonRenderState *state, PhotonCustomCommandFunc func, void *context)
{
	PhotonCommandInit((PhotonCommand *)custom, sizeof(PhotonCustomCommand), PhotonCustomCommandInvoke);
	custom->state = state;
	custom->func = func;
	custom->context = context;
}

void
PhotonRendererEnqueueCustomCommand(PhotonRenderer *renderer, PhotonRenderState *state, PhotonCustomCommandFunc func, void *context)
{
	PhotonCustomCommand *custom = PhotonBufferPushElements(renderer->commandBuffer, sizeof(PhotonCustomCommand));
	PhotonCustomCommandInit(custom, state, func, context);
	
	PhotonRendererResetBatch(renderer);
	
	if(state && state->uniforms){
		CopyUniformsAndBindings(state, renderer->buffers->uniforms, &custom->uniformBindings);
	}
}
