package com.example.testffmpeg2;

import android.view.Surface;

import java.nio.ByteBuffer;

public class FFUtils {
    static {
        System.loadLibrary("native-lib");
    }

    public static native String urlProtocolInfo();

    public static native String avFormatInfo();

    public static native String avCodecInfo();

    public static native String avFilterInfo();

    public static native void playVideo(String videoPath, Surface surface);

    public static native void playVideo2(String videoPath, Surface surface);

    public static native void makeMp4(String rawPath);

    public static native void startQueue(ByteBuffer buffer1);

    public static native void popQueue();

    public static native byte[] testByteArray();
}
