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

void ChipmunkDebugDrawInit(void);

extern float ChipmunkDebugDrawPointLineScale;
extern float ChipmunkDebugDrawOutlineWidth;

void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color);
void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawPolygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawBB(cpBB bb, cpSpaceDebugColor outlineColor);

// Call this at the end of the frame to draw the ChipmunkDebugDraw*() commands to the screen.
void ChipmunkDebugDrawFlushRenderer(void);
// Call this at the beginning of the frame to clear out any ChipmunkDebugDraw*() commands.
void ChipmunkDebugDrawClearRenderer(void);

// Save the current contents of the renderer.
void ChipmunkDebugDrawPushRenderer(void);
// Reset the renderer back to it's last pushed state.
void ChipmunkDebugDrawPopRenderer(void);
