# Builds Chipmunk as a static library
# This will have to do until I can figure out how to properly use the Ruby build system.

rm -f *.o
find ../src -name "*.c" -print0 | xargs -0 gcc -c -std=gnu99 -ffast-math -O3 -I../src -I../src/constraint -arch ppc -arch i386
ar rcs libchipmunk.a *.o
rm -f *.o
