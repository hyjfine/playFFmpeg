#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string>
#include <unistd.h>

#include "log_util.h"

// for native media
#include <OMXAL/OpenMAXAL.h>
#include <OMXAL/OpenMAXAL_Android.h>

extern "C" {
#include "libavformat/version.h"
#include "libavcodec/avcodec.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include <libavutil/imgutils.h>

#include "Muxer.h"
#include "NALUParser.h"
#include "core_player.h"

}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_testffmpeg2_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    const char *version = avcodec_configuration();
    return env->NewStringUTF(version);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_testffmpeg2_FFUtils_urlProtocolInfo(JNIEnv *env, jclass clazz) {
    char info[40000] = {0};
    av_register_all();

    struct URLProtocol *pup = NULL;

    struct URLProtocol **p_temp = (struct URLProtocol **) &pup;
    avio_enum_protocols((void **) p_temp, 0);

    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 0));
    }
    pup = NULL;
    avio_enum_protocols((void **) p_temp, 1);
    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 1));
    }
    LOGI("%s", info);
    return env->NewStringUTF(info);

}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_testffmpeg2_FFUtils_avFormatInfo(JNIEnv *env, jclass clazz) {
    char info[40000] = {0};

    av_register_all();

    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    while (if_temp != NULL) {
        sprintf(info, "%sInput: %s\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    while (of_temp != NULL) {
        sprintf(info, "%sOutput: %s\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    LOGI("%s", info);
    return env->NewStringUTF(info);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_testffmpeg2_FFUtils_avCodecInfo(JNIEnv *env, jclass clazz) {
    char info[40000] = {0};

    av_register_all();

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%sdecode:", info);
        } else {
            sprintf(info, "%sencode:", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s(video):", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s(audio):", info);
                break;
            case AVMEDIA_TYPE_SUBTITLE:
                sprintf(info, "%s(subtitle):", info);
                break;
            default:
                sprintf(info, "%s(other):", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }
    LOGI("%s", info);
    return env->NewStringUTF(info);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_testffmpeg2_FFUtils_avFilterInfo(JNIEnv *env, jclass clazz) {
    char info[40000] = {0};
//    avfilter_register_all();
//
//    AVFilter *f_temp = (AVFilter *) avfilter_next(NULL);
//    while (f_temp != NULL) {
//        sprintf(info, "%s%s\n", info, f_temp->name);
//        f_temp = f_temp->next;
//    }
//    LOGI("%s", info);
    return env->NewStringUTF(info);
}

static void parser_callback(void *opaque, Nalu *nalu) {
    dump_nalu(nalu);
    Player *player = (Player *) opaque;
    player_write_video_frame(player, nalu->data, nalu->size);
}

// native window
ANativeWindow *nativeWindow;
ANativeWindow_Buffer windowBuffer;

static void
render_frame(void *opaque, uint8_t *const data[8], const int size[8]) {

    ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);

    // 获取stride
    uint8_t *dst = (uint8_t *) windowBuffer.bits;
    uint8_t *src = data[0];
    int dstStride = windowBuffer.stride * 4;
    int srcStride = size[0];
//    LOGD("-----stride %d %d", dstStride, srcStride);

    // 由于window的stride和帧的stride不同,因此需要逐行复制
    for (int i = 0; i < 1376; i++) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    usleep(0.05 * 1000000);
    ANativeWindow_unlockAndPost(nativeWindow);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testffmpeg2_FFUtils_playVideo2(JNIEnv *env, jclass clazz, jstring video_path,
                                                jobject surface) {
    const char *videoPath = env->GetStringUTFChars(video_path, 0);
    LOGI("PlayVideo: %s", videoPath);

    if (videoPath == NULL) {
        LOGE("videoPath is null");
        return;
    }

    FILE *file = fopen(videoPath, "r");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    //----
    AVFrame *renderFrame = av_frame_alloc();

    if (renderFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return;
    }

    nativeWindow = ANativeWindow_fromSurface(env, surface);

    // get video width , height
    LOGI("get video width , height");
    int videoWidth = 1536;
    int videoHeight = 1376;
    LOGI("VideoSize: [%d,%d]", videoWidth, videoHeight);

    // 设置native window的buffer大小,可自动拉伸
    LOGI("set native window");
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);


    uint8_t *data = (uint8_t *) malloc(length);
    fread(data, length, 1, file);
    fclose(file);

    PlayerContext *context = NULL;
    Player *player = player_alloc(&context);
    context->video_callback = render_frame;
    player_open(player);

    NaluParser *parser = parser_alloc();
    parser->opaque = player;
    parser->callback = parser_callback;

    parser_parse(parser, data, length);

    player_close(player);
    player_free(&player);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testffmpeg2_FFUtils_playVideo(JNIEnv *env, jclass clazz, jstring video_path,
                                               jobject surface) {

    const char *videoPath = env->GetStringUTFChars(video_path, 0);
    LOGI("PlayVideo: %s", videoPath);

    if (videoPath == NULL) {
        LOGE("videoPath is null");
        return;
    }

    AVFormatContext *formatContext = avformat_alloc_context();

    // open video file
    LOGI("Open video file");
    if (avformat_open_input(&formatContext, videoPath, NULL, NULL) != 0) {
        LOGE("Cannot open video file: %s\n", videoPath);
        return;
    }

    // Retrieve stream information
    LOGI("Retrieve stream information");
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("Cannot find stream information.");
        return;
    }

    // Find the first video stream
    LOGI("Find video stream");
    int video_stream_index = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
        }
    }

    if (video_stream_index == -1) {
        LOGE("No video stream found.");
        return; // no video stream found.
    }

    // Get a pointer to the codec context for the video stream
    LOGI("Get a pointer to the codec context for the video stream");
    AVCodecParameters *codecParameters = formatContext->streams[video_stream_index]->codecpar;

    // Find the decoder for the video stream
    LOGI("Find the decoder for the video stream");
    AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
    if (codec == NULL) {
        LOGE("Codec not found.");
        return; // Codec not found
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);

    if (codecContext == NULL) {
        LOGE("CodecContext not found.");
        return; // CodecContext not found
    }

    // fill CodecContext according to CodecParameters
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        LOGD("Fill CodecContext failed.");
        return;
    }

    // init codex context
    LOGI("open Codec");
    if (avcodec_open2(codecContext, codec, NULL)) {
        LOGE("Init CodecContext failed.");
        return;
    }

    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;

    // Allocate av packet
    AVPacket *packet = av_packet_alloc();
    if (packet == NULL) {
        LOGD("Could not allocate av packet.");
        return;
    }

    // Allocate video frame
    LOGI("Allocate video frame");
    AVFrame *frame = av_frame_alloc();
    // Allocate render frame
    LOGI("Allocate render frame");
    AVFrame *renderFrame = av_frame_alloc();

    if (frame == NULL || renderFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return;
    }

    // Determine required buffer size and allocate buffer
    LOGI("Determine required buffer size and allocate buffer");
    int size = av_image_get_buffer_size(dstFormat, codecContext->width, codecContext->height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(size * sizeof(uint8_t));
    av_image_fill_arrays(renderFrame->data, renderFrame->linesize, buffer, dstFormat,
                         codecContext->width, codecContext->height, 1);

    // init SwsContext
    LOGI("init SwsContext");
    struct SwsContext *swsContext = sws_getContext(codecContext->width,
                                                   codecContext->height,
                                                   codecContext->pix_fmt,
                                                   codecContext->width,
                                                   codecContext->height,
                                                   dstFormat,
                                                   SWS_BILINEAR,
                                                   NULL,
                                                   NULL,
                                                   NULL);
    if (swsContext == NULL) {
        LOGE("Init SwsContext failed.");
        return;
    }

    // native window
    LOGI("native window");
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_Buffer windowBuffer;

    // get video width , height
    LOGI("get video width , height");
    int videoWidth = codecContext->width;
    int videoHeight = codecContext->height;
    LOGI("VideoSize: [%d,%d]", videoWidth, videoHeight);

    // 设置native window的buffer大小,可自动拉伸
    LOGI("set native window");
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);


    LOGI("read frame");
    while (av_read_frame(formatContext, packet) == 0) {
        // Is this a packet from the video stream?
        if (packet->stream_index == video_stream_index) {

            // Send origin data to decoder
            int sendPacketState = avcodec_send_packet(codecContext, packet);
            if (sendPacketState == 0) {
                LOGD("向解码器-发送数据");

                int receiveFrameState = avcodec_receive_frame(codecContext, frame);
                if (receiveFrameState == 0) {
                    LOGD("从解码器-接收数据");
                    // lock native window buffer
                    ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);

                    // 格式转换
                    sws_scale(swsContext, (uint8_t const *const *) frame->data,
                              frame->linesize, 0, codecContext->height,
                              renderFrame->data, renderFrame->linesize);

                    // 获取stride
                    uint8_t *dst = (uint8_t *) windowBuffer.bits;
                    uint8_t *src = (renderFrame->data[0]);
                    int dstStride = windowBuffer.stride * 4;
                    int srcStride = renderFrame->linesize[0];
                    LOGD("-----stride %d %d", dstStride, srcStride);

                    // 由于window的stride和帧的stride不同,因此需要逐行复制
                    for (int i = 0; i < videoHeight; i++) {
                        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
                    }

                    ANativeWindow_unlockAndPost(nativeWindow);
                } else if (receiveFrameState == AVERROR(EAGAIN)) {
                    LOGD("从解码器-接收-数据失败：AVERROR(EAGAIN)");
                } else if (receiveFrameState == AVERROR_EOF) {
                    LOGD("从解码器-接收-数据失败：AVERROR_EOF");
                } else if (receiveFrameState == AVERROR(EINVAL)) {
                    LOGD("从解码器-接收-数据失败：AVERROR(EINVAL)");
                } else {
                    LOGD("从解码器-接收-数据失败：未知");
                }
            } else if (sendPacketState == AVERROR(EAGAIN)) {//发送数据被拒绝，必须尝试先读取数据
                LOGD("向解码器-发送-数据包失败：AVERROR(EAGAIN)");//解码器已经刷新数据但是没有新的数据包能发送给解码器
            } else if (sendPacketState == AVERROR_EOF) {
                LOGD("向解码器-发送-数据失败：AVERROR_EOF");
            } else if (sendPacketState == AVERROR(EINVAL)) {//遍解码器没有打开，或者当前是编码器，也或者需要刷新数据
                LOGD("向解码器-发送-数据失败：AVERROR(EINVAL)");
            } else if (sendPacketState == AVERROR(ENOMEM)) {//数据包无法压如解码器队列，也可能是解码器解码错误
                LOGD("向解码器-发送-数据失败：AVERROR(ENOMEM)");
            } else {
                LOGD("向解码器-发送-数据失败：未知");
            }

        }
//        usleep(0.05 * 1000000);
        av_packet_unref(packet);
    }


    //内存释放
    LOGI("release memory");
    ANativeWindow_release(nativeWindow);
    av_frame_free(&frame);
    av_frame_free(&renderFrame);
    av_packet_free(&packet);
    avcodec_close(codecContext);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(video_path, videoPath);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testffmpeg2_FFUtils_makeMp4(JNIEnv *env, jclass clazz, jstring raw_path) {

    const char *videoPath = env->GetStringUTFChars(raw_path, 0);
    LOGI("make mp4 path : %s", videoPath);

    if (videoPath == NULL) {
        LOGE("videoPath is null");
        return;
    }

    FILE *file = fopen(videoPath, "r");

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *data = (uint8_t *) malloc(length);
    fread(data, length, 1, file);

    Muxer *muxer = muxer_alloc("/storage/emulated/0/testMPEG/testMuxer1.mp4");
    muxer_open(muxer);
    int ret = muxer_write_video_frame(muxer, data, length);
    muxer_close(muxer);
    muxer_free(muxer);
    LOGD("-----makeMp4 %u length %li", ret, length);

}