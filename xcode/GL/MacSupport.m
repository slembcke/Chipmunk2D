/* Copyright (c) 2016 Scott Lembcke
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

// Basic Mac re-implementation of GLFW2.
// Was easier than rewritting to use GLFW3 on all platforms.


#import <AppKit/AppKit.h>
#import "glfw.h"
#import "glew.h"


static GLFWwindowsizefun WindowSizeFunc = NULL;
static GLFWwindowclosefun WindowCloseFunc = NULL;
static GLFWcharfun CharFunc = NULL;
static GLFWkeyfun KeyFunc = NULL;
static GLFWmouseposfun MousePosFunc = NULL;
static GLFWmousebuttonfun MouseButtonFunc = NULL;


@interface AppDelegate : NSObject<NSApplicationDelegate> @end
@implementation AppDelegate {
	@public
	IBOutlet NSWindow *_window;
	IBOutlet NSView *_view;
	
}

+(NSWindow *)window {
	AppDelegate *delegate = [NSApplication sharedApplication].delegate;
	return delegate->_view.window;
}

-(void)applicationDidFinishLaunching:(NSNotification *)notification {
	[_window makeFirstResponder:_view];
	_window.acceptsMouseMovedEvents = YES;
}

-(void)applicationWillTerminate:(NSNotification *)notification {}

@end


@interface GLView : NSOpenGLView @end
@implementation GLView {
}

extern GLuint CompileShader(const char *vsource, const char *fsource);

-(void)awakeFromNib {
	self.openGLContext = [NSOpenGLContext currentContext];
}

-(NSPoint)convertCoords:(NSEvent *)event
{
	NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
	CGSize size = self.bounds.size;
	CGFloat scale = self.window.backingScaleFactor;
	
	return NSMakePoint(scale*point.x, scale*(size.height - point.y - 1));
}

-(void)mouseMoved:(NSEvent *)event {
	CGPoint p = [self convertCoords:event];
	MousePosFunc(p.x, p.y);
}

-(void)buttonEvent:(NSEvent *)event state:(bool)state {
	MouseButtonFunc((int)event.buttonNumber, state);
}

-(void)mouseDown:(NSEvent *)event {[self buttonEvent:event state:true];}
-(void)mouseUp:(NSEvent *)event {[self buttonEvent:event state:false];}
-(void)rightMouseDown:(NSEvent *)event {[self buttonEvent:event state:true];}
-(void)rightMouseUp:(NSEvent *)event {[self buttonEvent:event state:false];}
-(void)otherMouseDown:(NSEvent *)event {[self buttonEvent:event state:true];}
-(void)otherMouseUp:(NSEvent *)event {[self buttonEvent:event state:false];}

-(void)dragEvent:(NSEvent *)event {
	CGPoint p = [self convertCoords:event];
	MousePosFunc(p.x, p.y);
}

-(void)mouseDragged:(NSEvent *)event {[self dragEvent:event];}
-(void)rightMouseDragged:(NSEvent *)event {[self dragEvent:event];}
-(void)otherMouseDragged:(NSEvent *)event {[self dragEvent:event];}

-(void)keyEvent:(NSEvent *)event state:(bool)state {
	CharFunc([event.characters characterAtIndex:0], state);
	
	switch(event.keyCode){
		case 123: KeyFunc(GLFW_KEY_LEFT, state); break;
		case 124: KeyFunc(GLFW_KEY_RIGHT, state); break;
		case 125: KeyFunc(GLFW_KEY_DOWN, state); break;
		case 126: KeyFunc(GLFW_KEY_UP, state); break;
	}
}

-(void)keyDown:(NSEvent *)event {[self keyEvent:event state:true];}
-(void)keyUp:(NSEvent *)event {[self keyEvent:event state:false];}

-(void)reshape {
	NSSize size = [self convertSizeToBacking:self.bounds.size];
	WindowSizeFunc(size.width, size.height);
}

@end


NSAutoreleasePool *AUTORELEASE_POOL = nil;

// MARK: GLFW funcs.
int glfwInit(void){
	AUTORELEASE_POOL = [[NSAutoreleasePool alloc] init];
	
	[NSApplication sharedApplication];
	[NSApp finishLaunching];
	return true;
}

void glfwTerminate(void){}

int glfwOpenWindow(int width, int height, int redbits, int greenbits, int bluebits, int alphabits, int depthbits, int stencilbits, int mode){
	// Create and bind the OpenGL context early and permanently.
	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:(NSOpenGLPixelFormatAttribute[]){
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
		0
	}];
	
	NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	[context makeCurrentContext];
	
	[NSBundle loadNibNamed:@"MainMenu" owner:NSApp];
	
	return true;
}

void glfwSetWindowTitle(const char *title){
	[AppDelegate window].title = [NSString stringWithUTF8String:title];
}

void glfwGetWindowSize(int *w, int *h){
	NSWindow *window = [AppDelegate window];
	CGSize size = window.contentView.frame.size;
	CGFloat scale = window.backingScaleFactor;
	
	(*w) = scale*size.width;
	(*h) = scale*size.height;
}

void glfwSetWindowSizeCallback(GLFWwindowsizefun func){
	WindowSizeFunc = func;
}

void glfwSetWindowCloseCallback(GLFWwindowclosefun func){
	WindowCloseFunc = func;
}

void glfwSetCharCallback(GLFWcharfun func){
	CharFunc = func;
}

void glfwSetKeyCallback(GLFWkeyfun func){
	KeyFunc = func;
}

void glfwSetMousePosCallback(GLFWmouseposfun func){
	MousePosFunc = func;
}

void glfwSetMouseButtonCallback(GLFWmousebuttonfun func){
	MouseButtonFunc = func;
}


double glfwGetTime(void){
	return CACurrentMediaTime();
}

void glfwSwapInterval(int interval){
	CGLContextObj context = CGLGetCurrentContext();
	CGLSetParameter(context, kCGLCPSwapInterval, (GLint[]){interval});
}

void glfwSwapBuffers(void){
	[[NSOpenGLContext currentContext] flushBuffer];
	
	while(true){
		NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:true];
		if(event){
			[NSApp sendEvent:event];
		} else {
			break;
		}
	}
	
	[AUTORELEASE_POOL drain];
	AUTORELEASE_POOL = [[NSAutoreleasePool alloc] init];
}

int glewInit(void){
	return GLEW_NO_ERROR;
}
