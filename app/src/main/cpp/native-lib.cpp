#include <jni.h>
#include <string>

extern "C" {
#include "libavformat/version.h"
#include "libavcodec/avcodec.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_testffmpeg2_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    const char *version = avcodec_configuration();
    return env->NewStringUTF(version);
}
