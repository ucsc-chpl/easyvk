LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := my-app
LOCAL_C_INCLUDES := src/ 
LOCAL_SRC_FILES := src/easyvk.cpp
LOCAL_LDLIBS    += -lvulkan

#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)