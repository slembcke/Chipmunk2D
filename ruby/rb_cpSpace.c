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

ID id_call;


VALUE c_cpSpace;

static VALUE
rb_cpSpaceAlloc(VALUE klass)
{
	cpSpace *space = cpSpaceAlloc();
	return Data_Wrap_Struct(klass, NULL, cpSpaceFree, space);
}

static VALUE
rb_cpSpaceInitialize(VALUE self)
{
	cpSpace *space = SPACE(self);
	cpSpaceInit(space);
	
	// These might as well be in one shared hash.
	rb_iv_set(self, "static_shapes", rb_ary_new());
	rb_iv_set(self, "active_shapes", rb_ary_new());
	rb_iv_set(self, "bodies", rb_ary_new());
	rb_iv_set(self, "joints", rb_ary_new());
	rb_iv_set(self, "blocks", rb_hash_new());

	return self;
}

static VALUE
rb_cpSpaceGetIterations(VALUE self)
{
	return INT2NUM(SPACE(self)->iterations);
}

static VALUE
rb_cpSpaceSetIterations(VALUE self, VALUE val)
{
	SPACE(self)->iterations = NUM2INT(val);
	return val;
}

static VALUE
rb_cpSpaceGetElasticIterations(VALUE self)
{
	return INT2NUM(SPACE(self)->elasticIterations);
}

static VALUE
rb_cpSpaceSetElasticIterations(VALUE self, VALUE val)
{
	SPACE(self)->elasticIterations = NUM2INT(val);
	return val;
}

static VALUE
rb_cpSpaceGetDamping(VALUE self)
{
	return rb_float_new(SPACE(self)->damping);
}

static VALUE
rb_cpSpaceSetDamping(VALUE self, VALUE val)
{
	SPACE(self)->damping = NUM2DBL(val);
	return val;
}

static VALUE
rb_cpSpaceGetGravity(VALUE self)
{
	return VWRAP(self, &SPACE(self)->gravity);
}

static VALUE
rb_cpSpaceSetGravity(VALUE self, VALUE val)
{
	SPACE(self)->gravity = *VGET(val);
	return val;
}

static int
collisionCallback(cpShape *a, cpShape *b, cpContact *contacts, int numContacts, cpFloat normal_coef, void *data)
{
	VALUE block = (VALUE)data;
	VALUE shapea = (VALUE)a->data;
	VALUE shapeb = (VALUE)b->data;
	
	return rb_funcall(block, id_call, 2, shapea, shapeb);
}

static VALUE
rb_cpSpaceAddCollisionFunc(int argc, VALUE *argv, VALUE self)
{
	VALUE a, b, block;
	rb_scan_args(argc, argv, "20&", &a, &b, &block);

	VALUE id_a = rb_obj_id(a);
	VALUE id_b = rb_obj_id(b);
	if(NIL_P(block)) {
		cpSpaceAddCollisionPairFunc(SPACE(self), NUM2UINT(id_a), NUM2UINT(id_b),
								    NULL, NULL);
	} else {
		cpSpaceAddCollisionPairFunc(SPACE(self), NUM2UINT(id_a), NUM2UINT(id_b),
									collisionCallback, (void *)block);
	}
	
	VALUE blocks = rb_iv_get(self, "blocks");
	rb_hash_aset(blocks, rb_ary_new3(2, id_a, id_b), block);
	
	return Qnil;
}

static VALUE
rb_cpSpaceRemoveCollisionFunc(VALUE self, VALUE a, VALUE b)
{
	VALUE id_a = rb_obj_id(a);
	VALUE id_b = rb_obj_id(b);
	cpSpaceRemoveCollisionPairFunc(SPACE(self), NUM2UINT(id_a), NUM2UINT(id_b));
	
	VALUE blocks = rb_iv_get(self, "blocks");
	rb_hash_delete(blocks, rb_ary_new3(2, id_a, id_b));
	
	return Qnil;
}

static VALUE
rb_cpSpaceSetDefaultCollisionFunc(int argc, VALUE *argv, VALUE self)
{
	VALUE block;
	rb_scan_args(argc, argv, "00&", &block);
	
	if(NIL_P(block)) {
		cpSpaceSetDefaultCollisionPairFunc(SPACE(self), NULL, NULL);
	} else {
		cpSpaceSetDefaultCollisionPairFunc(SPACE(self), collisionCallback, (void *)block);
	}
		
	rb_hash_aset(rb_iv_get(self, "blocks"), ID2SYM(rb_intern("default")), block);
	
	return Qnil;
}

static VALUE
rb_cpSpaceAddShape(VALUE self, VALUE shape)
{
	cpSpaceAddShape(SPACE(self), SHAPE(shape));
	rb_ary_push(rb_iv_get(self, "active_shapes"), shape);
	return shape;
}

static VALUE
rb_cpSpaceAddStaticShape(VALUE self, VALUE shape)
{
	cpSpaceAddStaticShape(SPACE(self), SHAPE(shape));
	rb_ary_push(rb_iv_get(self, "static_shapes"), shape);
	return shape;
}

static VALUE
rb_cpSpaceAddBody(VALUE self, VALUE body)
{
	cpSpaceAddBody(SPACE(self), BODY(body));
	rb_ary_push(rb_iv_get(self, "bodies"), body);
	return body;
}

static VALUE
rb_cpSpaceAddJoint(VALUE self, VALUE joint)
{
	cpSpaceAddJoint(SPACE(self), JOINT(joint));
	rb_ary_push(rb_iv_get(self, "joints"), joint);
	return joint;
}

static VALUE
rb_cpSpaceRemoveShape(VALUE self, VALUE shape)
{
	cpSpaceRemoveShape(SPACE(self), SHAPE(shape));
	return rb_ary_delete(rb_iv_get(self, "active_shapes"), shape);
}

static VALUE
rb_cpSpaceRemoveStaticShape(VALUE self, VALUE shape)
{
	cpSpaceRemoveStaticShape(SPACE(self), SHAPE(shape));
	return rb_ary_delete(rb_iv_get(self, "static_shapes"), shape);
}

static VALUE
rb_cpSpaceRemoveBody(VALUE self, VALUE body)
{
	cpSpaceRemoveBody(SPACE(self), BODY(body));
	return rb_ary_delete(rb_iv_get(self, "bodies"), body);
}

static VALUE
rb_cpSpaceRemoveJoint(VALUE self, VALUE joint)
{
	cpSpaceRemoveJoint(SPACE(self), JOINT(joint));
	return rb_ary_delete(rb_iv_get(self, "joints"), joint);
}

static VALUE
rb_cpSpaceResizeStaticHash(VALUE self, VALUE dim, VALUE count)
{
	cpSpaceResizeStaticHash(SPACE(self), NUM2DBL(dim), NUM2INT(count));
	return Qnil;
}

static VALUE
rb_cpSpaceResizeActiveHash(VALUE self, VALUE dim, VALUE count)
{
	cpSpaceResizeActiveHash(SPACE(self), NUM2DBL(dim), NUM2INT(count));
	return Qnil;
}

static VALUE
rb_cpSpaceRehashStatic(VALUE self)
{
	cpSpaceRehashStatic(SPACE(self));
	return Qnil;
}

static void
pointQueryHelper(cpShape *shape, void *block)
{
	rb_funcall((VALUE)block, id_call, 1, (VALUE)shape->data);
}

static VALUE
rb_cpSpaceShapePointQuery(int argc, VALUE *argv, VALUE self)
{
	VALUE point, block;
	rb_scan_args(argc, argv, "10&", &point, &block);
	
	cpSpaceShapePointQuery(SPACE(self), *VGET(point), pointQueryHelper, (void *)block);
	
	return Qnil;
}

static VALUE
rb_cpSpaceStaticShapePointQuery(int argc, VALUE *argv, VALUE self)
{
	VALUE point, block;
	rb_scan_args(argc, argv, "10&", &point, &block);
	
	cpSpaceStaticShapePointQuery(SPACE(self), *VGET(point), pointQueryHelper, (void *)block);
	
	return Qnil;
}

static VALUE
rb_cpSpaceStep(VALUE self, VALUE dt)
{
	cpSpaceStep(SPACE(self), NUM2DBL(dt));
	return Qnil;
}



void
Init_cpSpace(void)
{
	id_call = rb_intern("call");
	
	c_cpSpace = rb_define_class_under(m_Chipmunk, "Space", rb_cObject);
	rb_define_alloc_func(c_cpSpace, rb_cpSpaceAlloc);
	rb_define_method(c_cpSpace, "initialize", rb_cpSpaceInitialize, 0);
	
	rb_define_method(c_cpSpace, "iterations", rb_cpSpaceGetIterations, 0);
	rb_define_method(c_cpSpace, "iterations=", rb_cpSpaceSetIterations, 1);
	
	rb_define_method(c_cpSpace, "elastic_iterations", rb_cpSpaceGetElasticIterations, 0);
	rb_define_method(c_cpSpace, "elastic_iterations=", rb_cpSpaceSetElasticIterations, 1);
	
	rb_define_method(c_cpSpace, "damping", rb_cpSpaceGetDamping, 0);
	rb_define_method(c_cpSpace, "damping=", rb_cpSpaceSetDamping, 1);
	
	rb_define_method(c_cpSpace, "gravity", rb_cpSpaceGetGravity, 0);
	rb_define_method(c_cpSpace, "gravity=", rb_cpSpaceSetGravity, 1);

	rb_define_method(c_cpSpace, "add_collision_func", rb_cpSpaceAddCollisionFunc, -1);
	rb_define_method(c_cpSpace, "remove_collision_func", rb_cpSpaceRemoveCollisionFunc, 2);
	rb_define_method(c_cpSpace, "set_default_collision_func", rb_cpSpaceSetDefaultCollisionFunc, -1);
	
	rb_define_method(c_cpSpace, "add_shape", rb_cpSpaceAddShape, 1);
	rb_define_method(c_cpSpace, "add_static_shape", rb_cpSpaceAddStaticShape, 1);
	rb_define_method(c_cpSpace, "add_body", rb_cpSpaceAddBody, 1);
	rb_define_method(c_cpSpace, "add_joint", rb_cpSpaceAddJoint, 1);
	
	rb_define_method(c_cpSpace, "remove_shape", rb_cpSpaceRemoveShape, 1);
	rb_define_method(c_cpSpace, "remove_static_shape", rb_cpSpaceRemoveStaticShape, 1);
	rb_define_method(c_cpSpace, "remove_body", rb_cpSpaceRemoveBody, 1);
	rb_define_method(c_cpSpace, "remove_joint", rb_cpSpaceRemoveJoint, 1);
	
	rb_define_method(c_cpSpace, "resize_static_hash", rb_cpSpaceResizeStaticHash, 2);
	rb_define_method(c_cpSpace, "resize_active_hash", rb_cpSpaceResizeActiveHash, 2);
	rb_define_method(c_cpSpace, "rehash_static", rb_cpSpaceRehashStatic, 0);
	
	rb_define_method(c_cpSpace, "shape_point_query", rb_cpSpaceShapePointQuery, -1);
	rb_define_method(c_cpSpace, "static_shape_point_query", rb_cpSpaceStaticShapePointQuery, -1);
	
	rb_define_method(c_cpSpace, "step", rb_cpSpaceStep, 1);
}
