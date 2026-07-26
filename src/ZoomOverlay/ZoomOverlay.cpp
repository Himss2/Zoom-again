#include "ZoomOverlay/ZoomOverlay.hpp"

#include "Core/ModContext.hpp"
#include "ZoomController/ZoomController.hpp"

#include <pl/memory/Hook.hpp>

#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace zoom_overlay {
namespace {

constexpr float kZoneX = 60.0f;
constexpr float kZoneY = 120.0f;
constexpr float kZoneW = 150.0f;
constexpr float kZoneH = 90.0f;

constexpr float kColorIdle[4]   = {0.4f, 0.4f, 0.4f, 0.55f};
constexpr float kColorActive[4] = {0.0f, 0.65f, 0.0f, 0.55f};

using EglSwapBuffersFn = EGLBoolean (*)(EGLDisplay, EGLSurface);
EglSwapBuffersFn g_origSwapBuffers = nullptr;
void* g_target = nullptr;

GLuint g_program = 0;
GLint g_attribPos = -1;
GLint g_uniformColor = -1;
bool g_glResourcesReady = false;

const char* kVertexSrc = R"(#version 300 es
layout(location = 0) in vec2 aPosNdc;
void main() {
    gl_Position = vec4(aPosNdc, 0.0, 1.0);
}
)";

const char* kFragmentSrc = R"(#version 300 es
precision mediump float;
uniform vec4 uColor;
out vec4 fragColor;
void main() {
    fragColor = uColor;
}
)";

GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        core::Log().error("ZoomOverlay: shader compile failed: {}", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool EnsureGlResources() {
    if (g_glResourcesReady) {
        return true;
    }

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVertexSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFragmentSrc);
    if (!vs || !fs) {
        return false;
    }

    g_program = glCreateProgram();
    glAttachShader(g_program, vs);
    glAttachShader(g_program, fs);
    glLinkProgram(g_program);

    GLint linked = 0;
    glGetProgramiv(g_program, GL_LINK_STATUS, &linked);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(g_program, sizeof(log), nullptr, log);
        core::Log().error("ZoomOverlay: program link failed: {}", log);
        return false;
    }

    g_attribPos = glGetAttribLocation(g_program, "aPosNdc");
    g_uniformColor = glGetUniformLocation(g_program, "uColor");

    g_glResourcesReady = true;
    core::Log().info("ZoomOverlay: GL resources ready (program={})", g_program);
    return true;
}

void DrawZoneNdc(int surfaceWidth, int surfaceHeight, const float color[4]) {
    auto toNdcX = [&](float px) { return (px / static_cast<float>(surfaceWidth)) * 2.0f - 1.0f; };
    auto toNdcY = [&](float py) { return 1.0f - (py / static_cast<float>(surfaceHeight)) * 2.0f; };

    float x0 = toNdcX(kZoneX);
    float y0 = toNdcY(kZoneY);
    float x1 = toNdcX(kZoneX + kZoneW);
    float y1 = toNdcY(kZoneY + kZoneH);

    const float verts[12] = {
        x0, y0,  x1, y0,  x0, y1,
        x0, y1,  x1, y0,  x1, y1,
    };

    glUseProgram(g_program);
    glUniform4f(g_uniformColor, color[0], color[1], color[2], color[3]);
    glEnableVertexAttribArray(static_cast<GLuint>(g_attribPos));
    glVertexAttribPointer(static_cast<GLuint>(g_attribPos), 2, GL_FLOAT, GL_FALSE, 0, verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(static_cast<GLuint>(g_attribPos));
}

void RenderOverlay(EGLDisplay dpy, EGLSurface surface) {
    if (!EnsureGlResources()) {
        return;
    }

    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
    if (width <= 0 || height <= 0) {
        return;
    }

    GLint prevProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
    GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float* color = zoom_controller::IsActive() ? kColorActive : kColorIdle;
    DrawZoneNdc(width, height, color);

    glUseProgram(static_cast<GLuint>(prevProgram));
    if (!blendWasEnabled) {
        glDisable(GL_BLEND);
    }
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

EGLBoolean Detour(EGLDisplay dpy, EGLSurface surface) {
    RenderOverlay(dpy, surface);
    return g_origSwapBuffers(dpy, surface);
}

} // namespace

bool Install() {
    auto& log = core::Log();

    void* eglHandle = dlopen("libEGL.so", RTLD_NOW | RTLD_NOLOAD);
    if (!eglHandle) {
        eglHandle = dlopen("libEGL.so", RTLD_NOW);
    }
    if (!eglHandle) {
        log.error("ZoomOverlay: dlopen(libEGL.so) failed: {}", dlerror());
        return false;
    }

    g_target = dlsym(eglHandle, "eglSwapBuffers");
    if (!g_target) {
        log.error("ZoomOverlay: dlsym(eglSwapBuffers) failed: {}", dlerror());
        return false;
    }

    void* originalOut = nullptr;
    int result = pl::memory::hook(
        g_target,
        reinterpret_cast<void*>(Detour),
        &originalOut,
        pl::memory::HookPriority::Normal);

    if (result != 0) {
        log.error("ZoomOverlay: pl::memory::hook failed, code={}", result);
        g_target = nullptr;
        return false;
    }

    g_origSwapBuffers = reinterpret_cast<EglSwapBuffersFn>(originalOut);
    log.info("ZoomOverlay: hook installed on eglSwapBuffers at 0x{:x}",
             reinterpret_cast<uintptr_t>(g_target));
    return true;
}

void Uninstall() {
    if (g_target) {
        pl::memory::unhook(g_target, reinterpret_cast<void*>(Detour));
        g_target = nullptr;
    }
}

bool Contains(float x, float y) {
    return x >= kZoneX && x <= (kZoneX + kZoneW) &&
           y >= kZoneY && y <= (kZoneY + kZoneH);
}

} // namespace zoom_overlay
