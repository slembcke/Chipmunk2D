#include "SimpleTestCase.h"

#include "ObjectiveChipmunk.h"
#include "ChipmunkAutoGeometry.h"

@interface ConvexTest : SimpleTestCase {}
@end

@implementation ConvexTest

//MARK: QuickHull tests

static inline cpFloat frand(void){return (cpFloat)rand()/(cpFloat)RAND_MAX;}

static int
FirstExpected(int expectedCount, cpVect *expectedVerts, cpVect *verts)
{
	for(int i=0; i<expectedCount; i++){
		if(cpveql(verts[0], expectedVerts[i])){
			return i;
		}
	}
	
	return -1;
}

static void
AssertExpected(id self, cpVect *verts, int expectedCount, cpVect *expectedVerts)
{
	int firstExpected = FirstExpected(expectedCount, expectedVerts, verts);
	GHAssertNotEquals(firstExpected, -1, nil);
	
	// Check the verts
	for(int i=0; i<expectedCount; i++){
		GHAssertEquals(verts[i], expectedVerts[(firstExpected + i)%expectedCount], nil);
	}
}

static void
AssertHullsEqual(id self, int resultCount, cpVect *resultVerts)
{
	// Check that running the convex hull again returns the same value
	cpVect result2Verts[resultCount];
	int result2Count = cpConvexHull(resultCount, resultVerts, result2Verts, NULL, 0.0);
	
	GHAssertEquals(resultCount, result2Count, nil);
	for(int i=0; i<resultCount; i++){
		GHAssertEquals(resultVerts[i], result2Verts[i], nil);
	}
}

static void
AssertHull(id self, int count, cpVect *verts, int expectedCount, cpVect *expectedVerts)
{
	// Rotate the input array all the way around to check for index issues when selecting maximal points.
	for(int rotate=0; rotate<count; rotate++){
		cpVect rotated[count];
		for(int i=0; i<count; i++){
			rotated[i] = verts[(i + rotate)%count];
		}
		
		int first = -1;
		cpVect resultVerts[count];
		int resultCount = cpConvexHull(count, rotated, resultVerts, &first, 0.0);
		
		// Check the count
		GHAssertEquals(resultCount, expectedCount, nil);
		
		// Check that the windings are positive.
		GHAssertGreaterThanOrEqual(cpAreaForPoly(expectedCount, expectedVerts), (cpFloat)0, nil);
		GHAssertGreaterThanOrEqual(cpAreaForPoly(resultCount, resultVerts), (cpFloat)0, nil);
		GHAssertTrue(cpPolyValidate(resultVerts, resultCount), nil);
		
		GHAssertEquals(resultVerts[0], rotated[first], nil);
		
		AssertExpected(self, resultVerts, expectedCount, expectedVerts);
		AssertHullsEqual(self, resultCount, resultVerts);
	}
}

static void
AssertRandomHull(id self, int count)
{
	cpVect verts[count];
	for(int i=0; i<count; i++){
		verts[i] = cpv(frand(), frand());
	}
	
	cpVect resultVerts[count];
	int resultCount = cpConvexHull(count, verts, resultVerts, NULL, 0.0);
	
	AssertHullsEqual(self, resultCount, resultVerts);
	GHAssertGreaterThanOrEqual(cpAreaForPoly(resultCount, resultVerts), (cpFloat)0, nil);
	GHAssertTrue(cpPolyValidate(resultVerts, resultCount), nil);
}

-(void)testConvexHull
{
	{
		// Trivial cases
		cpVect verts[] = {{0,0}, {1,0}};
		cpVect expected[] = {{0,0}, {1,0}};
		
		AssertHull(self, 1, verts, 1, expected);
		AssertHull(self, 2, verts, 2, expected);
	}
	
	{
		// Triangle with reversed winding.
		cpVect verts[] = {{0,0}, {1,0}, {0,1}};
		cpVect expected[] = {{0,0}, {0,1}, {1,0}};
		
		AssertHull(self, 3, verts, 3, expected);
	}
	
	{
		// Unit square with reversed winding.
		cpVect verts[] = {{1,0}, {1,1}, {0,1}, {0,0}};
		cpVect expected[] = {{0,0}, {0,1}, {1,1}, {1,0}};
		
		AssertHull(self, 4, verts, 4, expected);
	}
	
	{
		// Degenerate hourglass shaped square.
		cpVect verts[] = {{0,0}, {1,0}, {0,1}, {1,1}};
		cpVect expected[] = {{0,0}, {0,1}, {1,1}, {1,0}};
		
		AssertHull(self, 4, verts, 4, expected);
	}
	
	{
		// A unit square and a bunch of random vertexes inside it.
		cpVect verts[] = {
			{0.9, 0.5}, {0.2, 0.3}, {1.0, 0.0}, {0.8, 0.6}, {0.8, 0.4}, {0.1, 0.4},
			{0.4, 0.1}, {0.0, 0.0}, {0.1, 0.9}, {0.9, 0.6}, {0.6, 0.2}, {0.7, 0.3},
			{0.1, 0.3}, {0.1, 0.5}, {0.8, 0.3}, {0.3, 0.8}, {0.0, 1.0}, {0.6, 0.7},
			{1.0, 1.0}, {0.6, 0.8}, {0.1, 0.7}, {0.7, 0.9}, {0.9, 0.2}, {0.1, 0.1}
		};
		cpVect expected[] = {{0,0}, {0,1}, {1,1}, {1,0}};
		
		AssertHull(self, 24, verts, 4, expected);
	}
	
	{
		// A bunch of colinear points on X
		cpVect verts[] = {{0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}};
		cpVect expected[] = {{0,0}, {0,5}};
		
		AssertHull(self, 6, verts, 2, expected);
	}
	
	{
		// A bunch of colinear points on Y
		cpVect verts[] = {{0,0}, {1,0}, {2,0}, {3,0}, {4,0}, {5,0}};
		cpVect expected[] = {{0,0}, {5,0}};
		
		AssertHull(self, 6, verts, 2, expected);
	}
	
	{
		// A bunch of colinear points on XY
		cpVect verts[] = {{0,0}, {1,1}, {2,2}, {3,3}, {4,4}, {5,5}};
		cpVect expected[] = {{0,0}, {5,5}};
		
		AssertHull(self, 6, verts, 2, expected);
	}
	
	{
		// A bunch of equivalent points
		cpVect verts[] = {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}};
		cpVect expected[] = {{0,0}};
		
		AssertHull(self, 1, verts, 1, expected);
		AssertHull(self, 2, verts, 1, expected);
		AssertHull(self, 3, verts, 1, expected);
		AssertHull(self, 4, verts, 1, expected);
		AssertHull(self, 5, verts, 1, expected);
	}
	
	{
		// Unit square with reversed winding and colinear face points.
		cpVect verts[] = {
			{1.0f, 0.0f}, {1.0f, 0.5f}, {1.0f, 1.0f}, {0.5f, 1.0f},
			{0.0f, 1.0f}, {0.0f, 0.5f}, {0.0f, 0.0f}, {0.5f, 0.0f},
		};
		cpVect expected[] = {{0,0}, {0,1}, {1,1}, {1,0}};
		
		AssertHull(self, 8, verts, 4, expected);
	}
	
	{
		// Unit square with duplicate vertexes.
		cpVect verts1[] = {{0,0}, {0,1}, {1,1}, {1,0}, {0,0}, {0,1}, {1,1}, {1,0}};
		cpVect verts2[] = {{0,0}, {0,0}, {0,1}, {0,1}, {1,1}, {1,1}, {1,0}, {1,0}};
		cpVect expected[] = {{0,0}, {0,1}, {1,1}, {1,0}};
		
		AssertHull(self, 8, verts1, 4, expected);
		AssertHull(self, 8, verts2, 4, expected);
	}
	
	// Check that a convex hull returns itself using (seeded) random hulls.
	srand(5594);
	AssertRandomHull(self, 100);
	AssertRandomHull(self, 1000);
	AssertRandomHull(self, 10000);
	
	// TODO Need tolerance tests.
}

static cpPolyline
MakeLoopedPolyline(int count, cpVect *verts)
{
	int capacity = count + 1;
	cpPolyline line = {capacity, capacity, cpcalloc(capacity, sizeof(cpVect))};
	memcpy(line.verts, verts, count*sizeof(cpVect));
	line.verts[count] = verts[0];
	
	return line;
}

static void
AssertPolyline(id self, cpPolyline line, int expectedCount, cpVect *expectedVerts)
{
	GHAssertEquals(line.count, expectedCount + 1, nil);
	GHAssertTrue(cpPolylineIsLooped(line), nil);
	AssertExpected(self, line.verts, expectedCount, expectedVerts);
}

-(void)testConvexDecomposition
{
	{
		cpVect verts[] = {{0,0}};
		
		cpPolyline line = MakeLoopedPolyline(1, verts);
		cpPolylineSet *set = cpPolylineConvexDecomposition_BETA(line, 0.0);
		
		GHAssertEquals(set->count, 1, nil);
		AssertPolyline(self, set->lines[0], 1, verts);
	}
	
	{
		cpVect verts[] = {{0,0}, {1,0}};
		
		cpPolyline line = MakeLoopedPolyline(2, verts);
		cpPolylineSet *set = cpPolylineConvexDecomposition_BETA(line, 0.0);
		
		GHAssertEquals(set->count, 1, nil);
		AssertPolyline(self, set->lines[0], 2, verts);
	}
	
	{
		cpVect verts[] = {{0,0}, {0,1}, {1,0}};
		
		cpPolyline line = MakeLoopedPolyline(3, verts);
		cpPolylineSet *set = cpPolylineConvexDecomposition_BETA(line, 0.0);
		
		GHAssertEquals(set->count, 1, nil);
		AssertPolyline(self, set->lines[0], 3, verts);
	}
	
	{
		cpVect verts[] = {{0,0}, {0,1}, {1,1}, {1,0}};
		
		cpPolyline line = MakeLoopedPolyline(4, verts);
		cpPolylineSet *set = cpPolylineConvexDecomposition_BETA(line, 0.0);
		
		GHAssertEquals(set->count, 1, nil);
		AssertPolyline(self, set->lines[0], 4, verts);
	}
	
	{
		cpVect verts[] = {{0,0}, {1,2}, {2,0}, {1,1}};
		cpVect expected1[] = {{0,0}, {1,2}, {1,1}};
		cpVect expected2[] = {{1,2}, {2,0}, {1,1}};
		
		cpPolyline line = MakeLoopedPolyline(4, verts);
		cpPolylineSet *set = cpPolylineConvexDecomposition_BETA(line, 0.0);
		
		GHAssertEquals(set->count, 2, nil);
		AssertPolyline(self, set->lines[0], 3, expected1);
		AssertPolyline(self, set->lines[1], 3, expected2);
	}
}

-(void)testValidate
{
	cpVect a = cpv(3.354, 2.546);
	cpVect b = cpv(55.213, 77.1232);
	
	int numVerts = 113;
//	cpVect verts[] = {a, cpvadd(cpvlerp(a, b, 0.5), cpv(0, -1e-7)), b};
	
	cpVect verts[numVerts];
	for(int i=0; i<numVerts; i++){
		verts[i] = cpvadd(cpvlerp(a, b, (cpFloat)i/(cpFloat)(numVerts + 1)), cpv(0, 0));
	}
	
	CP_CONVEX_HULL(numVerts, verts, hullCount, hullVerts);
	GHAssertTrue(cpPolyValidate(hullVerts, hullCount), nil);
}

@end