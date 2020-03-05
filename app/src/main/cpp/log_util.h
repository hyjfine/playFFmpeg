//
// Created by hyjfine on 2020/2/29.
//
#ifndef TEST_FFMPEG_LOG_UTIL_H
#define TEST_FFMPEG_LOG_UTIL_H

#define LOG_TAG "JNI Call"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#endif //TEST_FFMPEG_LOG_UTIL_H
