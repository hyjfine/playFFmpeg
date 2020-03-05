package com.example.testffmpeg2;

import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Environment;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;

public class FFVideoView extends SurfaceView {

    Surface mSurface;

    public FFVideoView(Context context) {
        super(context);

        init();
    }

    public FFVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);

        init();
    }

    public FFVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        init();
    }

    public FFVideoView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);

        init();
    }

    private void init() {
        getHolder().setFormat(PixelFormat.RGBA_8888);
        mSurface = getHolder().getSurface();

    }

    public void playVideo(final String videoPath) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d("FFVideoView", "-----run: playVideo");
                String videoPath2 = Environment.getExternalStorageDirectory() + "/testMPEG/124840.264";
                FFUtils.playVideo2(videoPath2, mSurface);
            }
        }).start();
    }
}