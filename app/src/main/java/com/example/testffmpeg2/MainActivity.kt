package com.example.testffmpeg2

import android.Manifest
import android.content.pm.PackageManager
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioTrack
import android.os.*
import android.util.Log
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File
import java.io.FileInputStream
import java.lang.ref.WeakReference
import java.nio.ByteBuffer


class MainActivity : AppCompatActivity() {
    private lateinit var mTextView: TextView
    private lateinit var mVideoView: FFVideoView
    private val permissions = arrayOf<String>(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    )

    private lateinit var dashboardHandler: Handler
    private val outClass = WeakReference<MainActivity>(this)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mTextView = findViewById(R.id.sample_text2)
        mVideoView = findViewById(R.id.videoView)

        // Example of a call to a native method
        sample_text2.text = stringFromJNI()
        setupListener()
        dashboardHandler = MyHandle(outClass)

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
//            playStream()

//            startCountTime()

            setupAudioTrack()
            startCount2(true)
        }
        findViewById<Button>(R.id.button_format).setOnClickListener { setInfoText(FFUtils.avFormatInfo()) }
        findViewById<Button>(R.id.button_play).setOnClickListener {
            val videoPath = "sdcard/testMPEG/test.mp4"
            mVideoView.playVideo(videoPath)
        }
        findViewById<Button>(R.id.startQueue).setOnClickListener {
            FFUtils.startQueue(bufferAudio)
        }
        findViewById<Button>(R.id.popQueue).setOnClickListener {
            FFUtils.popQueue()
            audioTrack.write(bufferAudio, 0, AudioTrack.WRITE_NON_BLOCKING)
        }
        findViewById<Button>(R.id.testByteArray).setOnClickListener {
            val array = FFUtils.testByteArray()
            dashboardHandler.post { audioTrack.write(array, 0, array.size) }
            Log.d(TAG, "------array size ${array.size}")
            for (i in 1..30) {
                Log.d(TAG, "------byte array ${array[i]}")
            }
//            for (data in array) {
//            }
        }
        findViewById<Button>(R.id.playLocal).setOnClickListener {
            setupAudioTrack()
            playAudioFile()
        }

    }

    private val bufferAudio = ByteBuffer.allocateDirect(360)
    private lateinit var audioTrack: AudioTrack
    private lateinit var tempBuffer: ByteArray
    private fun setupAudioTrack() {
        if (::audioTrack.isInitialized) return
        val channelConfig = AudioFormat.CHANNEL_OUT_MONO
        val minBufferSize =
            AudioTrack.getMinBufferSize(SAMPLE_RATE_INHZ, channelConfig, AUDIO_FORMAT)
        tempBuffer = ByteArray(minBufferSize)
        audioTrack = AudioTrack(
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
        Log.d(TAG, "------setupAudioTrack minBufferSize $minBufferSize")
    }

    private fun playAudioFile() {
        val videoPath2 =
            Environment.getExternalStorageDirectory().toString() + "/testMPEG/record_temp_agc.pcm"
        val file = File(videoPath2)
        try {
            val fileInputStream = FileInputStream(file)
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

    private var count = 0
    private var countTime: CountDownTimer? = null
    private fun startCountTime(start: Boolean = true) {
        if (start) {
            count = 0
            if (countTime == null) {
                countTime = CountTime(1 * 1000, 50) {
                    count += 1
                    Log.d(TAG, "------startCountTime count $count")
                }
            }
            countTime?.start()
        } else {
            countTime?.cancel()
        }
    }

    private var startTS = 0L
    private fun startCount2(start: Boolean = false) {
        if (start) {
            count = 0
            startTS = System.currentTimeMillis()
        } else {
            if (System.currentTimeMillis() - startTS > 15000) {
                dashboardHandler.removeMessages(HANDLER_M)
                return
            }
            FFUtils.popQueue()
//            val array = FFUtils.testByteArray()
//            dashboardHandler.post { audioTrack.write(array, 0, array.size) }
            count += 1
            Log.d(TAG, "------startCount2 count $count")
        }
        dashboardHandler.removeMessages(HANDLER_M)
        dashboardHandler.sendEmptyMessageDelayed(HANDLER_M, 50)
    }


    private fun setInfoText(text: String) {
        mTextView.text = text
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private external fun stringFromJNI(): String

    class CountTime(
        millisInFuture: Long,
        countDownInterval: Long,
        private val callback: (Long) -> Unit
    ) :
        CountDownTimer(millisInFuture, countDownInterval) {
        override fun onFinish() {
            Log.d("CountTime", "-----onFinish")
        }

        override fun onTick(millisUntilFinished: Long) {
            callback(millisUntilFinished)
            Log.d("CountTime", "-----millisUntilFinished $millisUntilFinished")
        }

    }

    class MyHandle(private val outClass: WeakReference<MainActivity>) : Handler() {
        override fun handleMessage(msg: Message) {
            super.handleMessage(msg)
            outClass.get()?.startCount2()

            Log.d(TAG, "----handle message ${msg.what}")
        }
    }

    companion object {
        const val HANDLER_M = 1
        const val TAG = "MainActivity"
        const val SAMPLE_RATE_INHZ = 8000
        const val CHANNEL_CONFIG = AudioFormat.CHANNEL_IN_MONO
        const val AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")

//            listOf("avutil", "avcodec", "avformat", "swresample", "swscale").forEach {
//                System.loadLibrary(it)
//            }
        }
    }
}
