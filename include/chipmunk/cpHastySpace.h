/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
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

// Copyright 2013 Howling Moon Software. All rights reserved.
// See http://chipmunk2d.net/legal.php for more information.

#pragma once

/// cpHastySpace is exclusive to Chipmunk Pro
/// Currently it enables ARM NEON optimizations in the solver, but in the future will include other optimizations such as
/// a multi-threaded solver and multi-threaded collision broadphases.

struct cpHastySpace;
typedef struct cpHastySpace cpHastySpace;

/// Create a new hasty space.
/// On ARM platforms that support NEON, this will enable the vectorized solver.
/// cpHastySpace also supports multiple threads, but runs single threaded by default for determinism.
CP_EXPORT cpSpace *cpHastySpaceNew(void);
CP_EXPORT void cpHastySpaceFree(cpSpace *space);

/// Set the number of threads to use for the solver.
/// Currently Chipmunk is limited to 2 threads as using more generally provides very minimal performance gains.
/// Passing 0 as the thread count on iOS or OS X will cause Chipmunk to automatically detect the number of threads it should use.
/// On other platforms passing 0 for the thread count will set 1 thread.
CP_EXPORT void cpHastySpaceSetThreads(cpSpace *space, unsigned long threads);

/// Returns the number of threads the solver is using to run.
CP_EXPORT unsigned long cpHastySpaceGetThreads(cpSpace *space);

/// When stepping a hasty space, you must use this function.
CP_EXPORT void cpHastySpaceStep(cpSpace *space, cpFloat dt);
