	ABOUT:

Chipmunk is a simple, lightweight and fast 2D rigid body physics library written in C. It's licensed under the unrestrictive, OSI approved MIT license. My aim is to give 2D developers access the same quality of physics you find in newer 3D games. I hope you enjoy using Chipmunk, and please consider donating to help make it worth my time to continue to support Chipmunk.

	BUILDING:

OS X: There is an included XCode project file for building the static library and demo application. Alteratively you could use the CMake files.

UNIX: A forum user was kind enough to make a set of CMake files for Chipmunk. This will require you to have CMake installed. To build run 'cmake .' then 'make'. This should build a dynamic library, a static library, and the demo application.

Windows: There is an included MSVC project for building the library and demo application.

Ruby: I maintain a Ruby extension for Chipmunk. I've been toying around with using SCons for other projects, and at the moment this is the only good way to build the Ruby extension unless somebody wants to step up and help make a proper Ruby build script.

	GETTING STARTED:

First of all, you can find the C API documentation here: http://code.google.com/p/chipmunk-physics/wiki/Documentation

A good starting point is to take a look at the included Demo application. The demos all just set up a Chipmunk simulation space and the demo app draws the graphics directly out of that. This makes it easy to see how the Chipmunk API works without worrying about mixing graphics code in. This is also a good way to experiment with Chipmunk. The demo drawing code is not easily maintainable, extensible or scalable however.

	FORUM:

http://www.slembcke.net/forums

	CONTACT:

slembcke@gmail.com (also on Google Talk)

	CHANGES SINCE 4.1.x:

* New Joint/Constraint API. 