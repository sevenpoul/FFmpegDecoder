#include "decoder.h"

decoder::decoder() :
        codec(NULL), codecContext(NULL), frame_count(0), frame(
        NULL), parser(NULL), pFrameRGB(NULL), pFormatCtx(NULL), avDictionary(NULL) {
}

long decoder::initialize(enum AVPixelFormat format, int width, int height) {

    /* register all the codecs */
//    avcodec_register_all();
    av_register_all();
    avpkt = av_packet_alloc();
    av_init_packet(avpkt);
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    memset(inbuf, 0, INBUF_SIZE);
    memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);

    char path[] = "android.resource://com.ffmpeg/2131099650";
//    av_dict_set(&avDictionary, "rtsp_transport", "tcp", 0); //tcp连接
//    av_dict_set(&avDictionary, "protocol_whitelist", "file,udp,rtp", 0);
//    av_dict_set(&avDictionary, "max_delay", "5000000", 0);

    int rtsp = avformat_open_input(&pFormatCtx, path, NULL, NULL);
    LOGI("avformat_open_input %d", rtsp);
    LOGI("avformat_open_input url %s", path);

    /* find the x264 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        LOGI("Codec not found \n");
        return -1;
    }

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        LOGI("Could not allocate video codec context\n");
        return -1;
    }

    /* put sample parameters */
    codecContext->width = width;
    codecContext->height = height;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    //codecContext->bit_rate = 500000;
    //codecContext->time_base = (AVRational ) { 1, 15 };
    //codecContext->framerate = (AVRational ) { 1, 15 };
    //codecContext->gop_size = 1;
    //codecContext->max_b_frames = 0;
    //codecContext->ticks_per_frame = 0;
    //codecContext->delay = 0;
    //codecContext->b_quant_offset = 0.0;
    //codecContext->refs = 0;
    //codecContext->slices = 1;
    //codecContext->has_b_frames = 0;
    //codecContext->thread_count = 2;
    //av_opt_set(codecContext->priv_data, "zerolatency", "ultrafast", 0);

    /* we do not send complete frames */
    if (codec->capabilities & CODEC_CAP_TRUNCATED)
        codecContext->flags |= CODEC_FLAG_TRUNCATED;

    /* open it */
    int open = avcodec_open2(codecContext, codec, NULL);
    LOGI("open result %d", open);
    if (open < 0) {
        LOGI("Could not open codec \n");
        return -1;
    }

    frame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    if (!frame || !pFrameRGB) {
        LOGI("Could not allocate video frame \n");
        return -1;
    }

    parser = av_parser_init(AV_CODEC_ID_H264);
    if (!parser) {
        LOGI("cannot create parser");
        return -1;
    }
//    parser->flags |= PARSER_FLAG_ONCE;
    LOGI(" decoder init ..........\n");
    frame_count = 0;
    img_convert_ctx = NULL;
    renderParam = NULL;

    //pixelFormat = AV_PIX_FMT_BGRA;
    //pixelFormat = AV_PIX_FMT_RGB565LE;
    //pixelFormat = AV_PIX_FMT_BGR24;
    pixelFormat = format;
    return 0;
}

void decoder::decodeFrame(jbyte *data, int length, void *ctx) {

    int cur_size = length;
    memcpy(inbuf, data, length);
    uint8_t *cur_ptr = inbuf;
    // Parse input stream to check if there is a valid frame.
    //std::cout << " in data  --  -- " << length<< std::endl;
    while (cur_size > 0) {
        int parsedLength = av_parser_parse2(parser, codecContext, &avpkt->data, &avpkt->size, (const uint8_t *) cur_ptr,
                                            cur_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        cur_ptr += parsedLength;
        cur_size -= parsedLength;
        LOGI("cur_size %d", cur_size);
        LOGI("parserLength %d", parsedLength);
        if (!avpkt->size) {
            continue;
        } else {
            int len, got_frame;
            NalInfo nalInfo;
            handleH264Header(cur_ptr - parsedLength, &nalInfo);
//            len = avcodec_decode_video2(codecContext, frame, &got_frame, avpkt);
            len = avcodec_send_packet(codecContext, avpkt);
            av_packet_unref(avpkt); //清除引用，防止内存泄漏
            if (len != 0) {
                LOGI("Error while decoding avcodec_send_packet %d\n", len);
                continue;
            }
            got_frame = avcodec_receive_frame(codecContext, frame);
            if (got_frame != 0) {
                LOGI("Error while decoding avcodec_receive_frame %d\n", got_frame);
                continue;
            }
            frame_count++;
            if (img_convert_ctx == NULL) {
                LOGE("img_convert_ctx is null");
                img_convert_ctx = sws_getContext(codecContext->width, codecContext->height,
                                                 codecContext->pix_fmt, codecContext->width,
                                                 codecContext->height, pixelFormat, SWS_BICUBIC,
                                                 NULL, NULL, NULL);

                renderParam = (RenderParam *) malloc(sizeof(RenderParam));
                renderParam->swsContext = img_convert_ctx;
                renderParam->avCodecContext = codecContext;
            }

            if (img_convert_ctx != NULL) {
                LOGE("handle_data");
                handle_data(frame, renderParam, ctx);
            }
        }
    }
}

void decoder::decodeToYuv(JNIEnv *env, jbyte *data, int length, jbyteArray dst) {
    LOGE("decode h264 length: %d ", length);
    int cur_size = length;
    memcpy(inbuf, data, length);
    uint8_t *cur_ptr = inbuf;
    while (cur_size > 0) {
        int parserLength = av_parser_parse2(parser, codecContext, &avpkt->data, &avpkt->size, cur_ptr, cur_size,
                                            AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        cur_ptr += parserLength;
        cur_size -= parserLength;
        LOGI("cur_size %d", cur_size);
        LOGI("parserLength %d", parserLength);

        if (!avpkt->size) {
            LOGI("avpacket size error %d", avpkt->size);
            continue;
        }
        NalInfo nalInfo;
        handleH264Header(cur_ptr - parserLength, &nalInfo);
        int len, got_frame;
//            len = avcodec_decode_video2(codecContext, frame, &got_frame, avpkt);
        len = avcodec_send_packet(codecContext, avpkt);
        if (len != 0) {
            LOGI("Error while decoding avcodec_send_packet %d\n", len);
            continue;
        }
        av_packet_unref(avpkt); //清除引用，防止内存泄漏
        got_frame = avcodec_receive_frame(codecContext, frame);
        if (got_frame != 0) {
            LOGI("Error while decoding avcodec_receive_frame %d\n", got_frame);
            continue;
        }
        frame_count++;
        LOGI("decode success");
        LOGI("frame format type %d ", frame->format);
        if (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P) {
            LOGI("frame type yuv420");
            int height = codecContext->height;
            int width = codecContext->width + 32;
            int buf_size = height * width * 3 / 2;
            char *buf = new char[buf_size];
            int i, j, k, a = 0;
            for (i = 0; i < height; i++) {
                memcpy(buf + a, frame->data[0] + i * frame->linesize[0], width);
                a += width;
            }
            for (j = 0; j < height / 2; j++) {
                memcpy(buf + a, frame->data[1] + j * frame->linesize[1], width / 2);
                a += width / 2;
            }
            for (k = 0; k < height / 2; k++) {
                memcpy(buf + a, frame->data[2] + k * frame->linesize[2], width / 2);
                a += width / 2;
            }
            int size = strlen(buf);
            int dst_size = env->GetArrayLength(dst);
            LOGI("width %d", width);
            LOGI("height %d", height);
            LOGI("buf size %d", buf_size);
            LOGI("decode buf size %d", size);
            LOGI("dst size %d", dst_size);
            env->SetByteArrayRegion(dst, 0, buf_size, (const jbyte *) buf);
            delete buf;
            buf = NULL;
            av_frame_unref(frame);
        }
    }
}

int decoder::handleH264Header(uint8_t *ptr, NalInfo *nalInfo) {
    int startIndex = 0;
    uint32_t *checkPtr = (uint32_t *) ptr;
    if (*checkPtr == 0x01000000) {  // 00 00 00 01
        startIndex = 4;
    } else if (*(checkPtr) == 0 && *(checkPtr + 1) & 0x01000000) {  // 00 00 00 00 01
        startIndex = 5;
    }

    if (!startIndex) {
        return -1;
    } else {
        ptr = ptr + startIndex;
        nalInfo->nal_unit_type = 0x1f & *ptr;
        if (nalInfo->nal_unit_type == 5 || nalInfo->nal_unit_type == 7 ||
            nalInfo->nal_unit_type == 8 || nalInfo->nal_unit_type == 2) {  //I frame
            LOGE("I frame");
        } else if (nalInfo->nal_unit_type == 1) {
            LOGE("P frame");
        }
    }
    return 0;
}

uint8_t *decoder::yuv420p_2_argb(AVFrame *frame, SwsContext *swsContext, AVCodecContext *avCodecContext,
                        enum AVPixelFormat format) {
    uint8_t *out_bufferRGB;
    pFrameRGB->width = frame->width;
    pFrameRGB->height = frame->height;

    //给pFrameRGB帧加上分配的内存;  //AV_PIX_FMT_ARGB
    int size = av_image_get_buffer_size(format, avCodecContext->width, avCodecContext->height, 1);
    out_bufferRGB = (uint8_t *) (av_malloc(size * sizeof(uint8_t)));
    //YUV to RGB
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_bufferRGB, format,
                         avCodecContext->width, avCodecContext->height, 1);
    sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0,
              avCodecContext->height, pFrameRGB->data, pFrameRGB->linesize);

    //利用libyuv转换，效率更高
//    int convert = libyuv::I420ToRGB565((const uint8 *) frame->data[0], frame->width,
//                       (const uint8 *) frame->data[1], frame->width / 2,
//                       (const uint8 *) frame->data[2], frame->width / 2,
//                       out_bufferRGB, frame->width, frame->width, frame->height);
//    LOGI("yuv convert %d", convert);
//    if(convert != 0) {
//        return NULL;
//    }
    return out_bufferRGB;
}

void decoder::handle_data(AVFrame *pFrame, void *param, void *ctx) {
    RenderParam *renderParam = (RenderParam *) param;
    uint8_t *bufferRGB = yuv420p_2_argb(pFrame, renderParam->swsContext,
                                        renderParam->avCodecContext, pixelFormat);
    if (bufferRGB == NULL) {
        LOGI("bufferRGB is null");
        return;
    }
    EnvPackage *envPackage = (EnvPackage *) ctx;
    ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(envPackage->env, *(envPackage->surface));
    if (aNativeWindow == NULL) {
        LOGI("ANativeWindow is null");
        return;
    }

    VoutInfo voutInfo;
    voutInfo.buffer = pFrameRGB->data[0];
    voutInfo.buffer_width = pFrameRGB->width;
    voutInfo.buffer_height = pFrameRGB->height;
    voutInfo.pix_format = native_pix_format;

    android_native_window_display(aNativeWindow, &voutInfo);
    ANativeWindow_release(aNativeWindow);
    //释放引用计数和内存
    av_frame_unref(pFrame);
    av_frame_unref(pFrameRGB);
    av_free(bufferRGB);
}

void decoder::close() {
    av_packet_free(&avpkt); //释放内存
    if (parser) {
        av_parser_close(parser);
        parser = NULL;
    }
    if (renderParam) {
        free(renderParam);
        renderParam = NULL;
    }
    avcodec_close(codecContext);
    av_free(codecContext);
    av_frame_free(&frame);
    av_frame_free(&pFrameRGB);
    if (img_convert_ctx != NULL) {
        sws_freeContext(img_convert_ctx);
        img_convert_ctx = NULL;
    }
    printf(" decoder close ..........\n");
}