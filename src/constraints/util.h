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

void cpConstraintInit(cpConstraint *constraint, const cpConstraintClass *klass, cpBody *a, cpBody *b);

static inline cpVect
relative_velocity(cpVect r1, cpVect v1, cpFloat w1, cpVect r2, cpVect v2, cpFloat w2){
	cpVect v1_sum = cpvadd(v1, cpvmult(cpvperp(r1), w1));
	cpVect v2_sum = cpvadd(v2, cpvmult(cpvperp(r2), w2));
	
	return cpvsub(v2_sum, v1_sum);
}

static inline cpFloat
k_scalar(cpBody *a, cpBody *b, cpVect r1, cpVect r2, cpVect n)
{
	cpFloat mass_sum = a->m_inv + b->m_inv;
	cpFloat r1cn = cpvcross(r1, n);
	cpFloat r2cn = cpvcross(r2, n);

	return mass_sum + a->i_inv*r1cn*r1cn + b->i_inv*r2cn*r2cn;
}

static inline void
apply_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j)
{
	cpBodyApplyImpulse(a, cpvneg(j), r1);
	cpBodyApplyImpulse(b, j, r2);
}

static inline void
apply_bias_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j)
{
	cpBodyApplyBiasImpulse(a, cpvneg(j), r1);
	cpBodyApplyBiasImpulse(b, j, r2);
}
