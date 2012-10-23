This subdirectory contains project and solution files for building the Chipmunk
Physics library and its demo suite with Microsoft Visual C++ 2010.

The demo suite uses the external GLUT (OpenGL Utility Toolkit) library,
included in the glut subdirectory.

Here's how the Chipmunk and C runtime libraries are linked for each of the
available configurations:

                   Chipmunk    C Runtime

    Debug          Static      Dynamic
    Debug DLL      Dynamic     Dynamic
    Debug SCRT     Static      Static
    Release        Static      Dynamic
    Release DLL    Dynamic     Dynamic
    Release SCRT   Static      Static
