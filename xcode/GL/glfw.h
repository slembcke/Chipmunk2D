#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>


int glfwInit(void);
void glfwTerminate(void);

int glfwOpenWindow(int width, int height, int redbits, int greenbits, int bluebits, int alphabits, int depthbits, int stencilbits, int mode);
void glfwSetWindowTitle(const char *title);
void glfwGetWindowSize(int *w, int *h);


typedef void (*GLFWwindowsizefun)(int width, int height);
void glfwSetWindowSizeCallback(GLFWwindowsizefun func);

typedef int  (*GLFWwindowclosefun)(void);
void glfwSetWindowCloseCallback(GLFWwindowclosefun func);

typedef void (*GLFWcharfun)(int chr, int state);
void glfwSetCharCallback(GLFWcharfun func);

typedef void (*GLFWkeyfun)(int key, int state);
void glfwSetKeyCallback(GLFWkeyfun func);

typedef void (*GLFWmouseposfun)(int x, int y);
void glfwSetMousePosCallback(GLFWmouseposfun func);

typedef void (*GLFWmousebuttonfun)(int button, int state);
void glfwSetMouseButtonCallback(GLFWmousebuttonfun func);


double glfwGetTime(void);

void glfwSwapInterval(int interval);
void glfwSwapBuffers(void);


enum {
	GLFW_WINDOW = 0,
	
	GLFW_PRESS = 1,
	GLFW_RELEASE = 0,

	GLFW_MOUSE_BUTTON_1 = 0,
	GLFW_MOUSE_BUTTON_2 = 1,

	GLFW_KEY_UP = 0,
	GLFW_KEY_DOWN = 1,
	GLFW_KEY_LEFT = 2,
	GLFW_KEY_RIGHT = 3,
};
