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

VALUE m_Chipmunk;

ID id_parent;

static VALUE
rb_get_cp_bias_coef(VALUE self)
{
	return rb_float_new(cp_bias_coef);
}

static VALUE
rb_set_cp_bias_coef(VALUE self, VALUE num)
{
	cp_bias_coef = NUM2DBL(num);
	return num;
}

static VALUE
rb_get_cp_collision_slop(VALUE self)
{
	return rb_float_new(cp_collision_slop);
}

static VALUE
rb_set_cp_collision_slop(VALUE self, VALUE num)
{
	cp_collision_slop = NUM2DBL(num);
	return num;
}

static VALUE
rb_momentForCircle(VALUE self, VALUE m, VALUE r1, VALUE r2, VALUE offset)
{
	cpFloat i = cpMomentForCircle(NUM2DBL(m), NUM2DBL(r1), NUM2DBL(r2), *VGET(offset));
	return rb_float_new(i);
}

static VALUE
rb_momentForPoly(VALUE self, VALUE m, VALUE arr, VALUE offset)
{
	Check_Type(arr, T_ARRAY);
	int numVerts = RARRAY(arr)->len;
	cpVect verts[numVerts];
	
	for(int i=0; i<numVerts; i++)
		verts[i] = *VGET(RARRAY(arr)->ptr[i]);
	
	cpFloat inertia = cpMomentForPoly(NUM2DBL(m), numVerts, verts, *VGET(offset));
	return rb_float_new(inertia);
}

static VALUE
rb_dampedSpring(VALUE self, VALUE a, VALUE b, VALUE r1, VALUE r2, VALUE len, VALUE k, VALUE dmp, VALUE dt)
{
	cpDampedSpring(BODY(a), BODY(b), *VGET(r1), *VGET(r2), NUM2DBL(len), NUM2DBL(k), NUM2DBL(dmp), NUM2DBL(dt));
	return Qnil;
}

void
Init_chipmunk(void)
{
	id_parent = rb_intern("parent");
	
	cpInitChipmunk();
	
	m_Chipmunk = rb_define_module("CP");
	rb_define_module_function(m_Chipmunk, "bias_coef", rb_get_cp_bias_coef, 0);
	rb_define_module_function(m_Chipmunk, "bias_coef=", rb_set_cp_bias_coef, 1);
	rb_define_module_function(m_Chipmunk, "collision_slop", rb_get_cp_collision_slop, 0);
	rb_define_module_function(m_Chipmunk, "collision_slop=", rb_set_cp_collision_slop, 1);
	
	rb_define_module_function(m_Chipmunk, "moment_for_circle", rb_momentForCircle, 4);
	rb_define_module_function(m_Chipmunk, "moment_for_poly", rb_momentForPoly, 3);
	
	rb_define_module_function(m_Chipmunk, "damped_spring", rb_dampedSpring, 8);
	
	Init_cpVect();
	Init_cpBB();
	Init_cpBody();
	Init_cpShape();
	Init_cpJoint();
	Init_cpSpace();
}
