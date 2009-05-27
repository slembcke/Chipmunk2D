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

VALUE c_cpVect;

static VALUE
rb_cpVectForAngle(VALUE self, VALUE angle)
{
	return VNEW(cpvforangle(NUM2DBL(angle)));
}

static VALUE
rb_cpVectAlloc(VALUE klass)
{
	cpVect *v = malloc(sizeof(cpVect));
	return Data_Wrap_Struct(klass, NULL, free, v);
}

static VALUE
rb_cpVectInitialize(VALUE self, VALUE x, VALUE y)
{
	cpVect *v = VGET(self);
	v->x = NUM2DBL(x);
	v->y = NUM2DBL(y);
	
	return self;
}

static VALUE
rb_cpVectGetX(VALUE self)
{
	return rb_float_new(VGET(self)->x);
}

static VALUE
rb_cpVectGetY(VALUE self)
{
	return rb_float_new(VGET(self)->y);
}

static VALUE
rb_cpVectSetX(VALUE self, VALUE x)
{
	VGET(self)->x = NUM2DBL(x);
	return self;
}

static VALUE
rb_cpVectSetY(VALUE self, VALUE y)
{
	VGET(self)->y = NUM2DBL(y);
	return self;
}

static VALUE
rb_cpVectToString(VALUE self)
{
	char str[256];
	cpVect *v = VGET(self);
	
	sprintf(str, "(% .3f, % .3f)", v->x, v->y);
	
	return rb_str_new2(str);
}

static VALUE
rb_cpVectToArray(VALUE self)
{
	cpVect *v = VGET(self);
	return rb_ary_new3(2, rb_float_new(v->x), rb_float_new(v->y));
}

static VALUE
rb_cpVectToAngle(VALUE self)
{
	return rb_float_new(cpvtoangle(*VGET(self)));
}
	

static VALUE
rb_cpVectNegate(VALUE self)
{
	return VNEW(cpvneg(*VGET(self)));
}

static VALUE
rb_cpVectAdd(VALUE self, VALUE v)
{
	return VNEW(cpvadd(*VGET(self), *VGET(v)));
}

static VALUE
rb_cpVectSub(VALUE self, VALUE v)
{
	return VNEW(cpvsub(*VGET(self), *VGET(v)));
}

static VALUE
rb_cpVectSMult(VALUE self, VALUE s)
{
	return VNEW(cpvmult(*VGET(self), NUM2DBL(s)));
}

static VALUE
rb_cpVectSDiv(VALUE self, VALUE s)
{
	cpFloat factor = 1.0f/(float)NUM2DBL(s);
	return VNEW(cpvmult(*VGET(self), factor));
}

static VALUE
rb_cpVectDot(VALUE self, VALUE v)
{
	return rb_float_new(cpvdot(*VGET(self), *VGET(v)));
}

static VALUE
rb_cpVectCross(VALUE self, VALUE v)
{
	return rb_float_new(cpvcross(*VGET(self), *VGET(v)));
}

static VALUE
rb_cpVectLength(VALUE self)
{
	cpVect *v;
	Data_Get_Struct(self, cpVect, v);
	return rb_float_new(cpvlength(*v));
}

static VALUE
rb_cpVectLengthsq(VALUE self)
{
	cpVect *v;
	Data_Get_Struct(self, cpVect, v);
	return rb_float_new(cpvlengthsq(*v));
}

static VALUE
rb_cpVectNorm(VALUE self)
{
	return VNEW(cpvnormalize(*VGET(self)));
}

static VALUE
rb_cpVectNormBang(VALUE self)
{
	cpVect *v = VGET(self);
	*v = cpvnormalize(*v);
	return self;
}

static VALUE
rb_cpVectPerp(VALUE self)
{
	return VNEW(cpvperp(*VGET(self)));
}

static VALUE
rb_cpVectProject(VALUE self, VALUE v)
{
	return VNEW(cpvproject(*VGET(self), *VGET(v)));
}

static VALUE
rb_cpVectRotate(VALUE self, VALUE v)
{
	return VNEW(cpvrotate(*VGET(self), *VGET(v)));
}

static VALUE
rb_cpVectUnRotate(VALUE self, VALUE v)
{
	return VNEW(cpvunrotate(*VGET(self), *VGET(v)));
}

static VALUE
rb_cpVectNear(VALUE self, VALUE v, VALUE d)
{
	cpFloat dist = NUM2DBL(d);
	cpVect delta = cpvsub(*VGET(self), *VGET(v));
	return (cpvdot(delta, delta) <= dist*dist) ? Qtrue : Qfalse;
}

static VALUE
rb_vec2(VALUE self, VALUE x, VALUE y)
{
	return VNEW(cpv(NUM2DBL(x), NUM2DBL(y)));
}

void
Init_cpVect(void)
{
	c_cpVect = rb_define_class_under(m_Chipmunk, "Vec2", rb_cObject);
	rb_define_singleton_method(c_cpVect, "for_angle", rb_cpVectForAngle, 1);
	
	rb_define_alloc_func(c_cpVect, rb_cpVectAlloc);
	rb_define_method(c_cpVect, "initialize", rb_cpVectInitialize, 2);
	
	rb_define_method(c_cpVect, "x", rb_cpVectGetX, 0);
	rb_define_method(c_cpVect, "y", rb_cpVectGetY, 0);
	rb_define_method(c_cpVect, "x=", rb_cpVectSetX, 1);
	rb_define_method(c_cpVect, "y=", rb_cpVectSetY, 1);
	
	rb_define_method(c_cpVect, "to_s", rb_cpVectToString, 0);
	rb_define_method(c_cpVect, "to_a", rb_cpVectToArray, 0);
	rb_define_method(c_cpVect, "to_angle", rb_cpVectToAngle, 0);
	
	rb_define_method(c_cpVect, "-@", rb_cpVectNegate, 0);
	rb_define_method(c_cpVect, "+", rb_cpVectAdd, 1);
	rb_define_method(c_cpVect, "-", rb_cpVectSub, 1);
	rb_define_method(c_cpVect, "*", rb_cpVectSMult, 1);
	rb_define_method(c_cpVect, "/", rb_cpVectSDiv, 1);
	rb_define_method(c_cpVect, "dot", rb_cpVectDot, 1);
	rb_define_method(c_cpVect, "cross", rb_cpVectCross, 1);
	rb_define_method(c_cpVect, "length", rb_cpVectLength, 0);
	rb_define_method(c_cpVect, "lengthsq", rb_cpVectLengthsq, 0);
	rb_define_method(c_cpVect, "normalize", rb_cpVectNorm, 0);
	rb_define_method(c_cpVect, "normalize!", rb_cpVectNormBang, 0);
	rb_define_method(c_cpVect, "perp", rb_cpVectPerp, 0);
	rb_define_method(c_cpVect, "project", rb_cpVectProject, 1);
	rb_define_method(c_cpVect, "rotate", rb_cpVectRotate, 1);
	rb_define_method(c_cpVect, "unrotate", rb_cpVectUnRotate, 1);
	rb_define_method(c_cpVect, "near?", rb_cpVectNear, 2);
		
	rb_define_global_function("vec2", rb_vec2, 2);
}
