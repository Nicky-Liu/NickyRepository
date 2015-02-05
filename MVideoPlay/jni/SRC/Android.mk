LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SDL_PATH := ../SDL

PATH_TO_FFMPEG_SOURCE:=$(LOCAL_PATH)/../ffmpeg
LOCAL_C_INCLUDES += $(PATH_TO_FFMPEG_SOURCE)
LOCAL_LDLIBS :=  -llog
#使ffmpeg在c++中能进行编译
LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS 
LOCAL_LDLIBS += libs/libffmpeg-neon.so
LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_MODULE    := ffmpeg_test
#LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c SDLTest.cpp
#LOCAL_SRC_FILES := ffmpegTest.cpp Source/Conversion.cpp
#LOCAL_SRC_FILES :=$(SDL_PATH)/src/main/android/SDL_android_main.c audio_and_Video.cpp
LOCAL_SRC_FILES :=$(SDL_PATH)/src/main/android/SDL_android_main.c \
AVTest.cpp \
Source/DecodeInfo.cpp \
Source/PktQueue.cpp \
Source/SDLInfo.cpp \
Source/AudioHandle.cpp \
Source/VideoHandle.cpp

include $(BUILD_SHARED_LIBRARY)

#加载ffmpeg库
include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_so
LOCAL_SRC_FILES := ../libs/libffmpeg-neon.so
include $(PREBUILT_SHARED_LIBRARY)    