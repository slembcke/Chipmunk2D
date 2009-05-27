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
#include "chipmunk.h"

#include "ruby.h"
#include "rb_chipmunk.h"

VALUE c_cpBody;

static VALUE
rb_cpBodyAlloc(VALUE klass)
{
	cpBody *body = cpBodyNew(1.0f, 1.0f);
	return Data_Wrap_Struct(klass, NULL, cpBodyFree, body);
}

static VALUE
rb_cpBodyInitialize(VALUE self, VALUE m, VALUE i)
{
	cpBody *body = BODY(self);
	cpBodyInit(body, NUM2DBL(m), NUM2DBL(i));
	
	return self;
}

static VALUE
rb_cpBodyGetMass(VALUE self)
{
	return rb_float_new(BODY(self)->m);
}

static VALUE
rb_cpBodyGetMoment(VALUE self)
{
	return rb_float_new(BODY(self)->i);
}

static VALUE
rb_cpBodyGetPos(VALUE self)
{
	return VWRAP(self, &BODY(self)->p);
}

static VALUE
rb_cpBodyGetVel(VALUE self)
{
	return VWRAP(self, &BODY(self)->v);
}

static VALUE
rb_cpBodyGetForce(VALUE self)
{
	return VWRAP(self, &BODY(self)->f);
}

static VALUE
rb_cpBodyGetAngle(VALUE self)
{
	return rb_float_new(BODY(self)->a);
}

static VALUE
rb_cpBodyGetAVel(VALUE self)
{
	return rb_float_new(BODY(self)->w);
}

static VALUE
rb_cpBodyGetTorque(VALUE self)
{
	return rb_float_new(BODY(self)->t);
}

static VALUE
rb_cpBodyGetRot(VALUE self)
{
	return VWRAP(self, &BODY(self)->rot);
}


static VALUE
rb_cpBodySetMass(VALUE self, VALUE val)
{
	cpBodySetMass(BODY(self), NUM2DBL(val));
	return val;
}

static VALUE
rb_cpBodySetMoment(VALUE self, VALUE val)
{
	cpBodySetMoment(BODY(self), NUM2DBL(val));
	return val;
}

static VALUE
rb_cpBodySetPos(VALUE self, VALUE val)
{
	BODY(self)->p = *VGET(val);
	return val;
}

static VALUE
rb_cpBodySetVel(VALUE self, VALUE val)
{
	BODY(self)->v = *VGET(val);
	return val;
}

static VALUE
rb_cpBodySetForce(VALUE self, VALUE val)
{
	BODY(self)->f = *VGET(val);
	return val;
}

static VALUE
rb_cpBodySetAngle(VALUE self, VALUE val)
{
	cpBodySetAngle(BODY(self), NUM2DBL(val));
	return val;
}

static VALUE
rb_cpBodySetAVel(VALUE self, VALUE val)
{
	BODY(self)->w = NUM2DBL(val);
	return val;
}

static VALUE
rb_cpBodySetTorque(VALUE self, VALUE val)
{
	BODY(self)->t = NUM2DBL(val);
	return val;
}

static VALUE
rb_cpBodyLocal2World(VALUE self, VALUE v)
{
	return VNEW(cpBodyLocal2World(BODY(self), *VGET(v)));
}

static VALUE
rb_cpBodyWorld2Local(VALUE self, VALUE v)
{
	return VNEW(cpBodyWorld2Local(BODY(self), *VGET(v)));
}

static VALUE
rb_cpBodyResetForces(VALUE self)
{
	cpBodyResetForces(BODY(self));
	return Qnil;
}

static VALUE
rb_cpBodyApplyForce(VALUE self, VALUE f, VALUE r)
{
	cpBodyApplyForce(BODY(self), *VGET(f), *VGET(r));
	return Qnil;
}

static VALUE
rb_cpBodyApplyImpulse(VALUE self, VALUE j, VALUE r)
{
	cpBodyApplyImpulse(BODY(self), *VGET(j), *VGET(r));
	return Qnil;
}

static VALUE
rb_cpBodyUpdateVelocity(VALUE self, VALUE g, VALUE dmp, VALUE dt)
{
	cpBodyUpdateVelocity(BODY(self), *VGET(g), NUM2DBL(dmp), NUM2DBL(dt));
	return Qnil;
}

static VALUE
rb_cpBodyUpdatePosition(VALUE self, VALUE dt)
{
	cpBodyUpdatePosition(BODY(self), NUM2DBL(dt));
	return Qnil;
}


void
Init_cpBody(void)
{
	c_cpBody = rb_define_class_under(m_Chipmunk, "Body", rb_cObject);
	rb_define_alloc_func(c_cpBody, rb_cpBodyAlloc);
	rb_define_method(c_cpBody, "initialize", rb_cpBodyInitialize, 2);

	rb_define_method(c_cpBody, "m" , rb_cpBodyGetMass, 0);
	rb_define_method(c_cpBody, "i" , rb_cpBodyGetMoment, 0);	
	rb_define_method(c_cpBody, "p" , rb_cpBodyGetPos, 0);
	rb_define_method(c_cpBody, "v" , rb_cpBodyGetVel, 0);
	rb_define_method(c_cpBody, "f" , rb_cpBodyGetForce, 0);
	rb_define_method(c_cpBody, "a" , rb_cpBodyGetAngle, 0);
	rb_define_method(c_cpBody, "w" , rb_cpBodyGetAVel, 0);
	rb_define_method(c_cpBody, "t" , rb_cpBodyGetTorque, 0);
	rb_define_method(c_cpBody, "rot", rb_cpBodyGetRot, 0);
	
	rb_define_method(c_cpBody, "m=", rb_cpBodySetMass, 1);
	rb_define_method(c_cpBody, "i=", rb_cpBodySetMoment, 1);
	rb_define_method(c_cpBody, "p=", rb_cpBodySetPos, 1);
	rb_define_method(c_cpBody, "v=", rb_cpBodySetVel, 1);
	rb_define_method(c_cpBody, "f=", rb_cpBodySetForce, 1);
	rb_define_method(c_cpBody, "a=", rb_cpBodySetAngle, 1);
	rb_define_method(c_cpBody, "w=", rb_cpBodySetAVel, 1);
	rb_define_method(c_cpBody, "t=", rb_cpBodySetTorque, 1);
	
	rb_define_method(c_cpBody, "mass" , rb_cpBodyGetMass, 0);
	rb_define_method(c_cpBody, "moment" , rb_cpBodyGetMoment, 0);	
	rb_define_method(c_cpBody, "pos" , rb_cpBodyGetPos, 0);
	rb_define_method(c_cpBody, "vel" , rb_cpBodyGetVel, 0);
	rb_define_method(c_cpBody, "force" , rb_cpBodyGetForce, 0);
	rb_define_method(c_cpBody, "angle" , rb_cpBodyGetAngle, 0);
	rb_define_method(c_cpBody, "ang_vel" , rb_cpBodyGetAVel, 0);
	rb_define_method(c_cpBody, "torque" , rb_cpBodyGetTorque, 0);
	rb_define_method(c_cpBody, "rot", rb_cpBodyGetRot, 0);
	
	rb_define_method(c_cpBody, "mass=", rb_cpBodySetMass, 1);
	rb_define_method(c_cpBody, "moment=", rb_cpBodySetMoment, 1);
	rb_define_method(c_cpBody, "pos=", rb_cpBodySetPos, 1);
	rb_define_method(c_cpBody, "vel=", rb_cpBodySetVel, 1);
	rb_define_method(c_cpBody, "force=", rb_cpBodySetForce, 1);
	rb_define_method(c_cpBody, "angle=", rb_cpBodySetAngle, 1);
	rb_define_method(c_cpBody, "ang_vel=", rb_cpBodySetAVel, 1);
	rb_define_method(c_cpBody, "torque=", rb_cpBodySetTorque, 1);
	
	rb_define_method(c_cpBody, "local2world", rb_cpBodyLocal2World, 1);
	rb_define_method(c_cpBody, "world2local", rb_cpBodyWorld2Local, 1);

	rb_define_method(c_cpBody, "reset_forces", rb_cpBodyResetForces, 0);
	rb_define_method(c_cpBody, "apply_force", rb_cpBodyApplyForce, 2);
	rb_define_method(c_cpBody, "apply_impulse", rb_cpBodyApplyImpulse, 2);
	
	rb_define_method(c_cpBody, "update_velocity", rb_cpBodyUpdateVelocity, 3);
	rb_define_method(c_cpBody, "update_position", rb_cpBodyUpdatePosition, 1);
	
	// TODO integration functions?
}
