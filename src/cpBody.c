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
 
#include <stdlib.h>
#include <float.h>

#include "chipmunk.h"

cpBody*
cpBodyAlloc(void)
{
	return (cpBody *)malloc(sizeof(cpBody));
}

cpBody*
cpBodyInit(cpBody *body, cpFloat m, cpFloat i)
{
	body->velocity_func = cpBodyUpdateVelocity;
	body->position_func = cpBodyUpdatePosition;
	
	cpBodySetMass(body, m);
	cpBodySetMoment(body, i);

	body->pos = cpvzero;
	body->vel = cpvzero;
	body->force = cpvzero;
	
	cpBodySetAngle(body, 0.0f);
	body->ang_vel = 0.0f;
	body->torque = 0.0f;
	
	body->vel_bias = cpvzero;
	body->ang_vel_bias = 0.0f;
	
	body->data = NULL;
//	body->active = 1;

	return body;
}

cpBody*
cpBodyNew(cpFloat m, cpFloat i)
{
	return cpBodyInit(cpBodyAlloc(), m, i);
}

void cpBodyDestroy(cpBody *body){}

void
cpBodyFree(cpBody *body)
{
	if(body) cpBodyDestroy(body);
	free(body);
}

void
cpBodySetMass(cpBody *body, cpFloat mass)
{
	body->mass = mass;
	body->mass_inv = 1.0f/mass;
}

void
cpBodySetMoment(cpBody *body, cpFloat moment)
{
	body->moment = moment;
	body->moment_inv = 1.0f/moment;
}

void
cpBodySetAngle(cpBody *body, cpFloat angle)
{
	body->angle = angle;//fmod(a, (cpFloat)M_PI*2.0f);
	body->rot = cpvforangle(angle);
}

void
cpBodySlew(cpBody *body, cpVect pos, cpFloat dt)
{
	cpVect delta = cpvsub(pos, body->pos);
	body->vel = cpvmult(delta, 1.0/dt);
}

void
cpBodyUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	body->vel = cpvadd(cpvmult(body->vel, damping), cpvmult(cpvadd(gravity, cpvmult(body->force, body->mass_inv)), dt));
	body->ang_vel = body->ang_vel*damping + body->torque*body->moment_inv*dt;
}

void
cpBodyUpdatePosition(cpBody *body, cpFloat dt)
{
	body->pos = cpvadd(body->pos, cpvmult(cpvadd(body->vel, body->vel_bias), dt));
	cpBodySetAngle(body, body->angle + (body->ang_vel + body->ang_vel_bias)*dt);
	
	body->vel_bias = cpvzero;
	body->ang_vel_bias = 0.0f;
}

void
cpBodyResetForces(cpBody *body)
{
	body->force = cpvzero;
	body->torque = 0.0f;
}

void
cpBodyApplyForce(cpBody *body, cpVect force, cpVect r)
{
	body->force = cpvadd(body->force, force);
	body->torque += cpvcross(r, force);
}

void
cpApplyDampedSpring(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat rlen, cpFloat k, cpFloat dmp, cpFloat dt)
{
	// Calculate the world space anchor coordinates.
	cpVect r1 = cpvrotate(anchr1, a->rot);
	cpVect r2 = cpvrotate(anchr2, b->rot);
	
	cpVect delta = cpvsub(cpvadd(b->pos, r2), cpvadd(a->pos, r1));
	cpFloat dist = cpvlength(delta);
	cpVect n = dist ? cpvmult(delta, 1.0f/dist) : cpvzero;
	
	cpFloat f_spring = (dist - rlen)*k;

	// Calculate the world relative velocities of the anchor points.
	cpVect v1 = cpvadd(a->vel, cpvmult(cpvperp(r1), a->ang_vel));
	cpVect v2 = cpvadd(b->vel, cpvmult(cpvperp(r2), b->ang_vel));
	
	// Calculate the damping force.
	// This really should be in the impulse solver and can produce problems when using large damping values.
	cpFloat vrn = cpvdot(cpvsub(v2, v1), n);
	cpFloat f_damp = vrn*cpfmin(dmp, 1.0f/(dt*(a->mass_inv + b->mass_inv)));
	
	// Apply!
	cpVect f = cpvmult(n, f_spring + f_damp);
	cpBodyApplyForce(a, f, r1);
	cpBodyApplyForce(b, cpvneg(f), r2);
}

//int
//cpBodyMarkLowEnergy(cpBody *body, cpFloat dvsq, int max)
//{
//	cpFloat ke = body->m*cpvdot(body->v, body->v);
//	cpFloat re = body->i*body->w*body->w;
//	
//	if(ke + re > body->m*dvsq)
//		body->active = 1;
//	else if(body->active)
//		body->active = (body->active + 1)%(max + 1);
//	else {
//		body->v = cpvzero;
//		body->v_bias = cpvzero;
//		body->w = 0.0f;
//		body->w_bias = 0.0f;
//	}
//	
//	return body->active;
//}
