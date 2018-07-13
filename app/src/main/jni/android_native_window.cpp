//
// Created by 李德琛 on 6/10/17.
//
#include "android_native_window.h"

void render_on_rgb(ANativeWindow_Buffer *nwBuffer, VoutInfo *voutInfo, int bpp){

    int stride = nwBuffer->stride;
    int dst_width = nwBuffer->width;
    int dst_height = nwBuffer->height;
    LOGE("ANativeWindow stride %d width %d height %d", stride, dst_width, dst_height);
    int line = 0;

    int src_line_size = voutInfo->buffer_width * bpp / 8;
    int dst_line_size = stride * bpp / 8;

    int min_height = dst_height < voutInfo->buffer_height ? dst_height : voutInfo->buffer_height;

    if(src_line_size == dst_line_size) {
        memcpy((__uint8_t *) nwBuffer->bits, (__uint8_t *) voutInfo->buffer, src_line_size * min_height);
    }else{
        //直接copy
        /*for(int i=0; i<height; i++){
            memcpy((__uint8_t *) (nwBuffer.bits + line), (__uint8_t *)(rgbFrame->data[0]+ width*i * 2), width * 2);
            line += stride * 2;
        }*/

        //使用ffmpeg的函数 实现相同功能
        av_image_copy_plane((uint8_t *)nwBuffer->bits, dst_line_size, voutInfo->buffer, src_line_size, src_line_size, min_height);
    }
}

void render_on_rgb8888(ANativeWindow_Buffer *nwBuffer, VoutInfo *voutInfo){
    render_on_rgb(nwBuffer, voutInfo, 32);
}

void render_on_rgb565(ANativeWindow_Buffer *nwBuffer, VoutInfo *voutInfo){
    render_on_rgb(nwBuffer, voutInfo, 16);
}

static VoutRender g_pixformat_map[] = {
        {PIXEL_FORMAT_RGBA_8888, WINDOW_FORMAT_RGBA_8888,render_on_rgb8888},
        {PIXEL_FORMAT_RGBX_8888, WINDOW_FORMAT_RGBX_8888, render_on_rgb8888},
        {PIXEL_FORMAT_RGB_565, WINDOW_FORMAT_RGB_565, render_on_rgb565}
};

VoutRender *get_render_by_window_format(int window_format){
    int len = sizeof(g_pixformat_map);
    for(int i=0; i<len; i++){
        if(g_pixformat_map[i].window_format == window_format){
            return &g_pixformat_map[i];
        }
    }
}

void android_native_window_display(ANativeWindow *aNativeWindow, VoutInfo *voutInfo){

    if (aNativeWindow == NULL) {
        LOGE("ANativeWindow_fromSurface error");
        return;
    }
    int curr_format = ANativeWindow_getFormat(aNativeWindow);
    VoutRender *render = get_render_by_window_format(curr_format);

    ANativeWindow_Buffer nwBuffer;
    //ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(envPackage->env, *(envPackage->surface));

    //scaled buffer to fit window
    int retval = ANativeWindow_setBuffersGeometry(aNativeWindow, voutInfo->buffer_width, voutInfo->buffer_height,  render->window_format);
    if (retval < 0) {
        LOGE("ANativeWindow_setBuffersGeometry: error %d", retval);
        return;
    }

    if (0 != ANativeWindow_lock(aNativeWindow, &nwBuffer, 0)) {
        LOGE("ANativeWindow_lock error");
        return;
    }

    render->render(&nwBuffer, voutInfo);

    if(0 !=ANativeWindow_unlockAndPost(aNativeWindow)){
        LOGE("ANativeWindow_unlockAndPost error");
        return;
    }
    //ANativeWindow_release(aNativeWindow);
}