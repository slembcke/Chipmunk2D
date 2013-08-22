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

typedef struct Color {
	float r, g, b, a;
} Color;

static inline Color RGBAColor(float r, float g, float b, float a){
	Color color = {r, g, b, a};
	return color;
}

static inline Color LAColor(float l, float a){
	Color color = {l, l, l, a};
	return color;
}

void ChipmunkDebugDrawInit(void);

extern float ChipmunkDebugDrawPointLineScale;
extern float ChipmunkDebugDrawOutlineWidth;

void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, Color outlineColor, Color fillColor);
void ChipmunkDebugDrawSegment(cpVect a, cpVect b, Color color);
void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, Color outlineColor, Color fillColor);
void ChipmunkDebugDrawPolygon(int count, cpVect *verts, cpFloat radius, Color outlineColor, Color fillColor);
void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, Color fillColor);
void ChipmunkDebugDrawBB(cpBB bb, Color outlineColor);

void ChipmunkDebugDrawConstraint(cpConstraint *constraint);
void ChipmunkDebugDrawShape(cpShape *shape, Color outlineColor, Color fillColor);

void ChipmunkDebugDrawShapes(cpSpace *space);
void ChipmunkDebugDrawConstraints(cpSpace *space);
void ChipmunkDebugDrawCollisionPoints(cpSpace *space);

// Call this at the end of the frame to draw the ChipmunkDebugDraw*() commands to the screen.
void ChipmunkDebugDrawFlushRenderer(void);
// Call this at the beginning of the frame to clear out any ChipmunkDebugDraw*() commands.
void ChipmunkDebugDrawClearRenderer(void);

// Save the current contents of the renderer.
void ChipmunkDebugDrawPushRenderer(void);
// Reset the renderer back to it's last pushed state.
void ChipmunkDebugDrawPopRenderer(void);
