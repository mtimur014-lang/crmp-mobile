#include "SampHudRenderer.h"
#include <GLES3/gl3.h>
#include <cmath>

namespace crmp {

static float g_pulse = 0.0f;

void SampHudRenderer::OnSurfaceCreated() {
    glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
}

void SampHudRenderer::OnSurfaceChanged(int width, int height) {
    glViewport(0, 0, width, height);
}

void SampHudRenderer::OnDrawFrame() {
    g_pulse += 0.02f;
    float r = 0.04f + 0.015f * std::sin(g_pulse);
    float b = 0.08f + 0.025f * std::cos(g_pulse);
    glClearColor(r, 0.05f, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace crmp
