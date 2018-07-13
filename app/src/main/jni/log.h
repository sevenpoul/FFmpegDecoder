//
// Created by vonchenchen on 10/14/16.
//

#ifndef _LOG_H
#define _LOG_H

#include <jni.h>
#include <android/log.h>
#include <string.h>

#include <sys/time.h>

#define   LOG_TAG    "NATIVE_LOG"
#define   DEBUG 1
#define   LOGI(...) { if(DEBUG){__android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__);}}

#define   LOGE(...) { if(DEBUG){__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__);}}


//class log {
//public:
//    static jstring getCurrentTime();
//};


#endif