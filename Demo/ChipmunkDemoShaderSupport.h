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

#include <stddef.h>

void CheckGLErrors(void);
#define CHECK_GL_ERRORS() CheckGLErrors()

GLint CompileShader(GLenum type, const char *source);
GLint LinkProgram(GLint vshader, GLint fshader);
cpBool ValidateProgram(GLint program);

#define GLSL(x) #x

void SetAttribute(GLuint program, char const *name, GLint size, GLenum gltype, GLsizei stride, GLvoid *offset);

#define SET_ATTRIBUTE(program, type, name, gltype)\
	SetAttribute(program, #name, sizeof(((type *)NULL)->name)/sizeof(GLfloat), gltype, sizeof(type), (GLvoid *)offsetof(type, name))
