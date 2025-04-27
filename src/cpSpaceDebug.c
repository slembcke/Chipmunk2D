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

#include "chipmunk/chipmunk_private.h"

#ifndef CP_SPACE_DISABLE_DEBUG_API

static void
cpSpaceDebugDrawShape(cpShape *shape, cpSpaceDebugDrawOptions *options)
{
	cpBody *body = shape->body;
	cpDataPointer data = options->data;

	cpSpaceDebugColor outline_color = options->shapeOutlineColor;
	cpSpaceDebugColor fill_color = options->colorForShape(shape, data);

	switch (shape->klass->type)
	{
	case CP_CIRCLE_SHAPE:
	{
		cpCircleShape *circle = (cpCircleShape *)shape;

		cpVect tc = cpTransformPoint(options->transform, circle->tc);
		cpVect tc2 = cpv(circle->tc.x + circle->r, circle->tc.y);
		tc2 = cpTransformPoint(options->transform, tc2);
		cpFloat r = cpvdist(tc, tc2);

		cpVect pp0 = cpTransformPoint(options->transform, cpvzero);
		cpVect pp1 = cpTransformPoint(options->transform, cpv(1, 0));
		cpFloat dx = pp1.x - pp0.x;
		cpFloat dy = pp1.y - pp0.y;
		cpFloat angle = body->a + cpfatan2(dy, dx);

		options->drawCircle(tc, angle, r, outline_color, fill_color, data);
		break;
	}
	case CP_SEGMENT_SHAPE:
	{
		cpSegmentShape *seg = (cpSegmentShape *)shape;

		cpVect ta = cpTransformPoint(options->transform, seg->ta);
		cpVect tb = cpTransformPoint(options->transform, seg->tb);
		cpVect ta2 = cpv(seg->ta.x + seg->r, seg->ta.y);
		ta2 = cpTransformPoint(options->transform, ta2);
		cpFloat r = cpvdist(ta, ta2);

		options->drawFatSegment(ta, tb, r, outline_color, fill_color, data);
		break;
	}
	case CP_POLY_SHAPE:
	{
		cpPolyShape *poly = (cpPolyShape *)shape;

		int count = poly->count;
		struct cpSplittingPlane *planes = poly->planes;
		cpVect *verts = (cpVect *)alloca(count * sizeof(cpVect));
		cpFloat r = poly->r;
		for (int i = 0; i < count; i++)
		{

			verts[i] = cpTransformPoint(options->transform, planes[i].v0);
			if (i == 0)
			{
				cpVect p = cpv(planes[0].v0.x + r, planes[0].v0.y);
				cpVect v2 = cpTransformPoint(options->transform, p);
				r = cpvdist(verts[0], v2);
			}
		}

		options->drawPolygon(count, verts, r, outline_color, fill_color, data);
		break;
	}
	default:
		break;
	}
}

static const cpVect spring_verts[] = {
	{0.00f, 0.0f},
	{0.20f, 0.0f},
	{0.25f, 3.0f},
	{0.30f, -6.0f},
	{0.35f, 6.0f},
	{0.40f, -6.0f},
	{0.45f, 6.0f},
	{0.50f, -6.0f},
	{0.55f, 6.0f},
	{0.60f, -6.0f},
	{0.65f, 6.0f},
	{0.70f, -3.0f},
	{0.75f, 6.0f},
	{0.80f, 0.0f},
	{1.00f, 0.0f},
};
static const int spring_count = sizeof(spring_verts) / sizeof(cpVect);

static void
cpSpaceDebugDrawConstraint(cpConstraint *constraint, cpSpaceDebugDrawOptions *options)
{
	cpDataPointer data = options->data;
	cpSpaceDebugColor color = options->constraintColor;

	cpBody *body_a = constraint->a;
	cpBody *body_b = constraint->b;

	// The size of dots are not scaled by the transform, to make it possible
	// to use the transform to make drawing of a "small" (in pixels)
	// simulation without the dots filling up the screen.
	if (cpConstraintIsPinJoint(constraint))
	{
		cpPinJoint *joint = (cpPinJoint *)constraint;

		cpVect a = cpTransformPoint(body_a->transform, joint->anchorA);
		cpVect b = cpTransformPoint(body_b->transform, joint->anchorB);

		a = cpTransformPoint(options->transform, a);
		b = cpTransformPoint(options->transform, b);

		options->drawDot(5, a, color, data);
		options->drawDot(5, b, color, data);
		options->drawSegment(a, b, color, data);
	}
	else if (cpConstraintIsSlideJoint(constraint))
	{
		cpSlideJoint *joint = (cpSlideJoint *)constraint;

		cpVect a = cpTransformPoint(body_a->transform, joint->anchorA);
		cpVect b = cpTransformPoint(body_b->transform, joint->anchorB);

		a = cpTransformPoint(options->transform, a);
		b = cpTransformPoint(options->transform, b);

		options->drawDot(5, a, color, data);
		options->drawDot(5, b, color, data);
		options->drawSegment(a, b, color, data);
	}
	else if (cpConstraintIsPivotJoint(constraint))
	{
		cpPivotJoint *joint = (cpPivotJoint *)constraint;

		cpVect a = cpTransformPoint(body_a->transform, joint->anchorA);
		cpVect b = cpTransformPoint(body_b->transform, joint->anchorB);

		a = cpTransformPoint(options->transform, a);
		b = cpTransformPoint(options->transform, b);

		options->drawDot(5, a, color, data);
		options->drawDot(5, b, color, data);
	}
	else if (cpConstraintIsGrooveJoint(constraint))
	{
		cpGrooveJoint *joint = (cpGrooveJoint *)constraint;

		cpVect a = cpTransformPoint(body_a->transform, joint->grv_a);
		cpVect b = cpTransformPoint(body_a->transform, joint->grv_b);
		cpVect c = cpTransformPoint(body_b->transform, joint->anchorB);

		a = cpTransformPoint(options->transform, a);
		b = cpTransformPoint(options->transform, b);
		c = cpTransformPoint(options->transform, c);

		options->drawDot(5, c, color, data);
		options->drawSegment(a, b, color, data);
	}
	else if (cpConstraintIsDampedSpring(constraint))
	{
		cpDampedSpring *spring = (cpDampedSpring *)constraint;

		cpVect a = cpTransformPoint(body_a->transform, spring->anchorA);
		cpVect b = cpTransformPoint(body_b->transform, spring->anchorB);

		a = cpTransformPoint(options->transform, a);
		b = cpTransformPoint(options->transform, b);

		options->drawDot(5, a, color, data);
		options->drawDot(5, b, color, data);

		cpVect delta = cpvsub(b, a);
		cpFloat cos = delta.x;
		cpFloat sin = delta.y;
		cpFloat s = 1.0f / cpvlength(delta);

		cpVect r1 = cpv(cos, -sin * s);
		cpVect r2 = cpv(sin, cos * s);

		cpVect *verts = (cpVect *)alloca(spring_count * sizeof(cpVect));
		for (int i = 0; i < spring_count; i++)
		{
			cpVect v = spring_verts[i];
			verts[i] = cpv(cpvdot(v, r1) + a.x, cpvdot(v, r2) + a.y);
		}

		for (int i = 0; i < spring_count - 1; i++)
		{
			options->drawSegment(verts[i], verts[i + 1], color, data);
		}
	}
}

void cpSpaceDebugDraw(cpSpace *space, cpSpaceDebugDrawOptions *options)
{
	if (options->flags & CP_SPACE_DEBUG_DRAW_SHAPES)
	{
		cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)cpSpaceDebugDrawShape, options);
	}

	if (options->flags & CP_SPACE_DEBUG_DRAW_CONSTRAINTS)
	{
		cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)cpSpaceDebugDrawConstraint, options);
	}

	if (options->flags & CP_SPACE_DEBUG_DRAW_COLLISION_POINTS)
	{
		cpArray *arbiters = space->arbiters;
		cpSpaceDebugColor color = options->collisionPointColor;
		cpSpaceDebugDrawSegmentImpl draw_seg = options->drawSegment;
		cpDataPointer data = options->data;

		for (int i = 0; i < arbiters->num; i++)
		{
			cpArbiter *arb = (cpArbiter *)arbiters->arr[i];
			cpVect n = arb->n;

			for (int j = 0; j < arb->count; j++)
			{
				cpVect p1 = cpvadd(arb->body_a->p, arb->contacts[j].r1);
				cpVect p2 = cpvadd(arb->body_b->p, arb->contacts[j].r2);

				cpFloat d = 2.0f;
				cpVect a = cpvadd(p1, cpvmult(n, -d));
				cpVect b = cpvadd(p2, cpvmult(n, d));

				a = cpTransformPoint(options->transform, a);
				b = cpTransformPoint(options->transform, b);
				draw_seg(a, b, color, data);
			}
		}
	}
}

#endif
