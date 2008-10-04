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

VALUE m_cpConstraint;

static VALUE
rb_cpPinConstraintAlloc(VALUE klass)
{
	cpPinConstraint *constraint = cpPinConstraintAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpConstraintFree, constraint);
	
	return self;
}

static VALUE
rb_cpPinConstraintInit(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2)
{
	cpPinConstraint *constraint = (cpPinConstraint *)CONSTRAINT(self);
	cpPinConstraintInit(constraint, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}


static VALUE
rb_cpSlideConstraintAlloc(VALUE klass)
{
	cpSlideConstraint *constraint = cpSlideConstraintAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpConstraintFree, constraint);
	
	return self;
}

static VALUE
rb_cpSlideConstraintInit(VALUE self, VALUE a, VALUE b, VALUE anchr1, VALUE anchr2, VALUE min, VALUE max)
{
	cpSlideConstraint *constraint = (cpSlideConstraint *)CONSTRAINT(self);
	cpSlideConstraintInit(constraint, BODY(a), BODY(b), *VGET(anchr1), *VGET(anchr2), NUM2DBL(min), NUM2DBL(max));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}


static VALUE
rb_cpPivotConstraintAlloc(VALUE klass)
{
	cpPivotConstraint *constraint = cpPivotConstraintAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpConstraintFree, constraint);
	
	return self;
}

static VALUE
rb_cpPivotConstraintInit(VALUE self, VALUE a, VALUE b, VALUE pivot)
{
	cpPivotConstraint *constraint = (cpPivotConstraint *)CONSTRAINT(self);
	cpPivotConstraintInit(constraint, BODY(a), BODY(b), *VGET(pivot));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}


static VALUE
rb_cpGrooveConstraintAlloc(VALUE klass)
{
	cpGrooveConstraint *constraint = cpGrooveConstraintAlloc();
	VALUE self = Data_Wrap_Struct(klass, NULL, cpConstraintFree, constraint);
	
	return self;
}

static VALUE
rb_cpGrooveConstraintInit(VALUE self, VALUE a, VALUE b, VALUE grv_a, VALUE grv_b, VALUE anchr2)
{
	cpGrooveConstraint *constraint = (cpGrooveConstraint *)CONSTRAINT(self);
	cpGrooveConstraintInit(constraint, BODY(a), BODY(b), *VGET(grv_a), *VGET(grv_b), *VGET(anchr2));
	rb_iv_set(self, "body_a", a);
	rb_iv_set(self, "body_b", b);
	
	return self;
}

void
Init_cpConstraint(void)
{
	m_cpConstraint = rb_define_module_under(m_Chipmunk, "Constraint");	
	
	VALUE c_cpPinConstraint = rb_define_class_under(m_cpConstraint, "Pin", rb_cObject);
	rb_include_module(c_cpPinConstraint, m_cpConstraint);
	rb_define_alloc_func(c_cpPinConstraint, rb_cpPinConstraintAlloc);
	rb_define_method(c_cpPinConstraint, "initialize", rb_cpPinConstraintInit, 4);
	
	VALUE c_cpSlideConstraint = rb_define_class_under(m_cpConstraint, "Slide", rb_cObject);
	rb_include_module(c_cpSlideConstraint, m_cpConstraint);
	rb_define_alloc_func(c_cpSlideConstraint, rb_cpSlideConstraintAlloc);
	rb_define_method(c_cpSlideConstraint, "initialize", rb_cpSlideConstraintInit, 6);
	
	VALUE c_cpPivotConstraint = rb_define_class_under(m_cpConstraint, "Pivot", rb_cObject);
	rb_include_module(c_cpPivotConstraint, m_cpConstraint);
	rb_define_alloc_func(c_cpPivotConstraint, rb_cpPivotConstraintAlloc);
	rb_define_method(c_cpPivotConstraint, "initialize", rb_cpPivotConstraintInit, 3);
	
	VALUE c_cpGrooveConstraint = rb_define_class_under(m_cpConstraint, "Groove", rb_cObject);
	rb_include_module(c_cpGrooveConstraint, m_cpConstraint);
	rb_define_alloc_func(c_cpGrooveConstraint, rb_cpGrooveConstraintAlloc);
	rb_define_method(c_cpGrooveConstraint, "initialize", rb_cpGrooveConstraintInit, 5);
}
