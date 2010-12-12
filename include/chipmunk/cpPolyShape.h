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

// Axis structure used by cpPolyShape.
typedef struct cpPolyShapeAxis {
	// normal
	cpVect n;
	// distance from origin
	cpFloat d;
} cpPolyShapeAxis;

// Convex polygon shape structure.
typedef struct cpPolyShape {
	CP_PRIVATE(cpShape shape);
	
	// Vertex and axis lists.
	CP_PRIVATE(int numVerts);
	CP_PRIVATE(cpVect *verts);
	CP_PRIVATE(cpPolyShapeAxis *axes);

	// Transformed vertex and axis lists.
	CP_PRIVATE(cpVect *tVerts);
	CP_PRIVATE(cpPolyShapeAxis *tAxes);
} cpPolyShape;

// Basic allocation functions.
cpPolyShape *cpPolyShapeAlloc(void);
cpPolyShape *cpPolyShapeInit(cpPolyShape *poly, cpBody *body, int numVerts, cpVect *verts, cpVect offset);
cpShape *cpPolyShapeNew(cpBody *body, int numVerts, cpVect *verts, cpVect offset);

cpPolyShape *cpBoxShapeInit(cpPolyShape *poly, cpBody *body, cpFloat width, cpFloat height);
cpShape *cpBoxShapeNew(cpBody *body, cpFloat width, cpFloat height);

// Check that a set of vertexes has a correct winding and that they are convex
cpBool cpPolyValidate(const cpVect *verts, const int numVerts);

int cpPolyShapeGetNumVerts(cpShape *shape);
cpVect cpPolyShapeGetVert(cpShape *shape, int idx);
