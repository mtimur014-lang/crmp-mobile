cmake_minimum_required(VERSION 3.22.1)
project("samp")

add_library(
    samp
    SHARED
    main.cpp
    net/RakNetClient.cpp
    game/MemoryHook.cpp
    gui/SampHudRenderer.cpp
)

target_include_directories(samp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_CURRENT_SOURCE_DIR}/game
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(glesv3-lib GLESv3)

target_link_libraries(
    samp
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${glesv3-lib}
    dl
)
