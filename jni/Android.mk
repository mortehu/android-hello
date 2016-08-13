LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := hello-world
LOCAL_CFLAGS    := -Wall
LOCAL_CXXFLAGS  := -Wall -Wno-format-security -std=c++11 -fexceptions
LOCAL_SRC_FILES := hello-world.cc
LOCAL_LDLIBS    := -llog -lGLESv2

include $(BUILD_SHARED_LIBRARY)
