package com.crmp.sampmobile

import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class GameActivity : AppCompatActivity() {

    companion object {
        init {
            System.loadLibrary("samp")
        }
    }

    external fun nativeInitSamp(gameDir: String, nick: String, host: String, port: Int): String
    external fun nativeQueryServer(host: String, port: Int): String
    external fun nativeSendRakNetHandshake(host: String, port: Int): String
    external fun nativeOnSurfaceCreated()
    external fun nativeOnSurfaceChanged(width: Int, height: Int)
    external fun nativeOnDrawFrame()
    external fun nativeShutdown()

    private lateinit var glSurfaceView: GLSurfaceView
    private lateinit var tvHudHeader: TextView
    private lateinit var tvSampChat: TextView
    private lateinit var svChatScroll: ScrollView
    private lateinit var etChatCommand: EditText

    private var nick: String = "Player_CRMP"
    private var host: String = "127.0.0.1"
    private var port: Int = 7777
    private var gameDir: String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_game)

        nick = intent.getStringExtra("NICK") ?: "Player_CRMP"
        host = intent.getStringExtra("IP") ?: "127.0.0.1"
        port = intent.getIntExtra("PORT", 7777)
        gameDir = intent.getStringExtra("GAME_DIR") ?: (getExternalFilesDir(null)?.absolutePath ?: "")

        glSurfaceView = findViewById(R.id.glSurfaceView)
        tvHudHeader = findViewById(R.id.tvHudHeader)
        tvSampChat = findViewById(R.id.tvSampChat)
        svChatScroll = findViewById(R.id.svChatScroll)
        etChatCommand = findViewById(R.id.etChatCommand)

        setupOpenGLSurface()

        tvHudHeader.text = "SA-MP 0.3.7 | $nick -> $host:$port"
        val initReport = nativeInitSamp(gameDir, nick, host, port)
        appendChatLine(initReport)

        findViewById<Button>(R.id.btnPingServer).setOnClickListener {
            runServerQueryAsync()
        }

        findViewById<Button>(R.id.btnSendChat).setOnClickListener {
            val text = etChatCommand.text.toString().trim()
            etChatCommand.setText("")
            if (text.isNotEmpty()) {
                handleUserInput(text)
            }
        }

        findViewById<Button>(R.id.btnExitGame).setOnClickListener {
            finish()
        }

        runServerQueryAsync()
    }

    private fun setupOpenGLSurface() {
        glSurfaceView.setEGLContextClientVersion(3)
        glSurfaceView.setRenderer(object : GLSurfaceView.Renderer {
            override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
                nativeOnSurfaceCreated()
            }

            override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
                nativeOnSurfaceChanged(width, height)
            }

            override fun onDrawFrame(gl: GL10?) {
                nativeOnDrawFrame()
            }
        })
        glSurfaceView.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
    }

    private fun runServerQueryAsync() {
        appendChatLine("[RakNet] Отправка SA-MP Query ('i') и Handshake на $host:$port ...")
        Thread {
            val queryRes = nativeQueryServer(host, port)
            val handshakeRes = nativeSendRakNetHandshake(host, port)
            runOnUiThread {
                appendChatLine(queryRes)
                appendChatLine(handshakeRes)
            }
        }.start()
    }

    private fun handleUserInput(cmd: String) {
        when {
            cmd.equals("/query", ignoreCase = true) || cmd.equals("/ping", ignoreCase = true) -> {
                runServerQueryAsync()
            }
            cmd.equals("/help", ignoreCase = true) -> {
                appendChatLine("[SAMP] Доступные команды клиента: /query (опрос сервера), /connect (RakNet пакет), /cache (путь кэша)")
            }
            cmd.equals("/cache", ignoreCase = true) -> {
                appendChatLine("[FS] Папка кэша: $gameDir")
            }
            cmd.equals("/connect", ignoreCase = true) -> {
                runServerQueryAsync()
            }
            else -> {
                appendChatLine("[$nick]: $cmd")
            }
        }
    }

    private fun appendChatLine(line: String) {
        tvSampChat.append(line + "\n")
        svChatScroll.post {
            svChatScroll.fullScroll(View.FOCUS_DOWN)
        }
    }

    override fun onResume() {
        super.onResume()
        glSurfaceView.onResume()
    }

    override fun onPause() {
        super.onPause()
        glSurfaceView.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        nativeShutdown()
    }
}
