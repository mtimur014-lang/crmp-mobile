#pragma once

namespace crmp {

class SampHudRenderer {
public:
    static void OnSurfaceCreated();
    static void OnSurfaceChanged(int width, int height);
    static void OnDrawFrame();
};

} // namespace crmp
