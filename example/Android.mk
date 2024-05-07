LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := vect-add
LOCAL_C_INCLUDES := ../src/ ../volk
LOCAL_SRC_FILES := ../src/easyvk.cpp vect-add.cpp
LOCAL_LDLIBS    += -lvulkan -llog

include $(BUILD_EXECUTABLE)
