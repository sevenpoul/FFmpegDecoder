
#include "hard.h"

hard::hard() :
        avCodec(NULL), avCodecContext(NULL), avFrame(NULL), pFrameRGBA(NULL), parser(NULL),
        aNativeWindow(NULL), swsContext(NULL){

}

void hard::initHard(JNIEnv *env, int width, int height, jobject surface) {
    avcodec_register_all();
    av_init_packet(&avPacket);

    memset(inbuffer , 0, INBUF_SIZE);
    memset(inbuffer + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);
    avCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
//    avCodec = avcodec_find_decoder_by_name("h264_mediacodec"); //硬解码264
//    avCodec = avcodec_find_decoder_by_name("hevc_mediacodec"); //硬解码265
    if(!avCodec) {
        LOGI("Couldn not find codec.\n");
        exit(1);
    }
    //打开这个编码器，pCodecCtx表示编码器上下文，里面有流数据的信息
    // Get a pointer to the codec context for the video stream
    avCodecContext = avcodec_alloc_context3(avCodec);
    if(!avCodecContext) {
        LOGI("Couldn not find codec context.\n");
        exit(1);
    }
    /* put sample parameters */
    avCodecContext->width = width;
    avCodecContext->height = height;
//    avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
//    avCodecContext->bit_rate = 500000;
//    avCodecContext->time_base = (AVRational ) { 1, 15 };
//    avCodecContext->framerate = (AVRational ) { 1, 15 };
//    avCodecContext->gop_size = 1;
//    avCodecContext->max_b_frames = 0;
//    avCodecContext->ticks_per_frame = 0;
//    avCodecContext->delay = 0;
//    avCodecContext->b_quant_offset = 0.0;
//    avCodecContext->refs = 0;
//    avCodecContext->slices = 1;
//    avCodecContext->has_b_frames = 0;
//    avCodecContext->thread_count = 1;
//    av_opt_set(avCodecContext->priv_data, "zerolatency", "ultrafast", 0);

    /* we do not send complete frames */
    if (avCodec->capabilities & CODEC_CAP_TRUNCATED)
        avCodecContext->flags |= CODEC_FLAG_TRUNCATED;

    int open = avcodec_open2(avCodecContext, avCodec, NULL);
    LOGI("open result %d", open);
    if(open < 0) {
        LOGI("Could not open codec.\n");
        exit(1);
    }
    aNativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(aNativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);
    avFrame = av_frame_alloc();
    pFrameRGBA = av_frame_alloc();
    if(!avFrame || !pFrameRGBA) {
        LOGI("Could not allocate video frame.\n");
        exit(1);
    }
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA, width, height, 1);

    swsContext = sws_getContext(width, height, avCodecContext->pix_fmt, width, height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
    if(!swsContext) {
        LOGI("Could not initialize swscontext.\n");
        exit(1);
    }
    parser = av_parser_init(AV_CODEC_ID_H264);
    if(!parser) {
        LOGI("Could not create parser.\n");
        exit(1);
    }
}

int hard::hardDecode(jbyte *data, int len) {
    int cur_size = len;
    int ret = 0;

    memcpy(inbuffer, data, len);
    uint8_t *cur_ptr = inbuffer;
    // Parse input stream to check if there is a valid frame.
    //std::cout << " in data  --  -- " << length<< std::endl;
    while(cur_size > 0) {
        int parsedLength = av_parser_parse2(parser, avCodecContext, &avPacket.data, &avPacket.size,
                                            (const uint8_t *) cur_ptr, cur_size, AV_NOPTS_VALUE,
                                            AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        cur_ptr += parsedLength;
        cur_size -= parsedLength;

        ret = avcodec_send_packet(avCodecContext, &avPacket);
        if(ret < 0) {
            break;
        }
        int recv = avcodec_receive_frame(avCodecContext, avFrame);
        LOGI("hard decode success %d", recv);
        if (recv == 0) {
            LOGI("start receive frame.\n");
            ANativeWindow_lock(aNativeWindow, &nativeWindowBuffer, 0);
            //转换
            sws_scale(swsContext, (const uint8_t *const *) avFrame->data, avFrame->linesize, 0,
                      avCodecContext->height, pFrameRGBA->data, pFrameRGBA->linesize);
            // 获取stride
            uint8_t *dst = (uint8_t *) nativeWindowBuffer.bits;
            int dstStride = nativeWindowBuffer.stride * 4;
            uint8_t *src = pFrameRGBA->data[0];
            int srcStride = pFrameRGBA->linesize[0];

            // 由于window的stride和帧的stride不同,因此需要逐行复制
            int h;
            for (h = 0; h < avCodecContext->height; h++) {
                memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
            }
            ANativeWindow_unlockAndPost(aNativeWindow);
        }
        av_packet_unref(&avPacket);
    }
    return 1;
}

void hard::closeHard() {
    if(pFrameRGBA){
        delete pFrameRGBA;
        pFrameRGBA = NULL;
    }
    av_frame_free(&avFrame);

    avcodec_close(avCodecContext);
    av_free(avCodecContext);
    av_free_packet(&avPacket);
    avPacket.data = NULL;
    avPacket.size = 0;
    if (parser) {
        av_parser_close(parser);
        parser = NULL;
    }
    if(swsContext != NULL) {
        sws_freeContext(swsContext);
        swsContext = NULL;
    }
}
