/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
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

#ifndef CHIPMUNK_VECT_H
#define CHIPMUNK_VECT_H

#include "chipmunk_types.h"

/// @defgroup cpVect cpVect
/// Chipmunk's 2D vector type along with a handy 2D vector math lib.
/// @{

/// Constant for the zero vector.
#ifdef CP_USE_CGTYPES
__attribute__((swift_private))
#else
__attribute__((swift_name("Vector.zero")))
#endif
static const cpVect cpvzero = {0.0f,0.0f};

/// Convenience constructor for cpVect structs.
__attribute__((swift_private))
static inline cpVect cpv(const cpFloat x, const cpFloat y)
{
	cpVect v = {x, y};
	return v;
}

/// Check if two vectors are equal. (Be careful when comparing floating point numbers!)
__attribute__((swift_name("Vector.isEqual(self:_:)")))
static inline cpBool cpveql(const cpVect v1, const cpVect v2)
{
	return (v1.x == v2.x && v1.y == v2.y);
}

/// Add two vectors
__attribute__((swift_name("Vector.add(self:_:)")))
static inline cpVect cpvadd(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x + v2.x, v1.y + v2.y);
}

/// Subtract two vectors.
__attribute__((swift_name("Vector.subtract(self:_:)")))
static inline cpVect cpvsub(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x - v2.x, v1.y - v2.y);
}

/// Negate a vector.
__attribute__((swift_name("getter:Vector.negate(self:)")))
static inline cpVect cpvneg(const cpVect v)
{
	return cpv(-v.x, -v.y);
}

/// Scalar multiplication.
__attribute__((swift_name("Vector.multiply(self:_:)")))
static inline cpVect cpvmult(const cpVect v, const cpFloat s)
{
	return cpv(v.x*s, v.y*s);
}

/// Vector dot product.
__attribute__((swift_name("Vector.dotProduct(self:_:)")))
static inline cpFloat cpvdot(const cpVect v1, const cpVect v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

/// 2D vector cross product analog.
/// The cross product of 2D vectors results in a 3D vector with only a z component.
/// This function returns the magnitude of the z value.
__attribute__((swift_name("Vector.crossProduct(self:_:)")))
static inline cpFloat cpvcross(const cpVect v1, const cpVect v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

/// Returns a perpendicular vector. (90 degree rotation)
__attribute__((swift_name("getter:Vector.perperdicular(self:)")))
static inline cpVect cpvperp(const cpVect v)
{
	return cpv(-v.y, v.x);
}

/// Returns a perpendicular vector. (-90 degree rotation)
__attribute__((swift_name("getter:Vector.negativePerperdicular(self:)")))
static inline cpVect cpvrperp(const cpVect v)
{
	return cpv(v.y, -v.x);
}

/// Returns the vector projection of v1 onto v2.
__attribute__((swift_name("Vector.project(self:_:)")))
static inline cpVect cpvproject(const cpVect v1, const cpVect v2)
{
	return cpvmult(v2, cpvdot(v1, v2)/cpvdot(v2, v2));
}

/// Returns the unit length vector for the given angle (in radians).
__attribute__((swift_name("Vector.for(angle:)")))
static inline cpVect cpvforangle(const cpFloat a)
{
	return cpv(cpfcos(a), cpfsin(a));
}

/// Returns the angular direction v is pointing in (in radians).
__attribute__((swift_name("Vector.to(angle:)")))
static inline cpFloat cpvtoangle(const cpVect v)
{
	return cpfatan2(v.y, v.x);
}

/// Uses complex number multiplication to rotate v1 by v2. Scaling will occur if v1 is not a unit vector.
__attribute__((swift_name("Vector.rotate(self:_:)")))
static inline cpVect cpvrotate(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x);
}

/// Inverse of cpvrotate().
__attribute__((swift_name("Vector.unrotate(self:_:)")))
static inline cpVect cpvunrotate(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x*v2.x + v1.y*v2.y, v1.y*v2.x - v1.x*v2.y);
}

/// Returns the squared length of `v`. Faster than `cpvlength()` when you only need to compare lengths.
__attribute__((swift_name("getter:Vector.squaredLength(self:)")))
static inline cpFloat cpvlengthsq(const cpVect v)
{
	return cpvdot(v, v);
}

/// Returns the length of v.
__attribute__((swift_name("getter:Vector.length(self:)")))
static inline cpFloat cpvlength(const cpVect v)
{
	return cpfsqrt(cpvdot(v, v));
}

/// Linearly interpolate between v1 and v2.
__attribute__((swift_name("Vector.linearlyInterpolate(self:_:_:)")))
static inline cpVect cpvlerp(const cpVect v1, const cpVect v2, const cpFloat t)
{
	return cpvadd(cpvmult(v1, 1.0f - t), cpvmult(v2, t));
}

/// Returns a normalized copy of v.
__attribute__((swift_name("getter:Vector.normalize(self:)")))
static inline cpVect cpvnormalize(const cpVect v)
{
	// Neat trick I saw somewhere to avoid div/0.
	return cpvmult(v, 1.0f/(cpvlength(v) + CPFLOAT_MIN));
}

/// Spherical linearly interpolate between v1 and v2.
static inline cpVect
__attribute__((swift_name("Vector.sphericalLinearlyInterpolate(self:_:_:)")))
cpvslerp(const cpVect v1, const cpVect v2, const cpFloat t)
{
	cpFloat dot = cpvdot(cpvnormalize(v1), cpvnormalize(v2));
	cpFloat omega = cpfacos(cpfclamp(dot, -1.0f, 1.0f));
	
	if(omega < 1e-3){
		// If the angle between two vectors is very small, lerp instead to avoid precision issues.
		return cpvlerp(v1, v2, t);
	} else {
		cpFloat denom = 1.0f/cpfsin(omega);
		return cpvadd(cpvmult(v1, cpfsin((1.0f - t)*omega)*denom), cpvmult(v2, cpfsin(t*omega)*denom));
	}
}

/// Spherical linearly interpolate between v1 towards v2 by no more than angle a radians
__attribute__((swift_name("Vector.sphericalLinearlyInterpolate(self:_:angle:)")))
static inline cpVect
cpvslerpconst(const cpVect v1, const cpVect v2, const cpFloat a)
{
	cpFloat dot = cpvdot(cpvnormalize(v1), cpvnormalize(v2));
	cpFloat omega = cpfacos(cpfclamp(dot, -1.0f, 1.0f));
	
	return cpvslerp(v1, v2, cpfmin(a, omega)/omega);
}

/// Clamp v to length len.
__attribute__((swift_name("Vector.clamp(self:_:)")))
static inline cpVect cpvclamp(const cpVect v, const cpFloat len)
{
	return (cpvdot(v,v) > len*len) ? cpvmult(cpvnormalize(v), len) : v;
}

/// Linearly interpolate between v1 towards v2 by distance d.
__attribute__((swift_name("Vector.linearlyInterpolate(self:_:distance:)")))
static inline cpVect cpvlerpconst(cpVect v1, cpVect v2, cpFloat d)
{
	return cpvadd(v1, cpvclamp(cpvsub(v2, v1), d));
}

/// Returns the distance between v1 and v2.
__attribute__((swift_name("Vector.distance(self:_:)")))
static inline cpFloat cpvdist(const cpVect v1, const cpVect v2)
{
	return cpvlength(cpvsub(v1, v2));
}

/// Returns the squared distance between v1 and v2. Faster than cpvdist() when you only need to compare distances.
__attribute__((swift_name("Vector.squaredDistance(self:_:)")))
static inline cpFloat cpvdistsq(const cpVect v1, const cpVect v2)
{
	return cpvlengthsq(cpvsub(v1, v2));
}

/// Returns true if the distance between v1 and v2 is less than dist.
__attribute__((swift_name("Vector.near(self:_:distance:)")))
static inline cpBool cpvnear(const cpVect v1, const cpVect v2, const cpFloat dist)
{
	return cpvdistsq(v1, v2) < dist*dist;
}

/// @}

/// @defgroup cpMat2x2 cpMat2x2
/// 2x2 matrix type used for tensors and such.
/// @{

// NUKE
__attribute__((swift_name("Matrix2.init(a:b:c:d:)")))
static inline cpMat2x2
cpMat2x2New(cpFloat a, cpFloat b, cpFloat c, cpFloat d)
{
	cpMat2x2 m = {a, b, c, d};
	return m;
}

__attribute__((swift_name("Matrix2.transform(self:vector:)")))
static inline cpVect
cpMat2x2Transform(cpMat2x2 m, cpVect v)
{
	return cpv(v.x*m.a + v.y*m.b, v.x*m.c + v.y*m.d);
}

///@}

#endif
