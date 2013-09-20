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
#include <stdio.h>

#include "GL/glew.h"
#include "GL/glfw.h"

#include "chipmunk/chipmunk.h"

#include "ChipmunkDemoShaderSupport.h"

void
CheckGLErrors(void)
{
	for(GLenum err = glGetError(); err; err = glGetError()){
		if(err){
			fprintf(stderr, "GLError(%s:%d) 0x%04X\n", __FILE__, __LINE__, err);
			abort();
		}
	}
}

//typedef GLAPIENTRY void (*GETIV)(GLuint shader, GLenum pname, GLint *params);
//typedef GLAPIENTRY void (*GETINFOLOG)(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);

static cpBool
CheckError(GLint obj, GLenum status, PFNGLGETSHADERIVPROC getiv, PFNGLGETSHADERINFOLOGPROC getInfoLog)
{
	GLint success;
	getiv(obj, status, &success);
	
	if(!success){
		GLint length;
		getiv(obj, GL_INFO_LOG_LENGTH, &length);
		
		char *log = (char *)alloca(length);
		getInfoLog(obj, length, NULL, log);
		
		fprintf(stderr, "Shader compile error for 0x%04X: %s\n", status, log);
		return cpFalse;
	} else {
		return cpTrue;
	}
}

GLint
CompileShader(GLenum type, const char *source)
{
	GLint shader = glCreateShader(type);
	
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	
	// TODO: return placeholder shader instead?
	cpAssertHard(CheckError(shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog), "Error compiling shader");
	
	return shader;
}

GLint
LinkProgram(GLint vshader, GLint fshader)
{
	GLint program = glCreateProgram();
	
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	
	// todo return placeholder program instead?
	cpAssertHard(CheckError(program, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog), "Error linking shader program");
	
	return program;
}

cpBool
ValidateProgram(GLint program)
{
	// TODO: Implement?
	return cpTrue;
}

void
SetAttribute(GLuint program, char *name, GLint size, GLenum gltype, GLsizei stride, GLvoid *offset)
{
	GLint index = glGetAttribLocation(program, name);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, gltype, GL_FALSE, stride, offset);
}
