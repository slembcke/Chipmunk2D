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

VALUE m_cpConstraint;

#define CONSTRAINT_GETSET_FUNCS(member) \
static VALUE rb_cpConstraint_get_##member(VALUE self) \
{return rb_float_new(CONSTRAINT(self)->member);} \
static VALUE rb_cpConstraint_set_##member(VALUE self, VALUE value) \
{CONSTRAINT(self)->member = NUM2DBL(value); return value;}

CONSTRAINT_GETSET_FUNCS(maxForce)
CONSTRAINT_GETSET_FUNCS(biasCoef)
CONSTRAINT_GETSET_FUNCS(maxBias)


#define MAKE_FLT_GETTER(func) \
static VALUE rb_##func(VALUE self) \
{return rb_float_new(func(CONSTRAINT(self)));}

#define MAKE_FLT_SETTER(func) \
static VALUE rb_##func(VALUE self, VALUE value) \
{func(CONSTRAINT(self), NUM2DBL(value)); return value;} \

#define MAKE_FLT_ACCESSORS(s, m) \
MAKE_FLT_GETTER(s##Get##m) \
MAKE_FLT_SETTER(s##Set##m)

#define MAKE_VEC_GETTER(func) \
static VALUE rb_##func(VALUE self) \
{return VNEW(func(CONSTRAINT(self)));}

#define MAKE_VEC_SETTER(func) \
static VALUE rb_##func(VALUE self, VALUE value) \
{func(CONSTRAINT(self), *VGET(value)); return value;} \

#define MAKE_VEC_ACCESSORS(s, m) \
MAKE_VEC_GETTER(s##Get##m) \
MAKE_VEC_SETTER(s##Set##m)


#define ALLOC_TEMPLATE(s, alloc) \
static VALUE rb_##s##_alloc(VALUE klass) \
{return Data_Wrap_Struct(klass, NULL, cpConstraintFree, alloc);}

ALLOC_TEMPLATE(cpPinJoint, cpPinJointAlloc())

static VALUE
rb_cpPinJoint_init(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2)
{
	cpPinJoint *joint = (cpPinJoint *)CONSTRAINT(self);
	cpPinJointInit(joint, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_FLT_ACCESSORS(cpPinJoint, Dist)
MAKE_VEC_ACCESSORS(cpPinJoint, Anchr1)
MAKE_VEC_ACCESSORS(cpPinJoint, Anchr2)


ALLOC_TEMPLATE(cpDampedRotarySpring, cpDampedRotarySpringAlloc())

static VALUE
rb_cpDampedRotarySpring_init(VALUE self, VALUE a, VALUE b, VALUE restAngle, VALUE stiffness, VALUE damping)
{
	cpDampedRotarySpring *spring = (cpDampedRotarySpring *)CONSTRAINT(self);
	cpDampedRotarySpringInit(spring, BODY(a), BODY(b), NUM2DBL(restAngle), NUM2DBL(stiffness), NUM2DBL(damping));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_FLT_ACCESSORS(cpDampedRotarySpring, RestAngle);
MAKE_FLT_ACCESSORS(cpDampedRotarySpring, Stiffness);
MAKE_FLT_ACCESSORS(cpDampedRotarySpring, Damping);


ALLOC_TEMPLATE(cpDampedSpring, cpDampedSpringAlloc())

static VALUE
rb_cpDampedSpring_init(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2, VALUE restLength, VALUE stiffness, VALUE damping)
{
	cpDampedSpring *spring = (cpDampedSpring *)CONSTRAINT(self);
	cpDampedSpringInit(spring, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2), NUM2DBL(restLength), NUM2DBL(stiffness), NUM2DBL(damping));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_VEC_ACCESSORS(cpDampedSpring, Anchr1)
MAKE_VEC_ACCESSORS(cpDampedSpring, Anchr2)
MAKE_FLT_ACCESSORS(cpDampedSpring, RestLength);
MAKE_FLT_ACCESSORS(cpDampedSpring, Stiffness);
MAKE_FLT_ACCESSORS(cpDampedSpring, Damping);


ALLOC_TEMPLATE(cpGearJoint, cpGearJointAlloc())

static VALUE
rb_cpGearJoint_init(VALUE self, VALUE a, VALUE b, VALUE phase, VALUE ratio)
{
	cpGearJoint *joint = (cpGearJoint *)CONSTRAINT(self);
	cpGearJointInit(joint, BODY(a), BODY(b), NUM2DBL(phase), NUM2DBL(ratio));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_FLT_ACCESSORS(cpGearJoint, Phase);
MAKE_FLT_ACCESSORS(cpGearJoint, Ratio);


ALLOC_TEMPLATE(cpPivotJoint, cpPivotJointAlloc())

static VALUE
rb_cpPivotJoint_init(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2)
{
	cpPivotJoint *joint = (cpPivotJoint *)CONSTRAINT(self);
	cpPivotJointInit(joint, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_VEC_ACCESSORS(cpPivotJoint, Anchr1);
MAKE_VEC_ACCESSORS(cpPivotJoint, Anchr2);


ALLOC_TEMPLATE(cpRotaryLimitJoint, cpRotaryLimitJointAlloc())

static VALUE
rb_cpRotaryLimitJoint_init(VALUE self, VALUE a, VALUE b, VALUE min, VALUE max)
{
	cpRotaryLimitJoint *joint = (cpRotaryLimitJoint *)CONSTRAINT(self);
	cpRotaryLimitJointInit(joint, BODY(a), BODY(b), NUM2DBL(min), NUM2DBL(max));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_FLT_ACCESSORS(cpRotaryLimitJoint, Min);
MAKE_FLT_ACCESSORS(cpRotaryLimitJoint, Max);


ALLOC_TEMPLATE(cpSimpleMotor, cpSimpleMotorAlloc())

static VALUE
rb_cpSimpleMotor_init(VALUE self, VALUE a, VALUE b, VALUE rate)
{
	cpSimpleMotor *motor = (cpSimpleMotor *)CONSTRAINT(self);
	cpSimpleMotorInit(motor, BODY(a), BODY(b), NUM2DBL(rate));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_FLT_ACCESSORS(cpSimpleMotor, Rate);


ALLOC_TEMPLATE(cpSlideJoint, cpSlideJointAlloc())

static VALUE
rb_cpSlideJoint_init(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2, VALUE min, VALUE max)
{
	cpSlideJoint *joint = (cpSlideJoint *)CONSTRAINT(self);
	cpSlideJointInit(joint, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2), NUM2DBL(min), NUM2DBL(max));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_VEC_ACCESSORS(cpSlideJoint, Anchr1)
MAKE_VEC_ACCESSORS(cpSlideJoint, Anchr2)
MAKE_FLT_ACCESSORS(cpSlideJoint, Min);
MAKE_FLT_ACCESSORS(cpSlideJoint, Max);


ALLOC_TEMPLATE(cpGrooveJoint, cpGrooveJointAlloc())

static VALUE
rb_cpGrooveJoint_init(VALUE self, VALUE a, VALUE b, VALUE grv_a, VALUE grv_b, VALUE anchr2)
{
	cpGrooveJoint *joint = (cpGrooveJoint *)CONSTRAINT(self);
	cpGrooveJointInit(joint, BODY(a), BODY(b), *VGET(grv_a), *VGET(grv_b), *VGET(anchr2));
	rb_iv_set(self, "@body_a", a);
	rb_iv_set(self, "@body_b", b);
	
	return self;
}

MAKE_VEC_ACCESSORS(cpGrooveJoint, Anchr2)
// TODO more accessors


#define STRINGIFY(v) #v
#define ACCESSOR_METHODS(s, m, name) \
rb_define_method(c_##s, STRINGIFY(name), rb_##s##Get##m, 0); \
rb_define_method(c_##s, STRINGIFY(name=), rb_##s##Set##m, 1);

static VALUE
make_class(char *name, void *alloc_func, void *init_func, int init_params)
{
	VALUE klass = rb_define_class_under(m_cpConstraint, name, rb_cObject);
	rb_include_module(klass, m_cpConstraint);
	rb_define_alloc_func(klass, alloc_func);
	rb_define_method(klass, "initialize", init_func, init_params);
	
	return klass;
}

void
Init_cpConstraint(void)
{
	m_cpConstraint = rb_define_module_under(m_Chipmunk, "Constraint");
	rb_define_attr(m_cpConstraint, "body_a", 1, 0);
	rb_define_attr(m_cpConstraint, "body_b", 1, 0);
	rb_define_method(m_cpConstraint, "max_force", rb_cpConstraint_get_maxForce, 0);
	rb_define_method(m_cpConstraint, "max_force=", rb_cpConstraint_set_maxForce, 1);
	rb_define_method(m_cpConstraint, "bias_coef", rb_cpConstraint_get_biasCoef, 0);
	rb_define_method(m_cpConstraint, "bias_coef=", rb_cpConstraint_set_biasCoef, 1);
	rb_define_method(m_cpConstraint, "max_bias", rb_cpConstraint_get_maxBias, 0);
	rb_define_method(m_cpConstraint, "max_bias=", rb_cpConstraint_set_maxBias, 1);
	
	VALUE c_cpPinJoint = make_class("PinJoint", rb_cpPinJoint_alloc, rb_cpPinJoint_init, 4);
	ACCESSOR_METHODS(cpPinJoint, Anchr1, anchr1)
	ACCESSOR_METHODS(cpPinJoint, Anchr2, anchr2)
	ACCESSOR_METHODS(cpPinJoint, Dist, dist)
	
	VALUE c_cpDampedRotarySpring = make_class("DampedRotarySpring", rb_cpDampedRotarySpring_alloc, rb_cpDampedRotarySpring_init, 5);
	ACCESSOR_METHODS(cpDampedRotarySpring, RestAngle, rest_angle)
	ACCESSOR_METHODS(cpDampedRotarySpring, Stiffness, stiffness)
	ACCESSOR_METHODS(cpDampedRotarySpring, Damping, damping)
	
	VALUE c_cpDampedSpring = make_class("DampedSpring", rb_cpDampedSpring_alloc, rb_cpDampedSpring_init, 7);
	ACCESSOR_METHODS(cpDampedSpring, Anchr1, anchr1)
	ACCESSOR_METHODS(cpDampedSpring, Anchr2, anchr2)
	ACCESSOR_METHODS(cpDampedSpring, RestLength, rest_length)
	ACCESSOR_METHODS(cpDampedSpring, Stiffness, stiffness)
	ACCESSOR_METHODS(cpDampedSpring, Damping, damping)
	
	VALUE c_cpGearJoint = make_class("GearJoint", rb_cpGearJoint_alloc, rb_cpGearJoint_init, 4);
	ACCESSOR_METHODS(cpGearJoint, Phase, phase)
	ACCESSOR_METHODS(cpGearJoint, Ratio, ratio)
	
	VALUE c_cpPivotJoint = make_class("PivotJoint", rb_cpPivotJoint_alloc, rb_cpPivotJoint_init, 4);
	ACCESSOR_METHODS(cpPivotJoint, Anchr1, anchr1)
	ACCESSOR_METHODS(cpPivotJoint, Anchr2, anchr2)
	
	VALUE c_cpRotaryLimitJoint = make_class("RotaryLimitJoint", rb_cpRotaryLimitJoint_alloc, rb_cpRotaryLimitJoint_init, 4);
	ACCESSOR_METHODS(cpRotaryLimitJoint, Min, min)
	ACCESSOR_METHODS(cpRotaryLimitJoint, Max, max)
	
	VALUE c_cpSimpleMotor = make_class("SimpleMotor", rb_cpSimpleMotor_alloc, rb_cpSimpleMotor_init, 3);
	ACCESSOR_METHODS(cpSimpleMotor, Rate, rate);
	
	VALUE c_cpSlideJoint = make_class("SlideJoint", rb_cpSlideJoint_alloc, rb_cpSlideJoint_init, 6);
	ACCESSOR_METHODS(cpSlideJoint, Anchr1, anchr1)
	ACCESSOR_METHODS(cpSlideJoint, Anchr2, anchr2)
	ACCESSOR_METHODS(cpSlideJoint, Min, min)
	ACCESSOR_METHODS(cpSlideJoint, Max, max)
	
	VALUE c_cpGrooveJoint = make_class("GrooveJoint", rb_cpGrooveJoint_alloc, rb_cpGrooveJoint_init, 5);
	ACCESSOR_METHODS(cpGrooveJoint, Anchr2, anchr2)
// TODO groove joint accessors
	
	// TODO breakable joint
}
