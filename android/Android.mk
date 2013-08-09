# Copyright (C) PlayControl Software, LLC. 
# Eric Wing <ewing . public @ playcontrol.net>
#
# This is a "Prebuilt" Android Makefile provided as an example/template (or direct use if no tweaking is required).
# The idea is that you have already built the chipmunk .so and .a libraries using CMake.
# Now you want to use those prebuilt libraries in your own project.
# Android support prebuilt exteneral modules through its ndk-build system, but you need to have all the pieces setup and in the right place. This is one of those pieces.
# 
# This file assumes you built all your chipmunk libs and put things into a directory structure like so:
# 
# Android.mk (this file)
# 	libs/armeabi/libchipmunk.a
# 	libs/armeabi/libchipmunk.a
# 	libs/armeabi-v7a/libchipmunk.a
# 	libs/armeabi-v7a/libchipmunk.a
# 	libs/x86/libchipmunk.a
# 	libs/x86/libchipmunk.a
# 
# 	include/chipmunk/chipmunk.h
# 	... (the other header files here)
#
# Note that this file is copied into the directory above libs and include.
# Below is the code you need to make this Makefile export the correct headers, libraries, and flags for both dynamic and static versions.

# LOCAL_PATH needs to be before include
LOCAL_PATH := $(call my-dir)

# For the dynamic library
include $(CLEAR_VARS)
# This is the name of module the caller will use in LOCAL_SHARED_LIBRARIES
LOCAL_MODULE := chipmunk_shared
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libchipmunk.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include/chipmunk
# Use LOCAL_EXPORT_CFLAGS to automatically export the correct flags (as necessary) to the calling module so the caller doesn't need to know the details.
#LOCAL_EXPORT_CFLAGS := -DFOO=1 -DCP_USE_DOUBLES=1 -DCP_USE_CGPOINTS=0
# The .so is already linked so we don't really need to export this.
#LOCAL_EXPORT_LDLIBS := -lm
include $(PREBUILT_SHARED_LIBRARY)

# For the static library
include $(CLEAR_VARS)
# This is the name of module the caller will use in LOCAL_STATIC_LIBRARIES
LOCAL_MODULE := chipmunk_static
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libchipmunk.a
# Use LOCAL_EXPORT_CFLAGS to automatically export the correct flags (as necessary) to the calling module so the caller doesn't need to know the details.
#LOCAL_EXPORT_CFLAGS := -DFOO=1 -DCP_USE_DOUBLES=1 -DCP_USE_CGPOINTS=0
# Since the .a isn't linked, it's link dependencies must be passed on to the calling project.
LOCAL_EXPORT_LDLIBS := -lm
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include/chipmunk
include $(PREBUILT_STATIC_LIBRARY)



# Two other pieces are needed to make this work which fall outside the scope of this file.
# First, you must have a directory convention for the calling makefile.
# So let's say we put all the above in a directory called Chipmunk2D. The layout looks like this:
# Chipmunk2D/
# 	Android.mk (this file)
# 		libs/armeabi/libchipmunk.a
# 		libs/armeabi/libchipmunk.a
# 		libs/armeabi-v7a/libchipmunk.a
# 		libs/armeabi-v7a/libchipmunk.a
# 		libs/x86/libchipmunk.a
# 		libs/x86/libchipmunk.a
# 
# 		include/chipmunk/chipmunk.h
# 		... (the other header files here)

# So the calling makefile looks something like:
# LOCAL_PATH := $(call my-dir)
# include $(CLEAR_VARS)
# LOCAL_MODULE    := hello-jni
# LOCAL_SRC_FILES := hello-jni.c
# These are the LOCAL_MODULE names as defined in the prebuilt module's Android.mk. Define either shared or static, but not both. If you use dynamic, don't forget you need to do a System.loadLibrary("chipmunk") in your Java code.
# #LOCAL_SHARED_LIBRARIES := chipmunk_shared
# LOCAL_STATIC_LIBRARIES := chipmunk_static
# include $(BUILD_SHARED_LIBRARY)
# Android build system will look for folder `Chipmunk2D` in all import paths:
# $(call import-module,Chipmunk2D) 
# ------     end      -----

# Second, you need to set the environmental variable NDK_MODULE_PATH to list the directory containing Chipmunk2D.
# So if Chipmunk2D is in /Library/Frameworks/Android/PrebuiltModules
# export NDK_MODULE_PATH=/Library/Frameworks/Android/PrebuiltModules
# Note that NDK_MODULE_PATH may contain multiple directories like the PATH environmental variable.

