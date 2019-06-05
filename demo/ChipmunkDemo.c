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
 
/*
	IMPORTANT - READ ME!
	
	This file sets up a simple interface that the individual demos can use to get
	a Chipmunk space running and draw what's in it. In order to keep the Chipmunk
	examples clean and simple, they contain no graphics code. All drawing is done
	by accessing the Chipmunk structures at a very low level. It is NOT
	recommended to write a game or application this way as it does not scale
	beyond simple shape drawing and is very dependent on implementation details
	about Chipmunk which may change with little to no warning.
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "chipmunk/chipmunk_private.h"
#include "ChipmunkDemo.h"
#include "ChipmunkDemoTextSupport.h"

#include "sokol/sokol.h"

#undef Convex

static ChipmunkDemo demos[32];
static int demo_count;
static int demo_index;

static cpBool paused = cpFalse;
static cpBool step = cpFalse;

static cpSpace *space;

static double Accumulator;
static double LastTime;
int ChipmunkDemoTicks;
double ChipmunkDemoTime;

cpVect ChipmunkDemoMouse;
cpBool ChipmunkDemoRightClick;
cpBool ChipmunkDemoRightDown;
cpVect ChipmunkDemoKeyboard;

static cpBody *mouse_body = NULL;
static cpConstraint *mouse_joint = NULL;

char const *ChipmunkDemoMessageString = NULL;

#define GRABBABLE_MASK_BIT (1<<31)
cpShapeFilter GRAB_FILTER = {CP_NO_GROUP, GRABBABLE_MASK_BIT, GRABBABLE_MASK_BIT};
cpShapeFilter NOT_GRABBABLE_FILTER = {CP_NO_GROUP, ~GRABBABLE_MASK_BIT, ~GRABBABLE_MASK_BIT};

cpVect view_translate = {0, 0};
cpFloat view_scale = 1.0;

static void ShapeFreeWrap(cpSpace *space, cpShape *shape, void *unused){
	cpSpaceRemoveShape(space, shape);
	cpShapeFree(shape);
}

static void PostShapeFree(cpShape *shape, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ShapeFreeWrap, shape, NULL);
}

static void ConstraintFreeWrap(cpSpace *space, cpConstraint *constraint, void *unused){
	cpSpaceRemoveConstraint(space, constraint);
	cpConstraintFree(constraint);
}

static void PostConstraintFree(cpConstraint *constraint, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ConstraintFreeWrap, constraint, NULL);
}

static void BodyFreeWrap(cpSpace *space, cpBody *body, void *unused){
	cpSpaceRemoveBody(space, body);
	cpBodyFree(body);
}

static void PostBodyFree(cpBody *body, cpSpace *space){
	cpSpaceAddPostStepCallback(space, (cpPostStepFunc)BodyFreeWrap, body, NULL);
}

// Safe and future proof way to remove and free all objects that have been added to the space.
void
ChipmunkDemoFreeSpaceChildren(cpSpace *space)
{
	// Must remove these BEFORE freeing the body or you will access dangling pointers.
	cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)PostShapeFree, space);
	cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)PostConstraintFree, space);
	
	cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)PostBodyFree, space);
}

static void
DrawCircle(cpVect p, cpFloat a, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawCircle(p, a, r, outline, fill);}

static void
DrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data)
{ChipmunkDebugDrawSegment(a, b, color);}

static void
DrawFatSegment(cpVect a, cpVect b, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawFatSegment(a, b, r, outline, fill);}

static void
DrawPolygon(int count, const cpVect *verts, cpFloat r, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{ChipmunkDebugDrawPolygon(count, verts, r, outline, fill);}

static void
DrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data)
{ChipmunkDebugDrawDot(size, pos, color);}


static cpSpaceDebugColor Colors[] = {
	{0xb5/255.0f, 0x89/255.0f, 0x00/255.0f, 1.0f},
	{0xcb/255.0f, 0x4b/255.0f, 0x16/255.0f, 1.0f},
	{0xdc/255.0f, 0x32/255.0f, 0x2f/255.0f, 1.0f},
	{0xd3/255.0f, 0x36/255.0f, 0x82/255.0f, 1.0f},
	{0x6c/255.0f, 0x71/255.0f, 0xc4/255.0f, 1.0f},
	{0x26/255.0f, 0x8b/255.0f, 0xd2/255.0f, 1.0f},
	{0x2a/255.0f, 0xa1/255.0f, 0x98/255.0f, 1.0f},
	{0x85/255.0f, 0x99/255.0f, 0x00/255.0f, 1.0f},
};

static cpSpaceDebugColor
ColorForShape(cpShape *shape, cpDataPointer data)
{
	if(cpShapeGetSensor(shape)){
		return LAColor(1.0f, 0.1f);
	} else {
		cpBody *body = cpShapeGetBody(shape);
		
		if(cpBodyIsSleeping(body)){
			return RGBAColor(0x58/255.0f, 0x6e/255.0f, 0x75/255.0f, 1.0f);
		} else if(body->sleeping.idleTime > shape->space->sleepTimeThreshold) {
			return RGBAColor(0x93/255.0f, 0xa1/255.0f, 0xa1/255.0f, 1.0f);
		} else {
			uint32_t val = (uint32_t)shape->hashid;
			
			// scramble the bits up using Robert Jenkins' 32 bit integer hash function
			val = (val+0x7ed55d16) + (val<<12);
			val = (val^0xc761c23c) ^ (val>>19);
			val = (val+0x165667b1) + (val<<5);
			val = (val+0xd3a2646c) ^ (val<<9);
			val = (val+0xfd7046c5) + (val<<3);
			val = (val^0xb55a4f09) ^ (val>>16);
			return Colors[val & 0x7];
		}
	}
}

void
ChipmunkDemoDefaultDrawImpl(cpSpace *space)
{
	cpSpaceDebugDrawOptions drawOptions = {
		DrawCircle,
		DrawSegment,
		DrawFatSegment,
		DrawPolygon,
		DrawDot,
		
		(cpSpaceDebugDrawFlags)(CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_CONSTRAINTS | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS),
		
		{0xEE/255.0f, 0xE8/255.0f, 0xD5/255.0f, 1.0f}, // Outline color
		ColorForShape,
		{0.0f, 0.75f, 0.0f, 1.0f}, // Constraint color
		{1.0f, 0.0f, 0.0f, 1.0f}, // Collision point color
		NULL,
	};
	
	cpSpaceDebugDraw(space, &drawOptions);
}

static void
DrawInstructions()
{
	static char title[1024];
	sprintf(title, "Demo(%c): %s", 'A' + demo_index, demos[demo_index].name);
	ChipmunkDemoTextDrawString(cpv(-300, 220), title);
	
	ChipmunkDemoTextDrawString(cpv(-300, 200),
		"Controls:\n"
		"A - Z Switch demos. (return restarts)\n"
		"Use the mouse to grab objects.\n"
	);
}

static int max_arbiters = 0;
static int max_points = 0;
static int max_constraints = 0;

static void
DrawInfo()
{
	int arbiters = space->arbiters->num;
	int points = 0;
	
	for(int i=0; i<arbiters; i++)
		points += ((cpArbiter *)(space->arbiters->arr[i]))->count;
	
	int constraints = (space->constraints->num + points)*space->iterations;
	
	max_arbiters = arbiters > max_arbiters ? arbiters : max_arbiters;
	max_points = points > max_points ? points : max_points;
	max_constraints = constraints > max_constraints ? constraints : max_constraints;
	
	char buffer[1024];
	const char *format = 
		"Arbiters: %d (%d) - "
		"Contact Points: %d (%d)\n"
		"Other Constraints: %d, Iterations: %d\n"
		"Constraints x Iterations: %d (%d)\n"
		"Time:% 5.2fs, KE:% 5.2e";
	
	cpArray *bodies = space->dynamicBodies;
	cpFloat ke = 0.0f;
	for(int i=0; i<bodies->num; i++){
		cpBody *body = (cpBody *)bodies->arr[i];
		if(body->m == INFINITY || body->i == INFINITY) continue;
		
		ke += body->m*cpvdot(body->v, body->v) + body->i*body->w*body->w;
	}
	
	sprintf(buffer, format,
		arbiters, max_arbiters,
		points, max_points,
		space->constraints->num, space->iterations,
		constraints, max_constraints,
		ChipmunkDemoTime, (ke < 1e-10f ? 0.0f : ke)
	);
	
	ChipmunkDemoTextDrawString(cpv(0, 220), buffer);
}

static char PrintStringBuffer[1024*8];
static char *PrintStringCursor;

void
ChipmunkDemoPrintString(char const *fmt, ...)
{
	if (PrintStringCursor == NULL) {
		return;
	}

	ChipmunkDemoMessageString = PrintStringBuffer;

	va_list args;
	va_start(args, fmt);
	int remaining = sizeof(PrintStringBuffer) - (PrintStringCursor - PrintStringBuffer);
	int would_write = vsnprintf(PrintStringCursor, remaining, fmt, args);
	if (would_write > 0 && would_write < remaining) {
		PrintStringCursor += would_write;
	} else {
		// encoding error or overflow, prevent further use until reinitialized
		PrintStringCursor = NULL;
	}
	va_end(args);
}

static void
Tick(double dt)
{
	if(!paused || step){
		PrintStringBuffer[0] = 0;
		PrintStringCursor = PrintStringBuffer;
		
		// Completely reset the renderer only at the beginning of a tick.
		// That way it can always display at least the last ticks' debug drawing.
		ChipmunkDebugDrawClearRenderer();
		ChipmunkDemoTextClearRenderer();
		
		cpVect new_point = cpvlerp(mouse_body->p, ChipmunkDemoMouse, 0.25f);
		mouse_body->v = cpvmult(cpvsub(new_point, mouse_body->p), 60.0f);
		mouse_body->p = new_point;
		
		demos[demo_index].updateFunc(space, dt);
		
		ChipmunkDemoTicks++;
		ChipmunkDemoTime += dt;
		
		step = cpFalse;
		ChipmunkDemoRightDown = cpFalse;
		
		ChipmunkDemoTextDrawString(cpv(-300, -200), ChipmunkDemoMessageString);
	}
}

static void
Update(void)
{
	double time = stm_sec(stm_now());
	double dt = time - LastTime;
	if(dt > 0.2) dt = 0.2;
	
	double fixed_dt = demos[demo_index].timestep;
	
	for(Accumulator += dt; Accumulator > fixed_dt; Accumulator -= fixed_dt){
		Tick(fixed_dt);
	}
	
	LastTime = time;
}

static void
Display(void)
{
	cpVect screen_size = {sapp_width(), sapp_height()};
	cpTransform view_matrix = cpTransformMult(cpTransformScale(view_scale, view_scale), cpTransformTranslate(view_translate));
	
	float screen_scale = (float)cpfmin(screen_size.x/640.0, screen_size.y/480.0);
	float hw = (float)screen_size.x*(0.5f/screen_scale);
	float hh = (float)screen_size.y*(0.5f/screen_scale);
	cpTransform projection_matrix = cpTransformOrtho(cpBBNew(-hw, -hh, hw, hh));
	
	ChipmunkDebugDrawPointLineScale = 1.0f/(float)view_scale;
	ChipmunkDebugDrawVPMatrix = cpTransformMult(projection_matrix, view_matrix);
	
	Update();
	
	// Save the drawing commands from the most recent tick.
	ChipmunkDebugDrawPushRenderer();
	ChipmunkDemoTextPushRenderer();
	demos[demo_index].drawFunc(space);
	
//	// Highlight the shape under the mouse because it looks neat.
//	cpShape *nearest = cpSpacePointQueryNearest(space, ChipmunkDemoMouse, 0.0f, CP_ALL_LAYERS, CP_NO_GROUP, NULL);
//	if(nearest) ChipmunkDebugDrawShape(nearest, RGBAColor(1.0f, 0.0f, 0.0f, 1.0f), LAColor(0.0f, 0.0f));
	
	sg_pass_action action = {
		.colors[0] = {.action = SG_ACTION_CLEAR, .val = {0x07/255.0f, 0x36/255.0f, 0x42/255.0f}},
	};
	sg_begin_default_pass(&action, (int)screen_size.x, (int)screen_size.y);
	
	// Draw the renderer contents and reset it back to the last tick's state.
	ChipmunkDebugDrawFlushRenderer();
	
	// // Now render all the UI text.
	DrawInstructions();
	DrawInfo();
	
	ChipmunkDemoTextMatrix = projection_matrix;
	ChipmunkDemoTextFlushRenderer();
	
	ChipmunkDebugDrawPopRenderer();
	ChipmunkDemoTextPopRenderer();
	
	sg_end_pass();
	sg_commit();
}

static void
RunDemo(int index)
{
	srand(45073);
	
	demo_index = index;
	
	ChipmunkDemoTicks = 0;
	ChipmunkDemoTime = 0.0;
	Accumulator = 0.0;
	LastTime = stm_sec(stm_now());
	
	mouse_joint = NULL;
	ChipmunkDemoMessageString = "";
	max_arbiters = 0;
	max_points = 0;
	max_constraints = 0;
	space = demos[demo_index].initFunc();

	// Not supported by Sokol yet.
	// static char title[1024];
	// sprintf(title, "Demo(%c): %s", 'a' + demo_index, demos[demo_index].name);
}

static void
Keyboard(const sapp_event *event)
{
	float translate_increment = 50.0f/(float)view_scale;
	float scale_increment = 1.2f;
	
	if(event->type == SAPP_EVENTTYPE_CHAR && !event->key_repeat){
		int index = event->char_code - 'a';
		if(0 <= index && index < demo_count){
			demos[demo_index].destroyFunc(space);
			RunDemo(index);
		}
	} else if(event->type == SAPP_EVENTTYPE_KEY_DOWN){
		switch(event->key_code){
			case SAPP_KEYCODE_SPACE:{
				if(!event->key_repeat){
					demos[demo_index].destroyFunc(space);
					RunDemo(demo_index);
				}
			} break;
			case SAPP_KEYCODE_GRAVE_ACCENT : {
				if(!event->key_repeat) paused = !paused;
			} break;
			case SAPP_KEYCODE_1: {
				step = cpTrue;
			} break;
			
			case SAPP_KEYCODE_KP_4: view_translate.x += translate_increment; break;
			case SAPP_KEYCODE_KP_6: view_translate.x -= translate_increment; break;
			case SAPP_KEYCODE_KP_2: view_translate.y += translate_increment; break;
			case SAPP_KEYCODE_KP_8: view_translate.y -= translate_increment; break;
			case SAPP_KEYCODE_KP_7: view_scale /= scale_increment; break;
			case SAPP_KEYCODE_KP_9: view_scale *= scale_increment; break;
			case SAPP_KEYCODE_KP_5: {
				view_translate.x = 0.0f;
				view_translate.y = 0.0f;
				view_scale = 1.0f;
			} break;
			
			default: break;
		}
	}
	
	if(!event->key_repeat){
		switch(event->key_code){
			case SAPP_KEYCODE_UP    : ChipmunkDemoKeyboard.y += (event->type == SAPP_EVENTTYPE_KEY_DOWN ?  1.0 : -1.0); break;
			case SAPP_KEYCODE_DOWN  : ChipmunkDemoKeyboard.y += (event->type == SAPP_EVENTTYPE_KEY_DOWN ? -1.0 :  1.0); break;
			case SAPP_KEYCODE_LEFT  : ChipmunkDemoKeyboard.x += (event->type == SAPP_EVENTTYPE_KEY_DOWN ? -1.0 :  1.0); break;
			case SAPP_KEYCODE_RIGHT : ChipmunkDemoKeyboard.x += (event->type == SAPP_EVENTTYPE_KEY_DOWN ?  1.0 : -1.0); break;
			default: break;
		}
	}
}

static cpVect
MouseToSpace(const sapp_event *event)
{
	// Calculate clip coord for mouse.
	cpVect screen_size = cpv(sapp_width(), sapp_height());
	cpVect clip_coord = cpv(2*event->mouse_x/screen_size.x - 1, 1 - 2*event->mouse_y/screen_size.y);
	
	// Use the VP matrix to transform to world space.
	cpTransform vp_inverse = cpTransformInverse(ChipmunkDebugDrawVPMatrix);
	return cpTransformPoint(vp_inverse, clip_coord);
}

static void
Click(const sapp_event *event)
{
	cpVect mouse_pos = MouseToSpace(event);
	
	if(event->mouse_button == SAPP_MOUSEBUTTON_LEFT){
		if(event->type == SAPP_EVENTTYPE_MOUSE_DOWN){
			// give the mouse click a little radius to make it easier to click small shapes.
			cpFloat radius = 5.0;
			
			cpPointQueryInfo info = {0};
			cpShape *shape = cpSpacePointQueryNearest(space, mouse_pos, radius, GRAB_FILTER, &info);
			
			if(shape && cpBodyGetMass(cpShapeGetBody(shape)) < INFINITY){
				// Use the closest point on the surface if the click is outside of the shape.
				cpVect nearest = (info.distance > 0.0f ? info.point : mouse_pos);
				
				cpBody *body = cpShapeGetBody(shape);
				mouse_joint = cpPivotJointNew2(mouse_body, body, cpvzero, cpBodyWorldToLocal(body, nearest));
				mouse_joint->maxForce = 50000.0f;
				mouse_joint->errorBias = cpfpow(1.0f - 0.15f, 60.0f);
				cpSpaceAddConstraint(space, mouse_joint);
			}
		} else if(mouse_joint){
			cpSpaceRemoveConstraint(space, mouse_joint);
			cpConstraintFree(mouse_joint);
			mouse_joint = NULL;
		}
	} else if(event->mouse_button == SAPP_MOUSEBUTTON_RIGHT){
		ChipmunkDemoRightDown = ChipmunkDemoRightClick = (event->type == SAPP_EVENTTYPE_MOUSE_DOWN);
	}
}

static void
Event(const sapp_event *event)
{
	switch(event->type){
		case SAPP_EVENTTYPE_CHAR:
		case SAPP_EVENTTYPE_KEY_UP:
		case SAPP_EVENTTYPE_KEY_DOWN: {
			Keyboard(event);
		} break;
		
		case SAPP_EVENTTYPE_MOUSE_MOVE: {
			ChipmunkDemoMouse = MouseToSpace(event);
		}; break;
		
		case SAPP_EVENTTYPE_MOUSE_UP:
		case SAPP_EVENTTYPE_MOUSE_DOWN: {
			Click(event);
		} break;
		
		default: break;
	}
}

static void
TimeTrial(int index, int count)
{
	space = demos[index].initFunc();
	
	double start_time = stm_sec(stm_now());
	double dt = demos[index].timestep;
	
	for(int i=0; i<count; i++)
		demos[index].updateFunc(space, dt);
	
	double end_time = stm_sec(stm_now());
	
	demos[index].destroyFunc(space);
	
	printf("Time(%c) = %8.2f ms (%s)\n", index + 'a', (end_time - start_time)*1e3f, demos[index].name);
	fflush(stdout);
}

extern ChipmunkDemo LogoSmash;
extern ChipmunkDemo PyramidStack;
extern ChipmunkDemo Plink;
extern ChipmunkDemo BouncyHexagons;
extern ChipmunkDemo Tumble;
extern ChipmunkDemo PyramidTopple;
extern ChipmunkDemo Planet;
extern ChipmunkDemo Springies;
extern ChipmunkDemo Pump;
extern ChipmunkDemo TheoJansen;
extern ChipmunkDemo Query;
extern ChipmunkDemo OneWay;
extern ChipmunkDemo Player;
extern ChipmunkDemo Joints;
extern ChipmunkDemo Tank;
extern ChipmunkDemo Chains;
extern ChipmunkDemo Crane;
extern ChipmunkDemo Buoyancy;
extern ChipmunkDemo ContactGraph;
extern ChipmunkDemo Slice;
extern ChipmunkDemo Convex;
extern ChipmunkDemo Unicycle;
extern ChipmunkDemo Sticky;
extern ChipmunkDemo Shatter;
extern ChipmunkDemo GJK;

extern ChipmunkDemo bench_list[];
extern int bench_count;

static void
Init(void)
{
	sg_desc desc = {0};
	sg_setup(&desc);
	cpAssertHard(sg_isvalid(), "Could not init Sokol GFX.");
	
	ChipmunkDebugDrawInit();
	ChipmunkDemoTextInit();
	
	mouse_body = cpBodyNewKinematic();
	RunDemo(demo_index);
}

void Cleanup(void) {
	sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
	demos[ 0] = LogoSmash; //A
	demos[ 1] = PyramidStack; //B
	demos[ 2] = Plink; //C
	demos[ 3] = BouncyHexagons; //D
	demos[ 4] = Tumble; //E
	demos[ 5] = PyramidTopple; //F
	demos[ 6] = Planet; //G
	demos[ 7] = Springies; //H
	demos[ 8] = Pump; //I
	demos[ 9] = TheoJansen; //J
	demos[10] = Query; //K
	demos[11] = OneWay; //L
	demos[12] = Joints; //M
	demos[13] = Tank; //N
	demos[14] = Chains; //O
	demos[15] = Crane; //P
	demos[16] = ContactGraph; //Q
	demos[17] = Buoyancy; //R
	demos[18] = Player; //S
	demos[19] = Slice; //T
	demos[20] = Convex; //U
	demos[21] = Unicycle; //V
	demos[22] = Sticky; //W
	demos[23] = Shatter; //X
	demo_count = 24;
	
	int trial = 0;
	for(int i=0; i<argc; i++){
		if(strcmp(argv[i], "-bench") == 0){
			memcpy(demos, bench_list, bench_count*sizeof(ChipmunkDemo));
			demo_count = bench_count;
		} else if(strcmp(argv[i], "-trial") == 0){
			trial = 1;
		}
	}
	
	stm_setup();
	if(trial){
		for(int i=0; i<demo_count; i++) TimeTrial(i, 1000);
		exit(0);
	} else {
		return (sapp_desc){
			.init_cb = Init,
			.frame_cb = Display,
			.event_cb = Event,
			.cleanup_cb = Cleanup,
			.width = 1024,
			.height = 768,
			.high_dpi = true,
			.window_title = "Chipmunk2D",
		};
	}
}
