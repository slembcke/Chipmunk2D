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

#include "chipmunk.h"

#include "ruby.h"
#include "rb_chipmunk.h"

VALUE m_cpJoint;

static VALUE
rb_cpPinJointAlloc(VALUE klass)
{
	cpPinJoint *joint = cpPinJointAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpJointFree, joint);
	
	return self;
}

static VALUE
rb_cpPinJointInit(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2)
{
	cpPinJoint *joint = (cpPinJoint *)JOINT(self);
	cpPinJointInit(joint, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}


static VALUE
rb_cpSlideJointAlloc(VALUE klass)
{
	cpSlideJoint *joint = cpSlideJointAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpJointFree, joint);
	
	return self;
}

static VALUE
rb_cpSlideJointInit(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2, VALUE min, VALUE max)
{
	cpSlideJoint *joint = (cpSlideJoint *)JOINT(self);
	cpSlideJointInit(joint, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2), NUM2DBL(min), NUM2DBL(max));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}


static VALUE
rb_cpPivotJointAlloc(VALUE klass)
{
	cpPivotJoint *joint = cpPivotJointAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpJointFree, joint);
	
	return self;
}

static VALUE
rb_cpPivotJointInit(VALUE self, VALUE a, VALUE b, VALUE pivot)
{
	cpPivotJoint *joint = (cpPivotJoint *)JOINT(self);
	cpPivotJointInit(joint, BODY(a), BODY(b), *VGET(pivot));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}


static VALUE
rb_cpGrooveJointAlloc(VALUE klass)
{
	cpGrooveJoint *joint = cpGrooveJointAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpJointFree, joint);
	
	return self;
}

static VALUE
rb_cpGrooveJointInit(VALUE self, VALUE a, VALUE b, VALUE grv_a, VALUE grv_b, VALUE anchr2)
{
	cpGrooveJoint *joint = (cpGrooveJoint *)JOINT(self);
	cpGrooveJointInit(joint, BODY(a), BODY(b), *VGET(grv_a), *VGET(grv_b), *VGET(anchr2));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}

void
Init_cpJoint(void)
{
	m_cpJoint = rb_define_module_under(m_Chipmunk, "Joint");	
	
	VALUE c_cpPinJoint = rb_define_class_under(m_cpJoint, "Pin", rb_cObject);
	rb_include_module(c_cpPinJoint, m_cpJoint);
	rb_define_alloc_func(c_cpPinJoint, rb_cpPinJointAlloc);
	rb_define_method(c_cpPinJoint, "initialize", rb_cpPinJointInit, 4);
	
	VALUE c_cpSlideJoint = rb_define_class_under(m_cpJoint, "Slide", rb_cObject);
	rb_include_module(c_cpSlideJoint, m_cpJoint);
	rb_define_alloc_func(c_cpSlideJoint, rb_cpSlideJointAlloc);
	rb_define_method(c_cpSlideJoint, "initialize", rb_cpSlideJointInit, 6);
	
	VALUE c_cpPivotJoint = rb_define_class_under(m_cpJoint, "Pivot", rb_cObject);
	rb_include_module(c_cpPivotJoint, m_cpJoint);
	rb_define_alloc_func(c_cpPivotJoint, rb_cpPivotJointAlloc);
	rb_define_method(c_cpPivotJoint, "initialize", rb_cpPivotJointInit, 3);
	
	VALUE c_cpGrooveJoint = rb_define_class_under(m_cpJoint, "Groove", rb_cObject);
	rb_include_module(c_cpGrooveJoint, m_cpJoint);
	rb_define_alloc_func(c_cpGrooveJoint, rb_cpGrooveJointAlloc);
	rb_define_method(c_cpGrooveJoint, "initialize", rb_cpGrooveJointInit, 5);
}
