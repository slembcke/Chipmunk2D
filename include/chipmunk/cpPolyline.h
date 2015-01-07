// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

// PRO: Polylines are just arrays of vertexes.
// They are looped if the first vertex is equal to the last.
// cpPolyline structs are intended to be passed by value and destroyed when you are done with them.
typedef struct cpPolyline {
  int count, capacity;
  cpVect verts[];
} cpPolyline;

/// PRO: Destroy and free a polyline instance.
void cpPolylineFree(cpPolyline *line);

/// PRO: Returns true if the first vertex is equal to the last.
cpBool cpPolylineIsClosed(cpPolyline *line);

/**
	PRO: Returns a copy of a polyline simplified by using the Douglas-Peucker algorithm.
	This works very well on smooth or gently curved shapes, but not well on straight edged or angular shapes.
*/
cpPolyline *cpPolylineSimplifyCurves(cpPolyline *line, cpFloat tol);

/**
	PRO: Returns a copy of a polyline simplified by discarding "flat" vertexes.
	This works well on straigt edged or angular shapes, not as well on smooth shapes.
*/
cpPolyline *cpPolylineSimplifyVertexes(cpPolyline *line, cpFloat tol);

/// PRO: Get the convex hull of a polyline as a looped polyline.
cpPolyline *cpPolylineToConvexHull(cpPolyline *line, cpFloat tol);


/// PRO: Polyline sets are collections of polylines, generally built by cpMarchSoft() or cpMarchHard().
typedef struct cpPolylineSet {
  int count, capacity;
  cpPolyline **lines;
} cpPolylineSet;

/// PRO: Allocate a new polyline set.
cpPolylineSet *cpPolylineSetAlloc(void);

/// PRO: Initialize a new polyline set.
cpPolylineSet *cpPolylineSetInit(cpPolylineSet *set);

/// PRO: Allocate and initialize a polyline set.
cpPolylineSet *cpPolylineSetNew(void);

/// PRO: Destroy a polyline set.
void cpPolylineSetDestroy(cpPolylineSet *set, cpBool freePolylines);

/// PRO: Destroy and free a polyline set.
void cpPolylineSetFree(cpPolylineSet *set, cpBool freePolylines);

/**
	PRO: Add a line segment to a polyline set.
	A segment will either start a new polyline, join two others, or add to or loop an existing polyline.
	This is mostly intended to be used as a callback directly from cpMarchSoft() or cpMarchHard().
*/
void cpPolylineSetCollectSegment(cpVect v0, cpVect v1, cpPolylineSet *lines);

/**
	PRO: Get an approximate convex decomposition from a polyline.
	Returns a cpPolylineSet of convex hulls that match the original shape to within 'tol'.
	NOTE: I don't consider this function 1.0 worthy for two reasons.
		It doesn't always pick great splitting planes. This can lead to tiny or unnecessary polygons in the output.
		If the input is a self intersecting polygon, the output might end up overly simplified. 
*/

cpPolylineSet *cpPolylineConvexDecomposition_BETA(cpPolyline *line, cpFloat tol);

