LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := easyvk
LOCAL_C_INCLUDES := src/ volk/
LOCAL_SRC_FILES := src/easyvk.cpp
LOCAL_LDLIBS    += -lvulkan -llog

#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
