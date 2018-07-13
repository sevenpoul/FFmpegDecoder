//
// Created by 李德琛 on 6/10/17.
//
#ifndef _ANDROID_NATIVE_WINDOW_
#define _ANDROID_NATIVE_WINDOW_

#include <android/native_window.h>
#include <android/native_window_jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../log.h"
#include "ffmpeg/libavutil/imgutils.h"

enum {
    PIXEL_FORMAT_RGBA_8888 = 1,
    PIXEL_FORMAT_RGBX_8888 = 2,
    PIXEL_FORMAT_RGB_565 = 3
};

typedef struct _VoutInfo{

    /**
    WINDOW_FORMAT_RGBA_8888          = 1,
    WINDOW_FORMAT_RGBX_8888          = 2,
    WINDOW_FORMAT_RGB_565            = 4,*/
    uint32_t pix_format;

    uint32_t buffer_width;
    uint32_t buffer_height;
    uint8_t *buffer;
} VoutInfo;

typedef struct _VoutRender{
    uint32_t pix_format;
    uint32_t window_format;
    void (*render)(ANativeWindow_Buffer *nwBuffer, VoutInfo *voutInfo);
}VoutRender;

void android_native_window_display(ANativeWindow *aNativeWindow, VoutInfo *voutInfo);

#ifdef __cplusplus
}
#endif
#endif