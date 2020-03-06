package com.example.testffmpeg2

import android.Manifest
import android.content.pm.PackageManager
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioTrack
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File
import java.io.FileInputStream


class MainActivity : AppCompatActivity() {
    private lateinit var mTextView: TextView
    private lateinit var mVideoView: FFVideoView
    private val permissions = arrayOf<String>(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mTextView = findViewById(R.id.sample_text2)
        mVideoView = findViewById(R.id.videoView)

        // Example of a call to a native method
        sample_text.text = stringFromJNI()
        setupListener()

        ActivityCompat.requestPermissions(this, permissions, 3);
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        var granted = false
        if (requestCode == 3) {
            granted = grantResults[0] == PackageManager.PERMISSION_GRANTED;
        }
        Log.d("MainActivity", "------onRequestPermissionsResult $granted");
    }


    private fun setupListener() {
        findViewById<Button>(R.id.button_protocol).setOnClickListener {
            Log.d(TAG, "-----click protocol")
            setInfoText(FFUtils.urlProtocolInfo())
        }
        findViewById<Button>(R.id.button_codec).setOnClickListener {
            Log.d(TAG, "-----click codec")
            setInfoText(FFUtils.avCodecInfo())
        }
        findViewById<Button>(R.id.button_filter).setOnClickListener {
            //            setInfoText(FFUtils.avFilterInfo())
//            val rawPath = "sdcard/testMPEG/124840.264"
//            FFUtils.makeMp4(rawPath)
            playStream()
        }
        findViewById<Button>(R.id.button_format).setOnClickListener { setInfoText(FFUtils.avFormatInfo()) }
        findViewById<Button>(R.id.button_play).setOnClickListener {
            val videoPath = "sdcard/testMPEG/test.mp4"
            mVideoView.playVideo(videoPath)
        }
    }


    private fun playStream() {
        val channelConfig = AudioFormat.CHANNEL_OUT_MONO
        val minBufferSize =
            AudioTrack.getMinBufferSize(SAMPLE_RATE_INHZ, channelConfig, AUDIO_FORMAT)
        val audioTrack = AudioTrack(
            AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build(),
            AudioFormat.Builder().setSampleRate(SAMPLE_RATE_INHZ)
                .setEncoding(AUDIO_FORMAT)
                .setChannelMask(channelConfig)
                .build(),
            minBufferSize,
            AudioTrack.MODE_STREAM,
            AudioManager.AUDIO_SESSION_ID_GENERATE
        )
        audioTrack.play()

        val videoPath2 =
            Environment.getExternalStorageDirectory().toString() + "/testMPEG/record_temp_agc.pcm"
        val file = File(videoPath2)
        try {
            val fileInputStream = FileInputStream(file)
            val tempBuffer = ByteArray(minBufferSize)
            while (fileInputStream.available() > 0) {
                val readCount = fileInputStream.read(tempBuffer)
                if (readCount == AudioTrack.ERROR_INVALID_OPERATION ||
                    readCount == AudioTrack.ERROR_BAD_VALUE
                ) {
                    continue
                }
                if (readCount != 0 && readCount != -1) {
                    audioTrack.write(tempBuffer, 0, readCount)
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }


    }


    private fun setInfoText(text: String) {
        mTextView.text = text
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private external fun stringFromJNI(): String

    companion object {
        const val TAG = "MainActivity"
        const val SAMPLE_RATE_INHZ = 8000
        const val CHANNEL_CONFIG = AudioFormat.CHANNEL_IN_MONO
        const val AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")

            listOf("avutil", "avcodec", "avformat", "swresample", "swscale").forEach {
                System.loadLibrary(it)
            }
        }
    }
}
