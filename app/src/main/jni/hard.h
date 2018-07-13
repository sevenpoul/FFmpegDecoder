//
// Created by lishanshan on 2018/6/21.
//

#ifndef ANDROID_FFMPGE_MUTI_DECODE_HARD_H
#define ANDROID_FFMPGE_MUTI_DECODE_HARD_H
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libavcodec/jni.h"
#include "libavutil/opt.h"
#include "log.h"
#include "android/native_window.h"
#include "android/native_window_jni.h"
};

#define INBUF_SIZE 4096

class hard {

private:
    AVCodec *avCodec;
    AVCodecContext *avCodecContext;
    ANativeWindow *aNativeWindow;
    ANativeWindow_Buffer nativeWindowBuffer;
    AVCodecParserContext *parser;
    AVFrame *avFrame;
    AVFrame *pFrameRGBA;
    SwsContext *swsContext;
    AVPacket avPacket;
    uint8_t inbuffer[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];

public:
    hard();
    void initHard(JNIEnv *env, int width, int height, jobject surface);
    int hardDecode(jbyte *string, int len);
    void closeHard();
};


#endif //ANDROID_FFMPGE_MUTI_DECODE_HARD_H
