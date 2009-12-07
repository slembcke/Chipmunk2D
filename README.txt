ABOUT:
Chipmunk is a simple, lightweight and fast 2D rigid body physics library written in C. It's licensed under the unrestrictive, OSI approved MIT license. My aim is to give 2D developers access the same quality of physics you find in newer 3D games. I hope you enjoy using Chipmunk, and please consider donating to help make it worth our time to continue to support Chipmunk with great new features.

CONTRACTING:
Howling Moon Software (my company) is available for contracting if you want to make the physics in your game really stand out. Feel free to contact us through our webpage: http://howlingmoonsoftware.com/contracting.php

BUILDING:
OS X: There is an included XCode project file for building the static library and demo application. Alteratively you could use the CMake files.

iPhone: I'll be releasing an iPhone example project shortly that builds Chipmunk as a static library. For now, you can just copy the src/ directory into your project and compile it as a static library. Make sure to disable thumb compilation, and use the -03 and -ffast-math flags to get the best performance.

UNIX: A forum user was kind enough to make a set of CMake files for Chipmunk. This will require you to have CMake installed. To build run 'cmake .' then 'make'. This should build a dynamic library, a static library, and the demo application.

Windows: There is an included MSVC project for building the library and demo application. I do not personally maintain the MSVC project. If there are issues with it, please make a post on the forums.

Ruby: I've been using maintaining a Ruby extension for Chipmunk, but at this time is not up to date with all the latest changes. A forum member has been working on an FFI based extention, and that may be a better way to take advantage of Chipmunk from Ruby. Stay tuned.

GETTING STARTED:
First of all, you can find the C API documentation here: http://code.google.com/p/chipmunk-physics/wiki/Documentation

A good starting point is to take a look at the included Demo application. The demos all just set up a Chipmunk simulation space and the demo app draws the graphics directly out of that. This makes it easy to see how the Chipmunk API works without worrying about the graphics code. You are free to use the demo drawing routines in your own projects, though it is certainly not the recommended way of drawing Chipmunk objects as it pokes around at the undocumented parts of Chipmunk.

FORUM:
http://www.slembcke.net/forums

CONTACT:
slembcke@gmail.com (also on Google Talk)

CHANGES SINCE 4.x:
* Brand new Joint/Constraint API: New constraints can be added easily and are much more flexible than the old joint system
* Efficient Segment Queries - Like raycasting, but with line segments.
* Brand new collision callback API: Collision begin/separate events, API for removal of objects within callbacks, more programable control over collision handling.