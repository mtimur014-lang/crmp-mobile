# CRMP SA-MP Mobile Client (Android Launcher + Native C++ `libsamp.so`)

Нативный клиент на связке **Android Kotlin Launcher + C++ `libsamp.so` (NDK 26 + CMake + RakNet + MemoryHook + OpenGL ES 3.0)** с автоматической сборкой APK через GitHub Actions.

## Структура проекта
- `app/src/main/java/com/crmp/sampmobile/LauncherActivity.kt` — лаунчер: выбор сервера, ввод никнейма, генерация `SAMP/settings.ini` и проверка кэша.
- `app/src/main/java/com/crmp/sampmobile/GameActivity.kt` — игровая активити, загружающая `libsamp.so` и `libGTASA.so`.
- `app/src/main/cpp/main.cpp` — точка входа `JNI_OnLoad` и инициализация клиентского ядра `libsamp.so`.
- `app/src/main/cpp/net/RakNetClient.cpp` — нативный UDP-клиент протокола SA-MP (`'SAMP'` Query `'i'`) и RakNet (`ID_OPEN_CONNECTION_REQUEST_1`).
- `app/src/main/cpp/game/MemoryHook.cpp` — ARM64/ARMv7 патчер памяти (`mprotect` + кэш инструкций) для хуков `libGTASA.so`.
- `.github/workflows/build.yml` — автосборка `app-debug.apk` на GitHub Actions.

## Куда закинуть кэш GTA:SA Android 2.00
После первого запуска лаунчера в памяти телефона создастся папка:
`Android/data/com.crmp.sampmobile/files/`
- `lib/libGTASA.so` — нативная библиотека движка GTA:SA Android 2.00 (`arm64-v8a` / `armeabi-v7a`)
- `texdb/gta3.img` и `texdb/samp.img` — архивы моделей и текстур (машины, скины, карта CRMP)
- `SAMP/settings.ini` — создаётся и обновляется лаунчером автоматически
