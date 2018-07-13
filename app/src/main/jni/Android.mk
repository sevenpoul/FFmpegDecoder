LOCAL_PATH := $(call my-dir)

#ffmpeg
include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := ../libs/$(TARGET_ARCH_ABI)/libffmpeg.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := decoding
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/*.cpp) log.h
#$(wildcard $(LOCAL_PATH)/libyuv/source/*.cc)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/ffmpeg/
LOCAL_SHARED_LIBRARIES += ffmpeg
LOCAL_LDLIBS +=  -lm -llog  -ljnigraphics -landroid
LOCAL_CPPFLAGS += -std=c++11 -Wall -Wsign-compare -fpermissive -D_PLATFORM_ANDROID -Ofast
include $(BUILD_SHARED_LIBRARY)