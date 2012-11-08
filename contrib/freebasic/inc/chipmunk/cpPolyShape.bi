/' Copyright (c) 2007 Scott Lembcke
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
 '/



''/ @defgroup cpPolyShape cpPolyShape

''/ @{



''/ @private

type cpSplittingPlane

	as cpVect n

	as cpFloat d

end type



''/ @private

type cpPolyShape

	as cpShape shape

	as integer numVerts

	as cpVect ptr verts, tVerts

	as cpSplittingPlane ptr planes, tPlanes

end type



''/ Allocate a polygon shape.

declare function cpPolyShapeAlloc() as cpPolyShape ptr

''/ Initialize a polygon shape.

''/ A convex hull will be created from the vertexes.

declare function cpPolyShapeInit(byval poly as cpPolyShape ptr, byval body as cpBody ptr, byval numVerts as integer, byval verts as const cpVect ptr, byval offset as cpVect) as cpPolyShape ptr

''/ Allocate and initialize a polygon shape.

''/ A convex hull will be created from the vertexes.

declare function cpPolyShapeNew(byval body as cpBody ptr, byval numVerts as integer, byval verts as cpVect ptr, byval offset as cpVect) as cpShape ptr



''/ Initialize a box shaped polygon shape.

declare function cpBoxShapeInit(byval poly as cpPolyShape ptr, byval body as cpBody ptr, byval width as cpFloat, byval height as cpFloat) as cpPolyShape ptr

''/ Initialize an offset box shaped polygon shape.

declare function cpBoxShapeInit2(byval poly as cpPolyShape ptr, byval body as cpBody ptr, byval box as cpBB) as cpPolyShape ptr

''/ Allocate and initialize a box shaped polygon shape.

declare function cpBoxShapeNew(byval body as cpBody ptr, byval width as cpFloat, byval height as cpFloat) as cpShape ptr

''/ Allocate and initialize an offset box shaped polygon shape.

declare function cpBoxShapeNew2(byval body as cpBody ptr, byval box as cpBB) as cpShape ptr



''/ Check that a set of vertexes is convex and has a clockwise winding.

''/ NOTE: Due to floating point precision issues, hulls created with cpQuickHull() are not guaranteed to validate!

declare function cpPolyValidate(byval verts as const cpVect ptr, byval numVerts as const integer) as cpBool



''/ Get the number of verts in a polygon shape.

declare function cpPolyShapeGetNumVerts(byval shape as cpShape ptr) as integer

''/ Get the @c ith vertex of a polygon shape.

declare function cpPolyShapeGetVert(byval shape as cpShape ptr, byval idx as integer) as cpVect



''/ @}

