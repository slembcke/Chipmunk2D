// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
//#include <sys/param.h >
#include <sys/sysctl.h>

#include "chipmunk/chipmunk_private.h"
#include "chipmunk/cpHastySpace.h"


//MARK: ARM NEON Solver

#if __ARM_NEON__
#include <arm_neon.h>

// Tested and known to work fine with Clang 3.0 and GCC 4.2
// Doesn't work with Clang 1.6, and I have no idea why.
#if defined(__clang_major__) && __clang_major__ < 3
	#error Compiler not supported.
#endif

#if CP_USE_DOUBLES
	#if !__arm64
		#error Cannot use CP_USE_DOUBLES on 32 bit ARM.
	#endif
	
	typedef float64_t cpFloat_t;
	typedef float64x2_t cpFloatx2_t;
	#define vld vld1q_f64
	#define vdup_n vdupq_n_f64
	#define vst vst1q_f64
	#define vst_lane vst1q_lane_f64
	#define vadd vaddq_f64
	#define vsub vsubq_f64
	#define vpadd vpaddq_f64
	#define vmul vmulq_f64
	#define vmul_n vmulq_n_f64
	#define vneg vnegq_f64
	#define vget_lane vgetq_lane_f64
	#define vset_lane vsetq_lane_f64
	#define vmin vminq_f64
	#define vmax vmaxq_f64
	#define vrev(__a) __builtin_shufflevector(__a, __a, 1, 0)
#else
	typedef float32_t cpFloat_t;
	typedef float32x2_t cpFloatx2_t;
	#define vld vld1_f32
	#define vdup_n vdup_n_f32
	#define vst vst1_f32
	#define vst_lane vst1_lane_f32
	#define vadd vadd_f32
	#define vsub vsub_f32
	#define vpadd vpadd_f32
	#define vmul vmul_f32
	#define vmul_n vmul_n_f32
	#define vneg vneg_f32
	#define vget_lane vget_lane_f32
	#define vset_lane vset_lane_f32
	#define vmin vmin_f32
	#define vmax vmax_f32
	#define vrev vrev64_f32
#endif

// TODO could probably do better here, maybe using vcreate?
// especially for the constants
// Maybe use the {} notation for GCC/Clang?
static inline cpFloatx2_t
vmake(cpFloat_t x, cpFloat_t y)
{
//	cpFloatx2_t v = {};
//	v = vset_lane(x, v, 0);
//	v = vset_lane(y, v, 1);
//	
//	return v;
	
	// This might not be super compatible, but all the NEON headers use it...
	return (cpFloatx2_t){x, y};
}

static void
cpArbiterApplyImpulse_NEON(cpArbiter *arb)
{
	cpBody *a = arb->body_a;
	cpBody *b = arb->body_b;
	cpFloatx2_t surface_vr = vld((cpFloat_t *)&arb->surface_vr);
	cpFloatx2_t n = vld((cpFloat_t *)&arb->n);
	cpFloat_t friction = arb->u;
	
	int numContacts = arb->count;
	struct cpContact *contacts = arb->contacts;
	for(int i=0; i<numContacts; i++){
		struct cpContact *con = contacts + i;
		cpFloatx2_t r1 = vld((cpFloat_t *)&con->r1);
		cpFloatx2_t r2 = vld((cpFloat_t *)&con->r2);
		
		cpFloatx2_t perp = vmake(-1.0, 1.0);
		cpFloatx2_t r1p = vmul(vrev(r1), perp);
		cpFloatx2_t r2p = vmul(vrev(r2), perp);
		
		cpFloatx2_t vBias_a = vld((cpFloat_t *)&a->v_bias);
		cpFloatx2_t vBias_b = vld((cpFloat_t *)&b->v_bias);
		cpFloatx2_t wBias = vmake(a->w_bias, b->w_bias);
		
		cpFloatx2_t vb1 = vadd(vBias_a, vmul_n(r1p, vget_lane(wBias, 0)));
		cpFloatx2_t vb2 = vadd(vBias_b, vmul_n(r2p, vget_lane(wBias, 1)));
		cpFloatx2_t vbr = vsub(vb2, vb1);
		
		cpFloatx2_t v_a = vld((cpFloat_t *)&a->v);
		cpFloatx2_t v_b = vld((cpFloat_t *)&b->v);
		cpFloatx2_t w = vmake(a->w, b->w);
		cpFloatx2_t v1 = vadd(v_a, vmul_n(r1p, vget_lane(w, 0)));
		cpFloatx2_t v2 = vadd(v_b, vmul_n(r2p, vget_lane(w, 1)));
		cpFloatx2_t vr = vsub(v2, v1);
		
		cpFloatx2_t vbn_vrn = vpadd(vmul(vbr, n), vmul(vr, n));
		
		cpFloatx2_t v_offset = vmake(con->bias, -con->bounce);
		cpFloatx2_t jOld = vmake(con->jBias, con->jnAcc);
		cpFloatx2_t jbn_jn = vmul_n(vsub(v_offset, vbn_vrn), con->nMass);
		jbn_jn = vmax(vadd(jOld, jbn_jn), vdup_n(0.0));
		cpFloatx2_t jApply = vsub(jbn_jn, jOld);
		
		cpFloatx2_t t = vmul(vrev(n), perp);
		cpFloatx2_t vrt_tmp = vmul(vadd(vr, surface_vr), t);
		cpFloatx2_t vrt = vpadd(vrt_tmp, vrt_tmp);
		
		cpFloatx2_t jtOld = {}; jtOld = vset_lane(con->jtAcc, jtOld, 0);
		cpFloatx2_t jtMax = vrev(vmul_n(jbn_jn, friction));
		cpFloatx2_t jt = vmul_n(vrt, -con->tMass);
		jt = vmax(vneg(jtMax), vmin(vadd(jtOld, jt), jtMax));
		cpFloatx2_t jtApply = vsub(jt, jtOld);
		
		cpFloatx2_t i_inv = vmake(-a->i_inv, b->i_inv);
		cpFloatx2_t nperp = vmake(1.0, -1.0);
		
		cpFloatx2_t jBias = vmul_n(n, vget_lane(jApply, 0));
		cpFloatx2_t jBiasCross = vmul(vrev(jBias), nperp);
		cpFloatx2_t biasCrosses = vpadd(vmul(r1, jBiasCross), vmul(r2, jBiasCross));
		wBias = vadd(wBias, vmul(i_inv, biasCrosses));
		
		vBias_a = vsub(vBias_a, vmul_n(jBias, a->m_inv));
		vBias_b = vadd(vBias_b, vmul_n(jBias, b->m_inv));
		
		cpFloatx2_t j = vadd(vmul_n(n, vget_lane(jApply, 1)), vmul_n(t, vget_lane(jtApply, 0)));
		cpFloatx2_t jCross = vmul(vrev(j), nperp);
		cpFloatx2_t crosses = vpadd(vmul(r1, jCross), vmul(r2, jCross));
		w = vadd(w, vmul(i_inv, crosses));
		
		v_a = vsub(v_a, vmul_n(j, a->m_inv));
		v_b = vadd(v_b, vmul_n(j, b->m_inv));
		
		// TODO would moving these earlier help pipeline them better?
		vst((cpFloat_t *)&a->v_bias, vBias_a);
		vst((cpFloat_t *)&b->v_bias, vBias_b);
		vst_lane((cpFloat_t *)&a->w_bias, wBias, 0);
		vst_lane((cpFloat_t *)&b->w_bias, wBias, 1);
		
		vst((cpFloat_t *)&a->v, v_a);
		vst((cpFloat_t *)&b->v, v_b);
		vst_lane((cpFloat_t *)&a->w, w, 0);
		vst_lane((cpFloat_t *)&b->w, w, 1);
		
		vst_lane((cpFloat_t *)&con->jBias, jbn_jn, 0);
		vst_lane((cpFloat_t *)&con->jnAcc, jbn_jn, 1);
		vst_lane((cpFloat_t *)&con->jtAcc, jt, 0);
	}
}

#endif

//MARK: PThreads

// Right now using more than 2 threads probably wont help your performance any.
// If you are using a ridiculous number of iterations it could help though.
#define MAX_THREADS 2

struct ThreadContext {
	pthread_t thread;
	cpHastySpace *space;
	unsigned long thread_num;
};

typedef	void (*cpHastySpaceWorkFunction)(cpSpace *space, unsigned long worker, unsigned long worker_count);

struct cpHastySpace {
	cpSpace space;
	
	// Number of worker threads (including the main thread)
	unsigned long num_threads;
	
	// Number of worker threads currently executing. (also including the main thread)
	unsigned long num_working;
	
	// Number of constraints (plus contacts) that must exist per step to start the worker threads.
	unsigned long constraint_count_threshold;
	
	pthread_mutex_t mutex;
	pthread_cond_t cond_work, cond_resume;
	
	// Work function to invoke.
	cpHastySpaceWorkFunction work;
	
	struct ThreadContext workers[MAX_THREADS - 1];
};

static void *
WorkerThreadLoop(struct ThreadContext *context)
{
	cpHastySpace *hasty = context->space;
	
	unsigned long thread = context->thread_num;
	unsigned long num_threads = hasty->num_threads;
	
	for(;;){
		pthread_mutex_lock(&hasty->mutex); {
			if(--hasty->num_working == 0){
				pthread_cond_signal(&hasty->cond_resume);
			}
			
			pthread_cond_wait(&hasty->cond_work, &hasty->mutex);
		} pthread_mutex_unlock(&hasty->mutex);
		
		cpHastySpaceWorkFunction func = hasty->work;
		if(func){
			hasty->work(&hasty->space, thread, num_threads);
		} else {
			break;
		}
	}
	
	return NULL;
}

static void
RunWorkers(cpHastySpace *hasty, cpHastySpaceWorkFunction func)
{
	hasty->num_working = hasty->num_threads - 1;
	hasty->work = func;
	
	if(hasty->num_working > 0){
		pthread_mutex_lock(&hasty->mutex); {
			pthread_cond_broadcast(&hasty->cond_work);
		} pthread_mutex_unlock(&hasty->mutex);
		
		func((cpSpace *)hasty, 0, hasty->num_threads);
			
		pthread_mutex_lock(&hasty->mutex); {
			if(hasty->num_working > 0){
				pthread_cond_wait(&hasty->cond_resume, &hasty->mutex);
			}
		} pthread_mutex_unlock(&hasty->mutex);
	} else {
		func((cpSpace *)hasty, 0, hasty->num_threads);
	}
	
	hasty->work = NULL;
}

static void
Solver(cpSpace *space, unsigned long worker, unsigned long worker_count)
{
	cpArray *constraints = space->constraints;
	cpArray *arbiters = space->arbiters;
	
	cpFloat dt = space->curr_dt;
	unsigned long iterations = (space->iterations + worker_count - 1)/worker_count;
	
	for(unsigned long i=0; i<iterations; i++){
		for(int j=0; j<arbiters->num; j++){
			cpArbiter *arb = (cpArbiter *)arbiters->arr[j];
			#ifdef __ARM_NEON__
				cpArbiterApplyImpulse_NEON(arb);
			#else
				cpArbiterApplyImpulse(arb);
			#endif
		}
			
		for(int j=0; j<constraints->num; j++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[j];
			constraint->klass->applyImpulse(constraint, dt);
		}
	}
}

//MARK: Thread Management Functions

static void
HaltThreads(cpHastySpace *hasty)
{
	pthread_mutex_t *mutex = &hasty->mutex;
	pthread_mutex_lock(mutex); {
		hasty->work = NULL; // NULL work function means break and exit
		pthread_cond_broadcast(&hasty->cond_work);
	} pthread_mutex_unlock(mutex);
	
	for(unsigned long i=0; i<(hasty->num_threads-1); i++){
		pthread_join(hasty->workers[i].thread, NULL);
	}
}

void
cpHastySpaceSetThreads(cpSpace *space, unsigned long threads)
{
#if TARGET_IPHONE_SIMULATOR == 1
	// Individual values appear to be written non-atomically when compiled as debug for the simulator.
	// No idea why, so threads are disabled.
	threads = 1;
#endif	
	
	cpHastySpace *hasty = (cpHastySpace *)space;
	HaltThreads(hasty);
	
#ifdef __APPLE__
	if(threads == 0){
		size_t size = sizeof(threads);
		sysctlbyname("hw.ncpu", &threads, &size, NULL, 0);
	}
#else
	if(threads == 0) threads = 1;
#endif
	
	hasty->num_threads = (threads < MAX_THREADS ? threads : MAX_THREADS);
	hasty->num_working = hasty->num_threads - 1;
	
	// Create the worker threads and wait for them to signal ready.
	if(hasty->num_working > 0){
		pthread_mutex_lock(&hasty->mutex);
		for(unsigned long i=0; i<(hasty->num_threads-1); i++){
			hasty->workers[i].space = hasty;
			hasty->workers[i].thread_num = i + 1;
			
			pthread_create(&hasty->workers[i].thread, NULL, (void *)WorkerThreadLoop, &hasty->workers[i]);
		}
		
		pthread_cond_wait(&hasty->cond_resume, &hasty->mutex);
		pthread_mutex_unlock(&hasty->mutex);
	}
}

unsigned long
cpHastySpaceGetThreads(cpSpace *space)
{
	return ((cpHastySpace *)space)->num_threads;
}

//MARK: Overriden cpSpace Functions.

cpSpace *
cpHastySpaceNew(void)
{
	cpHastySpace *hasty = (cpHastySpace *)cpcalloc(1, sizeof(cpHastySpace));
	cpSpaceInit((cpSpace *)hasty);
	
	pthread_mutex_init(&hasty->mutex, NULL);
	pthread_cond_init(&hasty->cond_work, NULL);
	pthread_cond_init(&hasty->cond_resume, NULL);
	
	// TODO magic number, should test this more thoroughly.
	hasty->constraint_count_threshold = 50;
	
	// Default to 1 thread for determinism.
	hasty->num_threads = 1;
	cpHastySpaceSetThreads((cpSpace *)hasty, 1);

	return (cpSpace *)hasty;
}

void
cpHastySpaceFree(cpSpace *space)
{
	cpHastySpace *hasty = (cpHastySpace *)space;
	
	HaltThreads(hasty);
	
	pthread_mutex_destroy(&hasty->mutex);
	pthread_cond_destroy(&hasty->cond_work);
	pthread_cond_destroy(&hasty->cond_resume);
	
	cpSpaceFree(space);
}

void
cpHastySpaceStep(cpSpace *space, cpFloat dt)
{
	// don't step if the timestep is 0!
	if(dt == 0.0f) return;
	
	space->stamp++;
	
	cpFloat prev_dt = space->curr_dt;
	space->curr_dt = dt;
		
	cpArray *bodies = space->dynamicBodies;
	cpArray *constraints = space->constraints;
	cpArray *arbiters = space->arbiters;
	
	// Reset and empty the arbiter list.
	for(int i=0; i<arbiters->num; i++){
		cpArbiter *arb = (cpArbiter *)arbiters->arr[i];
		arb->state = CP_ARBITER_STATE_NORMAL;
		
		// If both bodies are awake, unthread the arbiter from the contact graph.
		if(!cpBodyIsSleeping(arb->body_a) && !cpBodyIsSleeping(arb->body_b)){
			cpArbiterUnthread(arb);
		}
	}
	arbiters->num = 0;

	cpSpaceLock(space); {
		// Integrate positions
		for(int i=0; i<bodies->num; i++){
			cpBody *body = (cpBody *)bodies->arr[i];
			body->position_func(body, dt);
		}
		
		// Find colliding pairs.
		cpSpacePushFreshContactBuffer(space);
		cpSpatialIndexEach(space->dynamicShapes, (cpSpatialIndexIteratorFunc)cpShapeUpdateFunc, NULL);
		cpSpatialIndexReindexQuery(space->dynamicShapes, (cpSpatialIndexQueryFunc)cpSpaceCollideShapes, space);
	} cpSpaceUnlock(space, cpFalse);
	
	// Rebuild the contact graph (and detect sleeping components if sleeping is enabled)
	cpSpaceProcessComponents(space, dt);
	
	cpSpaceLock(space); {
		// Clear out old cached arbiters and call separate callbacks
		cpHashSetFilter(space->cachedArbiters, (cpHashSetFilterFunc)cpSpaceArbiterSetFilter, space);

		// Prestep the arbiters and constraints.
		cpFloat slop = space->collisionSlop;
		cpFloat biasCoef = 1.0f - cpfpow(space->collisionBias, dt);
		for(int i=0; i<arbiters->num; i++){
			cpArbiterPreStep((cpArbiter *)arbiters->arr[i], dt, slop, biasCoef);
		}

		for(int i=0; i<constraints->num; i++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
			
			cpConstraintPreSolveFunc preSolve = constraint->preSolve;
			if(preSolve) preSolve(constraint, space);
			
			constraint->klass->preStep(constraint, dt);
		}
	
		// Integrate velocities.
		cpFloat damping = cpfpow(space->damping, dt);
		cpVect gravity = space->gravity;
		for(int i=0; i<bodies->num; i++){
			cpBody *body = (cpBody *)bodies->arr[i];
			body->velocity_func(body, gravity, damping, dt);
		}
		
		// Apply cached impulses
		cpFloat dt_coef = (prev_dt == 0.0f ? 0.0f : dt/prev_dt);
		for(int i=0; i<arbiters->num; i++){
			cpArbiterApplyCachedImpulse((cpArbiter *)arbiters->arr[i], dt_coef);
		}
		
		for(int i=0; i<constraints->num; i++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
			constraint->klass->applyCachedImpulse(constraint, dt_coef);
		}
		
		// Run the impulse solver.
		cpHastySpace *hasty = (cpHastySpace *)space;
		if((unsigned long)(arbiters->num + constraints->num) > hasty->constraint_count_threshold){
			RunWorkers(hasty, Solver);
		} else {
			Solver(space, 0, 1);
		}
		
		// Run the constraint post-solve callbacks
		for(int i=0; i<constraints->num; i++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
			
			cpConstraintPostSolveFunc postSolve = constraint->postSolve;
			if(postSolve) postSolve(constraint, space);
		}
		
		// run the post-solve callbacks
		for(int i=0; i<arbiters->num; i++){
			cpArbiter *arb = (cpArbiter *) arbiters->arr[i];
			
			cpCollisionHandler *handler = arb->handler;
			handler->postSolveFunc(arb, space, handler->userData);
		}
	} cpSpaceUnlock(space, cpTrue);
}
