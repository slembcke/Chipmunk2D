This subdirectory contains project and solution files for building the Chipmunk
Physics library and its demo suite with Microsoft Visual C++ 2010.

The demo suite uses the external GLUT (OpenGL Utility Toolkit) library,
included in the glut subdirectory. Since it loads glut32.dll at startup and
Visual Studio defaults the demo's working directory to msvc/vc10/demo there is
a copy of the DLL there.
