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

#import <XCTest/XCTest.h>
#import "ObjectiveChipmunk/ObjectiveChipmunk.h"

@interface ConvexTest : XCTestCase {}
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
	XCTAssertNotEqual(firstExpected, -1, @"");
	
	// Check the verts
	for(int i=0; i<expectedCount; i++){
		XCTAssertEqual(verts[i], expectedVerts[(firstExpected + i)%expectedCount], @"");
	}
}

static void
AssertHullsEqual(id self, int resultCount, cpVect *resultVerts)
{
	// Check that running the convex hull again returns the same value
	cpVect result2Verts[resultCount];
	int result2Count = cpConvexHull(resultCount, resultVerts, result2Verts, NULL, 0.0);
	
	XCTAssertEqual(resultCount, result2Count, @"");
	for(int i=0; i<resultCount; i++){
		XCTAssertEqual(resultVerts[i], result2Verts[i], @"");
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
		cpVect *resultVerts = alloca(count*sizeof(cpVect));
		int resultCount = cpConvexHull(count, rotated, resultVerts, &first, 0.0);
		
		// Check the count
		XCTAssertEqual(resultCount, expectedCount, @"");
		
		// Check that the windings are positive.
		XCTAssertTrue(cpAreaForPoly(expectedCount, expectedVerts, 0.0f) >= 0.0f, @"");
		XCTAssertTrue(cpAreaForPoly(resultCount, resultVerts, 0.0f) >= 0.0f, @"");
		
		XCTAssertEqual(resultVerts[0], rotated[first], @"");
		
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
	int resultCount = cpConvexHull(count, verts, resultVerts, NULL, 0.0f);
	
	AssertHullsEqual(self, resultCount, resultVerts);
	XCTAssertTrue(cpAreaForPoly(resultCount, resultVerts, 0.0f) > 0.0f, @"");
}

-(void)testConvexHull
{
	{
		// Trivial cases, reversed windings up to a square.
		cpVect verts[] = {{0,0}, {1,0}};
		cpVect expected[] = {{0,0}, {1,0}};
		
		AssertHull(self, 1, verts, 1, expected);
		AssertHull(self, 2, verts, 2, expected);
	}
	
	{
		cpVect verts[] = {{0,0}, {1,1}, {1,0}};
		cpVect expected[] = {{0,0}, {1,0}, {1,1}};
		
		AssertHull(self, 3, verts, 3, expected);
	}
	
	{
		cpVect verts[] = {{0,0}, {0,1}, {1,1}, {1,0}};
		cpVect expected[] = {{0,0}, {1,0}, {1,1}, {0,1}};
		
		AssertHull(self, 4, verts, 4, expected);
	}
	
	{
		// Degenerate hourglass shaped square.
		cpVect verts[] = {{0,0}, {1,0}, {0,1}, {1,1}};
		cpVect expected[] = {{0,0}, {1,0}, {1,1}, {0,1}};
		
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
		cpVect expected[] = {{0,0}, {1,0}, {1,1}, {0,1}};
		
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
		cpVect expected[] = {{0,0}, {1,0}, {1,1}, {0,1}};
		
		AssertHull(self, 8, verts, 4, expected);
	}
	
	{
		// Unit square with duplicate vertexes.
		cpVect verts1[] = {{0,0}, {0,1}, {1,1}, {1,0}, {0,0}, {0,1}, {1,1}, {1,0}};
		cpVect verts2[] = {{0,0}, {0,0}, {0,1}, {0,1}, {1,1}, {1,1}, {1,0}, {1,0}};
		cpVect expected[] = {{0,0}, {1,0}, {1,1}, {0,1}};
		
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
	XCTAssertTrue(cpAreaForPoly(hullCount, hullVerts, 0.0f) >= 0.0f, @"");
}

@end
