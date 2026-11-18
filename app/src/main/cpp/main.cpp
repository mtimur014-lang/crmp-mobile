#include <cstdio>
#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <string>
#include <sstream>

#include "net/RakNetClient.h"
#include "game/MemoryHook.h"
#include "gui/SampHudRenderer.h"

#define LOG_TAG "LIBSAMP_CRMP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void* g_hGTASA = nullptr;
static uintptr_t g_libGTASABase = 0;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* /*vm*/, void* /*reserved*/) {
    LOGI("libsamp.so JNI_OnLoad initialized (SA-MP 0.3.7 Mobile Architecture)");
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_crmp_sampmobile_GameActivity_nativeInitSamp(
    JNIEnv* env,
    jobject /*thiz*/,
    jstring jGameDir,
    jstring jNick,
    jstring jHost,
    jint jPort
) {
    const char* cGameDir = env->GetStringUTFChars(jGameDir, nullptr);
    const char* cNick = env->GetStringUTFChars(jNick, nullptr);
    const char* cHost = env->GetStringUTFChars(jHost, nullptr);

    std::string gameDir(cGameDir);
    std::string nick(cNick);
    std::string host(cHost);

    env->ReleaseStringUTFChars(jGameDir, cGameDir);
    env->ReleaseStringUTFChars(jNick, cNick);
    env->ReleaseStringUTFChars(jHost, cHost);

    std::ostringstream ss;
    ss << "[libsamp.so] Инициализация ядра SA-MP 0.3.7 Mobile...\n";
    ss << "[libsamp.so] Игрок: " << nick << " | Целевой сервер: " << host << ":" << jPort << "\n";

    std::string gtasaPath = gameDir + "/lib/libGTASA.so";
    g_hGTASA = dlopen(gtasaPath.c_str(), RTLD_LAZY);
    if (!g_hGTASA) {
        g_hGTASA = dlopen("libGTASA.so", RTLD_LAZY);
    }

    if (g_hGTASA) {
        g_libGTASABase = crmp::MemoryHook::FindLibraryBase("libGTASA.so");
        char addrBuf[32];
        snprintf(addrBuf, sizeof(addrBuf), "0x%llx", static_cast<unsigned long long>(g_libGTASABase));
        ss << "[RenderWare] libGTASA.so ПОДКЛЮЧЕН! BaseAddr=" << addrBuf << "\n";
        ss << "[MemoryHook] Готов к патчингу оффсетов и хуку RenderQueue.";
    } else {
        ss << "[RenderWare] libGTASA.so не найден в " << gtasaPath << "\n";
        ss << "[Режим] Активен нативный RakNet / SA-MP Query клиент. Закиньте libGTASA.so + кэш GTA:SA 2.00 для запуска 3D-мира.";
    }

    return env->NewStringUTF(ss.str().c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_crmp_sampmobile_GameActivity_nativeQueryServer(
    JNIEnv* env,
    jobject /*thiz*/,
    jstring jHost,
    jint jPort
) {
    const char* cHost = env->GetStringUTFChars(jHost, nullptr);
    std::string host(cHost);
    env->ReleaseStringUTFChars(jHost, cHost);

    crmp::SampServerInfo info = crmp::RakNetClient::QuerySampServer(host, static_cast<uint16_t>(jPort), 1500);
    std::ostringstream ss;
    if (info.reachable) {
        ss << "[SA-MP Query] СЕРВЕР ОНЛАЙН! Ping: " << info.pingMs << " ms\n"
           << "  Название: " << info.hostname << "\n"
           << "  Игроки: " << info.players << " / " << info.maxPlayers
           << " | Мод: " << info.gamemode;
    } else {
        ss << "[SA-MP Query] Нет ответа от " << host << ":" << jPort << " (таймаут 1500 мс)";
    }
    return env->NewStringUTF(ss.str().c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_crmp_sampmobile_GameActivity_nativeSendRakNetHandshake(
    JNIEnv* env,
    jobject /*thiz*/,
    jstring jHost,
    jint jPort
) {
    const char* cHost = env->GetStringUTFChars(jHost, nullptr);
    std::string host(cHost);
    env->ReleaseStringUTFChars(jHost, cHost);

    std::string res = crmp::RakNetClient::SendOpenConnectionRequest1(host, static_cast<uint16_t>(jPort), 1500);
    return env->NewStringUTF(res.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_crmp_sampmobile_GameActivity_nativeOnSurfaceCreated(JNIEnv*, jobject) {
    crmp::SampHudRenderer::OnSurfaceCreated();
}

extern "C" JNIEXPORT void JNICALL
Java_com_crmp_sampmobile_GameActivity_nativeOnSurfaceChanged(JNIEnv*, jobject, jint w, jint h) {
    crmp::SampHudRenderer::OnSurfaceChanged(w, h);
}

extern "C" JNIEXPORT void JNICALL
Java_com_crmp_sampmobile_GameActivity_nativeOnDrawFrame(JNIEnv*, jobject) {
    crmp::SampHudRenderer::OnDrawFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_crmp_sampmobile_GameActivity_nativeShutdown(JNIEnv*, jobject) {
    if (g_hGTASA) {
        dlclose(g_hGTASA);
        g_hGTASA = nullptr;
    }
}
