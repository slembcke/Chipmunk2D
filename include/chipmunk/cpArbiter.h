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

// User collision handler function types.
typedef cpBool (*cpCollisionBeginFunc)(cpArbiter *arb, cpSpace *space, void *data);
typedef cpBool (*cpCollisionPreSolveFunc)(cpArbiter *arb, cpSpace *space, void *data);
typedef void (*cpCollisionPostSolveFunc)(cpArbiter *arb, cpSpace *space, void *data);
typedef void (*cpCollisionSeparateFunc)(cpArbiter *arb, cpSpace *space, void *data);

// Structure for holding collision handler function callback information.
typedef struct cpCollisionHandler {
	cpCollisionType a;
	cpCollisionType b;
	cpCollisionBeginFunc begin;
	cpCollisionPreSolveFunc preSolve;
	cpCollisionPostSolveFunc postSolve;
	cpCollisionSeparateFunc separate;
	void *data;
} cpCollisionHandler;

// Data structure for contact points.
typedef struct cpContact cpContact;

#define CP_MAX_CONTACTS_PER_ARBITER 4

typedef enum cpArbiterState {
	cpArbiterStateNormal,
	cpArbiterStateFirstColl,
	cpArbiterStateIgnore,
	cpArbiterStateCached,
} cpArbiterState;

// Data structure for tracking collisions between shapes.
struct cpArbiter {
	// Information on the contact points between the objects.
	CP_PRIVATE(int numContacts);
	CP_PRIVATE(cpContact *contacts);
	
	// The two shapes and bodies involved in the collision.
	// These variables are NOT in the order defined by the collision handler.
	cpShape CP_PRIVATE(*a), CP_PRIVATE(*b);
	cpBody CP_PRIVATE(*body_a), CP_PRIVATE(*body_b);
	cpArbiter CP_PRIVATE(*nextA), CP_PRIVATE(*nextB);
	
	// Calculated before calling the pre-solve collision handler
	// Override them with custom values if you want specialized behavior
	CP_PRIVATE(cpFloat e);
	CP_PRIVATE(cpFloat u);
	 // Used for surface_v calculations, implementation may change
	CP_PRIVATE(cpVect surface_vr);
	
	// Time stamp of the arbiter. (from cpSpace)
	CP_PRIVATE(cpTimestamp stamp);
	
	CP_PRIVATE(cpCollisionHandler *handler);
	
	// Are the shapes swapped in relation to the collision handler?
	CP_PRIVATE(cpBool swappedColl);
	CP_PRIVATE(cpArbiterState state);
};

// Arbiter Helper Functions
cpVect cpArbiterTotalImpulse(cpArbiter *arb);
cpVect cpArbiterTotalImpulseWithFriction(cpArbiter *arb);
void cpArbiterIgnore(cpArbiter *arb);


static inline void
cpArbiterGetShapes(const cpArbiter *arb, cpShape **a, cpShape **b)
{
	if(arb->CP_PRIVATE(swappedColl)){
		(*a) = arb->CP_PRIVATE(b), (*b) = arb->CP_PRIVATE(a);
	} else {
		(*a) = arb->CP_PRIVATE(a), (*b) = arb->CP_PRIVATE(b);
	}
}
#define CP_ARBITER_GET_SHAPES(arb, a, b) cpShape *a, *b; cpArbiterGetShapes(arb, &a, &b);

static inline void
cpArbiterGetBodies(const cpArbiter *arb, cpBody **a, cpBody **b)
{
	CP_ARBITER_GET_SHAPES(arb, shape_a, shape_b);
	(*a) = shape_a->body;
	(*b) = shape_b->body;
}
#define CP_ARBITER_GET_BODIES(arb, a, b) cpBody *a, *b; cpArbiterGetBodies(arb, &a, &b);

static inline cpBool
cpArbiterIsFirstContact(const cpArbiter *arb)
{
	return arb->CP_PRIVATE(state) == cpArbiterStateFirstColl;
}

static inline int
cpArbiterGetCount(const cpArbiter *arb)
{
	return arb->CP_PRIVATE(numContacts);
}

typedef struct cpContactPointSet {
	int count;
	
	struct {
		cpVect point, normal;
		cpFloat dist;
	} points[CP_MAX_CONTACTS_PER_ARBITER];
} cpContactPointSet;

cpVect cpArbiterGetNormal(const cpArbiter *arb, int i);
cpVect cpArbiterGetPoint(const cpArbiter *arb, int i);
cpFloat cpArbiterGetDepth(const cpArbiter *arb, int i);
cpContactPointSet cpArbiterGetContactPointSet(const cpArbiter *arb);
