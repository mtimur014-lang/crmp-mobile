package com.crmp.sampmobile

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class LauncherActivity : AppCompatActivity() {

    data class SampServer(val title: String, val ip: String, val port: Int)

    private val serverList = listOf(
        SampServer("CRMP #1 | MOSCOW (Test Pawn)", "127.0.0.1", 7777),
        SampServer("CRMP #2 | ARZAMAS (VPS)", "127.0.0.1", 7778),
        SampServer("СВОЙ IP / ПОРТ (ВРУЧНУЮ)", "127.0.0.1", 7777)
    )

    private lateinit var etNickname: EditText
    private lateinit var spServer: Spinner
    private lateinit var etServerIp: EditText
    private lateinit var etServerPort: EditText
    private lateinit var tvCachePath: TextView
    private lateinit var tvCacheStatus: TextView
    private lateinit var btnPlay: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_launcher)

        etNickname = findViewById(R.id.etNickname)
        spServer = findViewById(R.id.spServer)
        etServerIp = findViewById(R.id.etServerIp)
        etServerPort = findViewById(R.id.etServerPort)
        tvCachePath = findViewById(R.id.tvCachePath)
        tvCacheStatus = findViewById(R.id.tvCacheStatus)
        btnPlay = findViewById(R.id.btnPlay)

        prepareSampDirectories()
        loadSavedSettings()
        setupServerSpinner()
        inspectGameCache()

        btnPlay.setOnClickListener {
            launchSampClient()
        }
    }

    private fun getBaseGameDir(): File {
        return getExternalFilesDir(null) ?: filesDir
    }

    private fun prepareSampDirectories() {
        val baseDir = getBaseGameDir()
        listOf("SAMP", "texdb", "models", "data", "audio", "lib").forEach { folder ->
            val dir = File(baseDir, folder)
            if (!dir.exists()) dir.mkdirs()
        }
    }

    private fun loadSavedSettings() {
        val prefs = getSharedPreferences("crmp_samp_prefs", MODE_PRIVATE)
        etNickname.setText(prefs.getString("nick", "Player_CRMP"))
        etServerIp.setText(prefs.getString("ip", "127.0.0.1"))
        etServerPort.setText(prefs.getInt("port", 7777).toString())
    }

    private fun setupServerSpinner() {
        val adapter = ArrayAdapter(
            this,
            android.R.layout.simple_spinner_dropdown_item,
            serverList.map { "${it.title} (${it.ip}:${it.port})" }
        )
        spServer.adapter = adapter
        spServer.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(parent: AdapterView<*>?, view: View?, pos: Int, id: Long) {
                if (pos < 2) {
                    etServerIp.setText(serverList[pos].ip)
                    etServerPort.setText(serverList[pos].port.toString())
                }
            }
            override fun onNothingSelected(parent: AdapterView<*>?) {}
        }
    }

    private fun inspectGameCache() {
        val baseDir = getBaseGameDir()
        tvCachePath.text = "Папка кэша: ${baseDir.absolutePath}"

        val gtasaLib = File(baseDir, "lib/libGTASA.so")
        val gta3Img = File(baseDir, "texdb/gta3.img")
        val sampImg = File(baseDir, "texdb/samp.img")

        val statusLines = StringBuilder()
        statusLines.append("• libsamp.so (C++ RakNet Engine): ВШИТ В APK [OK]\n")
        statusLines.append(if (gtasaLib.exists()) "• libGTASA.so: НАЙДЕН [OK]\n" else "• libGTASA.so: НЕ НАЙДЕН (закиньте в папку lib/)\n")
        statusLines.append(if (gta3Img.exists()) "• texdb/gta3.img: НАЙДЕН [OK]\n" else "• texdb/gta3.img: ОЖИДАЕТСЯ КЭШ GTA:SA 2.00\n")
        statusLines.append(if (sampImg.exists()) "• texdb/samp.img: НАЙДЕН [OK]" else "• texdb/samp.img: БУДЕТ ПОДКЛЮЧЕН ПРИ ЗАГРУЗКЕ")

        tvCacheStatus.text = statusLines.toString()
    }

    private fun writeSampSettingsIni(nick: String, ip: String, port: Int) {
        val iniFile = File(getBaseGameDir(), "SAMP/settings.ini")
        val content = buildString {
            appendLine("[client]")
            appendLine("name=$nick")
            appendLine("version=0.3.7")
            appendLine("timestamp=1")
            appendLine("chat_lines=10")
            appendLine("")
            appendLine("[server]")
            appendLine("host=$ip")
            appendLine("port=$port")
            appendLine("")
            appendLine("[gui]")
            appendLine("Font=Arial.ttf")
            appendLine("FontSize=28.0")
            appendLine("fps_limit=60")
        }
        iniFile.writeText(content)
    }

    private fun launchSampClient() {
        val nick = etNickname.text.toString().trim()
        val ip = etServerIp.text.toString().trim()
        val port = etServerPort.text.toString().trim().toIntOrNull() ?: 7777

        if (nick.length < 3) {
            Toast.makeText(this, "Введите никнейм (минимум 3 символа)", Toast.LENGTH_SHORT).show()
            return
        }
        if (ip.isEmpty()) {
            Toast.makeText(this, "Укажите IP сервера", Toast.LENGTH_SHORT).show()
            return
        }

        getSharedPreferences("crmp_samp_prefs", MODE_PRIVATE).edit()
            .putString("nick", nick)
            .putString("ip", ip)
            .putInt("port", port)
            .apply()

        writeSampSettingsIni(nick, ip, port)

        val intent = Intent(this, GameActivity::class.java).apply {
            putExtra("NICK", nick)
            putExtra("IP", ip)
            putExtra("PORT", port)
            putExtra("GAME_DIR", getBaseGameDir().absolutePath)
        }
        startActivity(intent)
    }
}
