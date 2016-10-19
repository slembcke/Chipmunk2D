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

static inline cpSpaceDebugColor RGBAColor(float r, float g, float b, float a){
	cpSpaceDebugColor color = {r, g, b, a};
	return color;
}

static inline cpSpaceDebugColor LAColor(float l, float a){
	cpSpaceDebugColor color = {l, l, l, a};
	return color;
}

static const cpSpaceDebugColor ChipmunkDebugDrawOutlineColor = {0xFBp-8, 0xFFp-8, 0xB9p-8, 1};  
static const cpSpaceDebugColor ChipmunkDebugDrawTextColor = {1, 1, 1, 1};  

void ChipmunkDebugDrawInit(void);

extern float ChipmunkDebugDrawScaleFactor;
extern cpTransform ChipmunkDebugDrawProjection;
extern cpTransform ChipmunkDebugDrawCamera;

extern cpVect ChipmunkDebugDrawLightPosition;
extern cpFloat ChipmunkDebugDrawLightRadius;

void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor fill);
void ChipmunkDebugDrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color);
void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor fill);
void ChipmunkDebugDrawPolygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor fill);
void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fill);
void ChipmunkDebugDrawBB(cpBB bb, cpSpaceDebugColor color);

void ChipmunkDebugDrawText(cpVect pos, char const *str, cpSpaceDebugColor color);

void ChipmunkDebugDrawShadow(cpTransform transform, int count, cpVect *verts);
void ChipmunkDebugDrawApplyShadows(void);

void ChipmunkDebugDrawBegin(int width, int height);
void ChipmunkDebugDrawFlush(void);
