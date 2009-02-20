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

struct cpBody;
typedef void (*cpBodyVelocityFunc)(struct cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);
typedef void (*cpBodyPositionFunc)(struct cpBody *body, cpFloat dt);

//#define CP_USE_DEPRECATED_API_4
typedef struct cpBody {
	// *** Integration Functions.

	// Function that is called to integrate the body's velocity. (Defaults to cpBodyUpdateVelocity)
	cpBodyVelocityFunc velocity_func;
	
	// Function that is called to integrate the body's position. (Defaults to cpBodyUpdatePosition)
	cpBodyPositionFunc position_func;
	
	// *** Mass Properties
	
	// Mass and it's inverse.
	// Always use cpBodySetMass() whenever changing the mass as these values must agree.
	union{
		cpFloat mass;
#ifdef CP_USE_DEPRECATED_API_4
		cpFloat m;
#endif
	};
	union{
		cpFloat mass_inv;
#ifdef CP_USE_DEPRECATED_API_4
		cpFloat m_inv;
#endif
	};
	
	// Moment of inertia and it's inverse.
	// Always use cpBodySetMass() whenever changing the mass as these values must agree.
	union{
		cpFloat moment;
#ifdef CP_USE_DEPRECATED_API_4
		cpFloat i;
#endif
	};
	union{
		cpFloat moment_inv;
#ifdef CP_USE_DEPRECATED_API_4
		cpFloat i_inv;
#endif
	};
	
	// *** Positional Properties
	
	// Linear components of motion (position, velocity, and force)
	union{
		cpVect pos;
#ifdef CP_USE_DEPRECATED_API_4
		cpVect p;
#endif
	};
	union{
		cpVect vel;
#ifdef CP_USE_DEPRECATED_API_4
		cpVect v;
#endif
	};
	union{
		cpVect force;
#ifdef CP_USE_DEPRECATED_API_4
		cpVect f;
#endif
	};
	
	// Angular components of motion (angle, angular velocity, and torque)
	// Always use cpBodySetAngle() to set the angle of the body as a and rot must agree.
	union{
		cpFloat angle;
#ifdef CP_USE_DEPRECATED_API_4
		cpFloat a;
#endif
	};
	union{
		cpFloat ang_vel;
#ifdef CP_USE_DEPRECATED_API_4
		cpFloat w;
#endif
	};
	union{
		cpFloat torque;
#ifdef CP_USE_DEPRECATED_API_4
		cpFloat t;
#endif
	};
	
	// Cached unit length vector representing the angle of the body.
	// Used for fast vector rotation using cpvrotate().
	cpVect rot;
	
	// *** User Definable Fields
	
	// User defined data pointer.
	void *data;
	
	// *** Internally Used Fields
	
	// Velocity bias values used when solving penetrations and correcting constraints.
	cpVect vel_bias;
	cpFloat ang_vel_bias;
	
//	int active;
} cpBody;

// Basic allocation/destruction functions
cpBody *cpBodyAlloc(void);
cpBody *cpBodyInit(cpBody *body, cpFloat m, cpFloat i);
cpBody *cpBodyNew(cpFloat m, cpFloat i);

void cpBodyDestroy(cpBody *body);
void cpBodyFree(cpBody *body);

// Setters for some of the special properties (mandatory!)
void cpBodySetMass(cpBody *body, cpFloat m);
void cpBodySetMoment(cpBody *body, cpFloat i);
void cpBodySetAngle(cpBody *body, cpFloat a);

//  Modify the velocity of the body so that it will move to the specified absolute coordinates in the next timestep.
// Intended for objects that are moved manually with a custom velocity integration function.
void cpBodySlew(cpBody *body, cpVect pos, cpFloat dt);

// Default Integration functions.
void cpBodyUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);
void cpBodyUpdatePosition(cpBody *body, cpFloat dt);

// Convert body local to world coordinates
static inline cpVect
cpBodyLocal2World(cpBody *body, cpVect v)
{
	return cpvadd(body->pos, cpvrotate(v, body->rot));
}

// Convert world to body local coordinates
static inline cpVect
cpBodyWorld2Local(cpBody *body, cpVect v)
{
	return cpvunrotate(cpvsub(v, body->pos), body->rot);
}

// Apply an impulse (in world coordinates) to the body.
static inline void
cpBodyApplyImpulse(cpBody *body, cpVect j, cpVect r)
{
	body->vel = cpvadd(body->vel, cpvmult(j, body->mass_inv));
	body->ang_vel += body->moment_inv*cpvcross(r, j);
}

// Not intended for external use. Used by cpArbiter.c and cpConstraint.c.
static inline void
cpBodyApplyBiasImpulse(cpBody *body, cpVect j, cpVect r)
{
	body->vel_bias = cpvadd(body->vel_bias, cpvmult(j, body->mass_inv));
	body->ang_vel_bias += body->moment_inv*cpvcross(r, j);
}

// Zero the forces on a body.
void cpBodyResetForces(cpBody *body);
// Apply a force (in world coordinates) to a body.
void cpBodyApplyForce(cpBody *body, cpVect f, cpVect r);

// Apply a damped spring force between two bodies.
void cpApplyDampedSpring(cpBody *a, cpBody *b, cpVect anchr1, cpVect anchr2, cpFloat rlen, cpFloat k, cpFloat dmp, cpFloat dt);

//int cpBodyMarkLowEnergy(cpBody *body, cpFloat dvsq, int max);
