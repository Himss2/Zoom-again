#include "CameraHook/FrameHook.hpp"
#include "Core/ModContext.hpp"

#include <EGL/egl.h>
#include <dlfcn.h>
#include <pl/memory/Hook.hpp>

namespace frame_hook {
namespace {

using EglSwapBuffersFn = EGLBoolean (*)(EGLDisplay, EGLSurface);

EglSwapBuffersFn g_origSwapBuffers = nullptr;
void* g_target = nullptr;
std::function<void()> g_callback;

EGLBoolean DetourSwapBuffers(EGLDisplay display, EGLSurface surface) {
    // Only fires when there's an active GL context to swap, which
    // matches "a frame was actually rendered" - same guard BedrockTools
    // uses for its FrameEvent.
    if (eglGetCurrentContext() != EGL_NO_CONTEXT && g_callback) {
        g_callback();
    }
    return g_origSwapBuffers ? g_origSwapBuffers(display, surface) : EGL_FALSE;
}

} // namespace

bool Install() {
    auto& log = core::Log();

    void* egl = dlopen("libEGL.so", RTLD_NOW | RTLD_NOLOAD);
    if (!egl) egl = dlopen("libEGL.so", RTLD_NOW);
    if (!egl) {
        log.error("FrameHook: failed to open libEGL.so");
        return false;
    }

    void* symbol = dlsym(egl, "eglSwapBuffers");
    dlclose(egl); // resolved symbol address stays valid after this
    if (!symbol) {
        log.error("FrameHook: failed to resolve eglSwapBuffers");
        return false;
    }

    g_target = symbol;
    void* origOut = nullptr;
    int res = pl::memory::hook(
        g_target,
        reinterpret_cast<void*>(DetourSwapBuffers),
        &origOut,
        pl::memory::HookPriority::Normal);

    if (res != 0) {
        log.error("FrameHook: pl::memory::hook failed, code={}", res);
        g_target = nullptr;
        return false;
    }

    g_origSwapBuffers = reinterpret_cast<EglSwapBuffersFn>(origOut);
    log.info("FrameHook: eglSwapBuffers hook installed");
    return true;
}

void Uninstall() {
    if (g_target) {
        pl::memory::unhook(g_target, reinterpret_cast<void*>(DetourSwapBuffers));
        g_target = nullptr;
    }
    g_origSwapBuffers = nullptr;
}

void SetFrameCallback(std::function<void()> callback) {
    g_callback = std::move(callback);
}

} // namespace frame_hook
