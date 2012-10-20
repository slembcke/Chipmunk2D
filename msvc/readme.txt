This subdirectory contains project and solution files for building the Chipmunk
Physics library and its demo suite with Microsoft Visual C++ 2010.

The demo suite uses the external GLUT (OpenGL Utility Toolkit) library for
rendering, a copy of which is included in the glut subdirectory. The demo
program will look for glut.dll at load time, so you'll need to ensure it is
findable. One way to do this is to set the working directory (found under
Debugging configuration properties in Developer Studio) to
"../../glut/runtime". This will launch the demo in that directory, enabling
Windows to find the required DLL.
