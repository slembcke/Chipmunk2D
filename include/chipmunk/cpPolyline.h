/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
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

#pragma once

// Polylines are just arrays of vertexes.
// They are looped if the first vertex is equal to the last.
// cpPolyline structs are intended to be passed by value and destroyed when you are done with them.
typedef struct cpPolyline {
  int count, capacity;
  cpVect verts[];
} cpPolyline;

/// Destroy and free a polyline instance.
CP_EXPORT void cpPolylineFree(cpPolyline *line);

/// Returns true if the first vertex is equal to the last.
CP_EXPORT bool cpPolylineIsClosed(cpPolyline *line);

/**
	Returns a copy of a polyline simplified by using the Douglas-Peucker algorithm.
	This works very well on smooth or gently curved shapes, but not well on straight edged or angular shapes.
*/
CP_EXPORT cpPolyline *cpPolylineSimplifyCurves(cpPolyline *line, cpFloat tol);

/**
	Returns a copy of a polyline simplified by discarding "flat" vertexes.
	This works well on straigt edged or angular shapes, not as well on smooth shapes.
*/
CP_EXPORT cpPolyline *cpPolylineSimplifyVertexes(cpPolyline *line, cpFloat tol);

/// Get the convex hull of a polyline as a looped polyline.
CP_EXPORT cpPolyline *cpPolylineToConvexHull(cpPolyline *line, cpFloat tol);


/// Polyline sets are collections of polylines, generally built by cpMarchSoft() or cpMarchHard().
typedef struct cpPolylineSet {
  int count, capacity;
  cpPolyline **lines;
} cpPolylineSet;

/// Allocate a new polyline set.
CP_EXPORT cpPolylineSet *cpPolylineSetAlloc(void);

/// Initialize a new polyline set.
CP_EXPORT cpPolylineSet *cpPolylineSetInit(cpPolylineSet *set);

/// Allocate and initialize a polyline set.
CP_EXPORT cpPolylineSet *cpPolylineSetNew(void);

/// Destroy a polyline set.
CP_EXPORT void cpPolylineSetDestroy(cpPolylineSet *set, bool freePolylines);

/// Destroy and free a polyline set.
CP_EXPORT void cpPolylineSetFree(cpPolylineSet *set, bool freePolylines);

/**
	Add a line segment to a polyline set.
	A segment will either start a new polyline, join two others, or add to or loop an existing polyline.
	This is mostly intended to be used as a callback directly from cpMarchSoft() or cpMarchHard().
*/
CP_EXPORT void cpPolylineSetCollectSegment(cpVect v0, cpVect v1, cpPolylineSet *lines);

/**
	Get an approximate convex decomposition from a polyline.
	Returns a cpPolylineSet of convex hulls that match the original shape to within 'tol'.
	NOTE: If the input is a self intersecting polygon, the output might end up overly simplified.
*/

CP_EXPORT cpPolylineSet *cpPolylineConvexDecomposition(cpPolyline *line, cpFloat tol);

#define cpPolylineConvexDecomposition_BETA cpPolylineConvexDecomposition
