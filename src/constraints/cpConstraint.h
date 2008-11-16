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

// TODO: Comment me!
	
extern cpFloat cp_constraint_bias_coef;

struct cpConstraintClass;
struct cpConstraint;

typedef void (*cpConstraintPreStepFunction)(struct cpConstraint *constraint, cpFloat dt, cpFloat dt_inv);
typedef void (*cpConstraintApplyImpulseFunction)(struct cpConstraint *constraint);
typedef cpFloat (*cpConstraintGetImpulseFunction)(struct cpConstraint *constraint);

typedef struct cpConstraintClass {
	cpConstraintPreStepFunction preStep;
	cpConstraintApplyImpulseFunction applyImpulse;
	cpConstraintGetImpulseFunction getImpulse;
} cpConstraintClass;

typedef struct cpConstraint {
	const cpConstraintClass *klass;
	
	cpBody *a, *b;
	cpFloat maxForce;
	cpFloat biasCoef;
	cpFloat maxBias;
} cpConstraint;

void cpConstraintDestroy(cpConstraint *constraint);
void cpConstraintFree(cpConstraint *constraint);


void cpConstraintCheckCast(cpConstraint *constraint, const cpConstraintClass *klass);

#define cpConstraintAccessor(s, t, m) \
static inline t \
s##_get_##m(cpConstraint *constraint){ \
	cpConstraintCheckCast(constraint, &s##Class); \
	return ((s *)constraint)->m; \
} \
static inline void \
s##_set_##m(cpConstraint *constraint, t value){ \
	cpConstraintCheckCast(constraint, &s##Class); \
	((s *)constraint)->m = value; \
}

// Built in Joint types
#include "cpPinJoint.h"
#include "cpSlideJoint.h"
#include "cpPivotJoint.h"
#include "cpGrooveJoint.h"
#include "cpDampedSpring.h"
#include "cpDampedRotarySpring.h"
#include "cpBreakableJoint.h"
#include "cpRotaryLimitJoint.h"
#include "cpGearJoint.h"
#include "cpSimpleMotor.h"
