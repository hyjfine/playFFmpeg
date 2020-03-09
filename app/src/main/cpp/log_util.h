//
// Created by hyjfine on 2020/2/29.
//
#include <android/log.h>
#ifndef TEST_FFMPEG_LOG_UTIL_H
#define TEST_FFMPEG_LOG_UTIL_H
#define LOG_TAG "JNI Call"
#define DEBUG 1
#if DEBUG
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#else
#define LOGI(...)
#define LOGD(...)
#define LOGW(...)
#endif
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif //TEST_FFMPEG_LOG_UTIL_H
