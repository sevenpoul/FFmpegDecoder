//frameEncoder

#ifndef X264_EXAMPLE_DECODER_H
#define X264_EXAMPLE_DECODER_H

#include <inttypes.h>
#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>

//#include <opencv2/core/core.hpp>
extern "C"
{
//#ifdef PLATFORM_ANDROID
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libswscale/swscale.h"
#include "../log.h"
#include "android_native_window.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include "libyuv/libyuv.h"
};

#define INBUF_SIZE 4096
//#define INBUF_SIZE 8192

typedef struct _RenderParam{
    SwsContext *swsContext;
    AVCodecContext *avCodecContext;
}RenderParam;

typedef struct _NalInfo{
    uint8_t forbidden_zero_bit;
    uint8_t nal_ref_idc;
    uint8_t nal_unit_type;
} NalInfo;

typedef struct _EnvPackage {
    JNIEnv *env;
    jobject *obj;
    jobject *surface;
}EnvPackage;

class decoder {

private:

    int frame_count;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    AVPacket *avpkt;
    AVCodecParserContext *parser;
    AVCodec *codec;
    AVCodecContext *codecContext;
    SwsContext *img_convert_ctx;
    AVFormatContext *pFormatCtx;
    AVDictionary *avDictionary;

    enum AVPixelFormat pixelFormat;
    //rgb frame cache
    AVFrame	*pFrameRGB;
    //std::queue < cv::Mat > vecMat;
    int native_pix_format = PIXEL_FORMAT_RGB_565;
public:

    decoder();
    RenderParam *renderParam = NULL;

    void handle_data(AVFrame *pFrame, void *param, void *ctx);
    long initialize(enum AVPixelFormat format, int width, int height);
    void decodeFrame(jbyte *data, int length, void *ctx);
    void close();
    //bool getFrame(cv::Mat & curmat);

    void setFrameRGB(AVFrame *frame);
    int handleH264Header(uint8_t* ptr, NalInfo *nalInfo);
    AVFrame *getFrameRGB();

    void decodeToYuv(JNIEnv *env, jbyte *data, int lenght, jbyteArray dst);

    uint8_t *yuv420p_2_argb(AVFrame	*frame, SwsContext *swsContext, AVCodecContext *avCodecContext,
                               enum AVPixelFormat format);
};

#endif
