//
// Created by lishanshan on 2018/5/31.
//
#include <jni.h>
/* Header for class com_znv_decoder_NativeLib */

#include "log.h"
#include "decoder.h"
#include "android_native_window.h"
#include "hard.h"

#ifndef _Included_com_znv_decoder_NativeLib
#define _Included_com_znv_decoder_NativeLib
#ifdef __cplusplus
extern "C" {
#endif

enum AVPixelFormat pixelFormat = AV_PIX_FMT_RGB565LE;

/*
 * Class:     com_znv_decoder_NativeLib
 * Method:    initDecode
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_znv_decoder_NativeLib_initDecode
  (JNIEnv *env, jobject obj, jint width, jint height) {
    decoder *p = new decoder();
    long init = p->initialize(pixelFormat, width, height);
    if(init != 0) {
        return init;
    }
    jlong res = (jlong) p;
    LOGI("init surface %ld", res);
    return res;
}

/*
 * Class:     com_znv_decoder_NativeLib
 * Method:    initHardDecode
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_znv_decoder_NativeLib_initHardDecode
        (JNIEnv *env, jobject obj, jobject surface, jint width, jint height) {
    hard *p = new hard();
    p->initHard(env, width, height, surface);
    jlong res = (jlong) p;
    LOGI("init hard decode %ld", res);
    return res;
}

/*
 * Class:     com_znv_decoder_NativeLib
 * Method:    decodeH264
 * Signature: ([BIJLjava/lang/Object;)J
 */
JNIEXPORT void JNICALL Java_com_znv_decoder_NativeLib_decodeH264
  (JNIEnv *env, jobject obj, jbyteArray jdata, jint length, jlong this_obj_long, jobject surface) {

    decoder *this_obj = (decoder *)this_obj_long;
    jbyte *data = env->GetByteArrayElements(jdata, JNI_FALSE);

    if(data != NULL) {
        EnvPackage package;
        package.env = env;
        package.obj = &obj;
        package.surface = &surface;

        int len = 0;
        while (1) {
            if (length > INBUF_SIZE) {
                len = INBUF_SIZE;
                length -= INBUF_SIZE;
            } else if (length > 0 && length <= INBUF_SIZE) {
                len = length;
                length = 0;
            } else {
                break;
            }
            //decode h264 cdata to yuv and save yuv data to avFrame which would be passed to handle_data
            this_obj->decodeFrame(data, len, &package);
            data = data + len;
        }
    } else {
        LOGE("stream data is NULL");
    }
//    env->ReleaseByteArrayElements(jdata, data, 0);
//    LOGE("decode release");
}

/*
 * Class:     com_znv_decoder_NativeLib
 * Method:    hardDecode
 * Signature: ([BIJLjava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_znv_decoder_NativeLib_hardDecode
        (JNIEnv *env, jobject obj, jbyteArray jdata, jint length, jlong this_obj_long) {
    hard *this_obj = (hard *)this_obj_long;
    jbyte *data = env->GetByteArrayElements(jdata, JNI_FALSE);

    if(data != NULL) {
        int len = 0;
        while (1) {
            if (length > INBUF_SIZE) {
                len = INBUF_SIZE;
                length -= INBUF_SIZE;
            } else if (length > 0 && length <= INBUF_SIZE) {
                len = length;
                length = 0;
            } else {
                break;
            }
            //decode h264 cdata to yuv and save yuv data to avFrame which would be passed to handle_data
            this_obj->hardDecode(data, len);
            data = data + len;
            LOGE("hard decode length: %d ", length);
        }
    } else {
        LOGE("hard stream data is NULL");
    }
//    env->ReleaseByteArrayElements(jdata, data, 0);
}

/*
 * Class:     com_znv_decoder_NativeLib
 * Method:    releaseDecode
 * Signature: (J)J
 */
JNIEXPORT void JNICALL Java_com_znv_decoder_NativeLib_releaseDecode
  (JNIEnv *env, jobject obj, jlong this_obj_long) {
    decoder *this_obj = (decoder *)this_obj_long;

    this_obj->close();
    delete this_obj;
}

/*
 * Class:     com_znv_decoder_NativeLib
 * Method:    releaseHardDecode
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_znv_decoder_NativeLib_releaseHardDecode
        (JNIEnv *env, jobject obj, jlong this_obj_long) {
    hard *this_obj = (hard *) this_obj_long;

    this_obj->closeHard();
    delete this_obj;
}

/*
 * Class:     com_znv_decoder_NativeLib
 * Method:    decodeToYuv
 * Signature: ([BIJ[B)V
 */
JNIEXPORT void JNICALL Java_com_znv_decoder_NativeLib_decodeToYuv
        (JNIEnv *env, jobject obj, jbyteArray src, jint length, jlong this_obj_long, jbyteArray dst) {
    decoder *this_obj = (decoder *) this_obj_long;
    jbyte *data = env->GetByteArrayElements(src, NULL);

    if(data != NULL) {
        int len = 0;
        while (1) {
            if (length > INBUF_SIZE) {
                len = INBUF_SIZE;
                length -= INBUF_SIZE;
            } else if (length > 0 && length <= INBUF_SIZE) {
                len = length;
                length = 0;
            } else {
                break;
            }
            this_obj->decodeToYuv(env, data, len, dst);
            data = data + len;
        }
    } else {
        LOGE("stream h264 data is NULL");
    }
//    env->ReleaseByteArrayElements(src, data, 0);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)//这个类似android的生命周期，加载jni的时候会自己调用
{
    LOGI("ffmpeg JNI_OnLoad");
    av_jni_set_java_vm(vm, reserved);
    return JNI_VERSION_1_6;
}

#ifdef __cplusplus
}
#endif
#endif
